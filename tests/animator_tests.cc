#include <iostream>

#include <boost/scoped_ptr.hpp>

#include "animation/animator.h"
#include "animation/standard_easing.h"
#include "glutils/fbo.h"
#include "glutils/geometry.h"
#include "glutils/vbo.h"
#include "sdl/sdl_app.h"

class MyApp : public SdlGlApp {
 public:
  MyApp() {}

  virtual bool init();
  virtual bool initGl();
  virtual void handleEvent(SDL_Event* event) {
    if (event -> type == SDL_KEYUP && event -> key.keysym.sym == SDLK_ESCAPE)
      running_ = false;
    else
      SdlGlApp::handleEvent(event);
  }
  virtual void render();

 private:
  boost::scoped_ptr<Vbo>      vbo_;
  boost::scoped_ptr<Fbo>      fbo_;
  Animator                    animator_;
  GlVertex2                   x_;
};

bool MyApp::init()
{
  return SdlGlApp::init();
}

bool MyApp::initGl()
{
  // setup the viewport
  glViewport(0, 0, scr_w_, scr_h_);

  // setup the projection matrix
  // make normalized device coordinates identical to eye coordinates
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, scr_w_, 0, scr_h_);

  // setup the model matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0, 0, 0, 0);

  // initialize the VBO
  const size_t vbo_size = 256*1024;  // 256 kbytes
  vbo_.reset(new Vbo(vbo_size));

  // create the framebuffer
  fbo_.reset(new Fbo(scr_w_, scr_h_));

  return true;
}

void MyApp::render()
{
  static float last_x = 0, last_y = 0, last_t = 0;
  static bool first = true;

  animator_.update();

  // get access to the VBO
  std::vector<GlVertex2> points;

  // draw in FBO
  fbo_ -> bind();
  glDisable(GL_TEXTURE_2D);
  if (!animator_.isAnimated(&x_) && first) {
    first = false;
    glClear(GL_COLOR_BUFFER_BIT);

    animator_.addKeyframe(&x_, 0, GlVertex2(0, 0));
    animator_.addKeyframe(&x_, 1.5, GlVertex2(0.3, 0.15), BaseEasingPtr(
      (new StandardEasing(StandardEasing::QUADRATIC))
      -> setSubtype(StandardEasing::OUTIN)
    ));
    animator_.addKeyframe(&x_, 3, GlVertex2(0.4, 0.6), BaseEasingPtr(
      new StandardEasing(StandardEasing::LINEAR)
    ));
    animator_.addKeyframe(&x_, 4, GlVertex2(1, 1), BaseEasingPtr(
      (new StandardEasing(StandardEasing::SINE))
      -> setSubtype(StandardEasing::IN)
    ));

    glColor3f(1, 1, 1);
    points.push_back(GlVertex2(10, 10));
    points.push_back(GlVertex2(340, 10));
    points.push_back(GlVertex2(10, 10));
    points.push_back(GlVertex2(10, 340));

    last_x = 0;
    last_y = 0;
    last_t = 0;
  } else if (animator_.isAnimated(&x_)) {
    float t = animator_.getElapsed(&x_);

    glColor3f(1, 0.1, 0.1);
    points.push_back(GlVertex2(10 + last_t*300/4, 10 + last_x*300));
    points.push_back(GlVertex2(10 + t*300/4, 10 + x_.x*300));
    points.push_back(GlVertex2(10 + last_t*300/4, 10 + last_y*300));
    points.push_back(GlVertex2(10 + t*300/4, 10 + x_.y*300));

    last_x = x_.x;
    last_y = x_.y;
    last_t = t;
  }

/*  std::cout << "(" << points[0].x << "," << points[0].y << ") -- "
            << "(" << points[1].x << "," << points[1].y << ") ; "
            << "(" << points[2].x << "," << points[2].y << ") -- "
            << "(" << points[3].x << "," << points[3].y << ")." << std::endl;*/

  // send the data to OpenGL
  vbo_ -> draw(points, GL_LINES);

  // switch back to display
  fbo_ -> unbind();

  // draw the FBO to screen
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  fbo_ -> getTexture() -> bind();
  glColor3f(1, 1, 1);

  // draw to screen
  std::vector<GlVertexTex2> points_tex;

  points_tex.push_back(GlVertexTex2(0, 0, 0, 0));
  points_tex.push_back(GlVertexTex2(scr_w_, 0, 1, 0));
  points_tex.push_back(GlVertexTex2(scr_w_, scr_h_, 1, 1));
  points_tex.push_back(GlVertexTex2(0, scr_h_, 0, 1));

  // select the texture
  glClientActiveTexture(GL_TEXTURE0);

  // send the data to OpenGL
  vbo_ -> draw(points_tex, GL_QUADS);

  SDL_GL_SwapBuffers();

  // don't eat up unnecessary time
  SDL_Delay(10);
}

int main(int argc, char* argv[])
{
  try {
    MyApp app;
    app.execute();
  }
  catch (const std::runtime_error& e) {
    std::cout << "runtime error: " << e.what() << std::endl;
  }
  catch (const boost::bad_any_cast& e) {
    std::cout << "bad any cast: " << e.what() << std::endl;
  }
  catch (...) {
    std::cout << "exception." << std::endl;
  }

  return 0;
}

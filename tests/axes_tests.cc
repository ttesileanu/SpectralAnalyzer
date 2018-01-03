#include <iostream>

#include <cmath>

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/scoped_ptr.hpp>

#include "animation/animator.h"
#include "animation/standard_easing.h"
#include "animation/transition_store.h"
#include "display/axes.h"
#include "sdl/sdl_app.h"

class MyApp : public SdlGlApp {
 public:
  MyApp() {}

  virtual bool init();
  virtual bool initGl();
  virtual void handleEvent(SDL_Event* event);
  virtual void render();

 private:
  Animator                    animator_;
  Axes                        axes_;
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

  glClearColor(0, 0, 0, 1);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // set up some preliminary transitions
  TransitionStorePtr transitions = boost::make_shared<TransitionStore>();

  axes_.setTransitionStore(transitions);
  transitions -> add("open", 1,
    boost::make_shared<StandardEasing>(StandardEasing::QUADRATIC,
      StandardEasing::OUT)
  );

  // initialize the axes
  axes_.setRange(Rectangle(1, 1, 11, 11), "none");
  axes_.setCrossing(GlVertex2(6, 6), "none");

  axes_.setTickSpacingLinearX(Axes::MAJOR, 1, "none");
  axes_.setTickSpacingLinearX(Axes::MINOR, 0.5, "none");
  axes_.setTickSpacingLinearY(Axes::MAJOR, 1, "none");
  axes_.setTickSpacingLinearY(Axes::MINOR, 0.5, "none");

  axes_.setTickSpacingLogX(Axes::MAJOR, std::sqrt(2), "none");
  axes_.setTickSpacingLogX(Axes::MINOR, std::sqrt(std::sqrt(2)), "none");
  axes_.setTickSpacingLogY(Axes::MAJOR, std::sqrt(2), "none");
  axes_.setTickSpacingLogY(Axes::MINOR, std::sqrt(std::sqrt(2)), "none");

  // make them appear nicely
  axes_.setExtents(Rectangle(scr_w_/2 - 3*scr_w_/8, scr_h_/2 - 3*scr_h_/8,
    scr_w_/2 + 3*scr_w_/8, scr_h_/2 + 3*scr_h_/8), "none");
  axes_.setVisibility(false, "none");

  axes_.setExtents(Rectangle(scr_w_/20, scr_h_/20, 19*scr_w_/20, 19*scr_h_/20),
    "open");
  axes_.setVisibility(true, "open");

  // set up the rest of the transitions
  transitions -> add("fade", 0.3,
    boost::make_shared<StandardEasing>(StandardEasing::LINEAR)
  );
  transitions -> add("zoom", 0.4,
    boost::make_shared<StandardEasing>(StandardEasing::QUADRATIC,
      StandardEasing::OUT)
  );
  transitions -> add("type_change", 0.7,
    boost::make_shared<StandardEasing>(StandardEasing::LINEAR)
  );
  // XXX make a transition with overshoot here
  transitions -> add("tick", 0.3,
    boost::make_shared<StandardEasing>(StandardEasing::LINEAR)
  );
  transitions -> add("shift", 0.3,
    boost::make_shared<StandardEasing>(StandardEasing::QUADRATIC,
      StandardEasing::OUT)
  );

  return true;
}

void MyApp::handleEvent(SDL_Event* event)
{
  if (event -> type == SDL_KEYUP) {
    bool no_mods = (event -> key.keysym.mod == 0);
    bool shift = ((event -> key.keysym.mod & KMOD_SHIFT) != 0);
    bool just_shift = shift && ((event -> key.keysym.mod & (~KMOD_SHIFT)) == 0);
    switch (event -> key.keysym.sym) {
      case SDLK_ESCAPE:
        running_ = false;
        break;
      case SDLK_a:
        if (no_mods) {
          if (axes_.getType() == Axes::CROSS)
            axes_.setType(Axes::BOX);
          else
            axes_.setType(Axes::CROSS);
        }
        break;
      case SDLK_v:
        if (no_mods)
          axes_.flipVisibility();
        break;
      case SDLK_t:
        if (no_mods)
          axes_.flipTickVisibility();
        break;
      case SDLK_m:
        if (axes_.getTickVisibility()) {
          Axes::TicksType t = axes_.getTickType();
          if (no_mods) {
            if (t == Axes::MAJOR)
              t = Axes::BOTH;
            else if (t == Axes::BOTH)
              t = Axes::MAJOR;

            axes_.setTickType(t);
          } else if (just_shift) {
            if (t == Axes::MINOR)
              t = Axes::BOTH;
            else if (t == Axes::BOTH)
              t = Axes::MINOR;

            axes_.setTickType(t);
          }
        }
        break;
      case SDLK_d:
        if (no_mods)
          axes_.setTicksTwoSided(!axes_.areTicksTwoSided());
        break;
      case SDLK_g:
        if (no_mods)
          axes_.flipGridVisibility();
        break;
      case SDLK_b:
        if (no_mods)
          axes_.flipBoxVisibility();
        break;
      case SDLK_EQUALS:
        if (just_shift) {
          Rectangle r = axes_.getRange();
          float mid_x = (r.start.x + r.end.x) / 2;
          float len_x_2 = (r.end.x - r.start.x) / 2;
          r.start.x = mid_x - 0.9*len_x_2;
          r.end.x = mid_x + 0.9*len_x_2;

          axes_.setRange(r);
        } else if (no_mods) {
          Rectangle r = axes_.getRange();
          float mid_y = (r.start.y + r.end.y) / 2;
          float len_y_2 = (r.end.y - r.start.y) / 2;
          r.start.y = mid_y - 0.9*len_y_2;
          r.end.y = mid_y + 0.9*len_y_2;

          axes_.setRange(r);
        }
        break;
      case SDLK_MINUS:
        if (just_shift) {
          Rectangle r = axes_.getRange();
          float mid_x = (r.start.x + r.end.x) / 2;
          float len_x_2 = (r.end.x - r.start.x) / 2;
          r.start.x = mid_x - len_x_2 / 0.9;
          r.end.x = mid_x + len_x_2 / 0.9;

          axes_.setRange(r);
        } else if (no_mods) {
          Rectangle r = axes_.getRange();
          float mid_y = (r.start.y + r.end.y) / 2;
          float len_y_2 = (r.end.y - r.start.y) / 2;
          r.start.y = mid_y - len_y_2 / 0.9;
          r.end.y = mid_y + len_y_2 / 0.9;

          axes_.setRange(r);
        }
        break;
      case SDLK_k:
        if (no_mods) {
          Axes::ScalingType s = axes_.getTickSpacingY();
          if (s == Axes::LOG)
            s = Axes::LINEAR;
          else
            s = Axes::LOG;

          axes_.setTickSpacingY(s);
        } else if (just_shift) {
          Axes::ScalingType s = axes_.getTickSpacingX();
          if (s == Axes::LOG)
            s = Axes::LINEAR;
          else
            s = Axes::LOG;

          axes_.setTickSpacingX(s);
        }
        break;
      default:;
    }
  }
  else
    SdlGlApp::handleEvent(event);
}

void MyApp::render()
{
  glClear(GL_COLOR_BUFFER_BIT);

  axes_.updateAnimations();
  axes_.draw();

  SDL_GL_SwapBuffers();

  // don't eat up unnecessary time
  SDL_Delay(5);
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

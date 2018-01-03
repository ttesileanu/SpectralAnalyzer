#include "sdl/sdl_app.h"

#include <iostream>
//#include <ostream>

//#include <cmath>

int SdlGlApp::execute()
{
  if (!init())
    return -1;

  SDL_Event event;

  while (running_) {
    // do event handling
    // note that we keep track of how long each of these processes takes
    while (SDL_PollEvent(&event)) {
      handleEvent(&event);
    }

    // run the loop
    loop();
    
    // draw to screen
    render();
  }

  cleanup();

  return 0;
}

bool SdlGlApp::init()
{
  // init all SDL subsystems
  // XXX did I want to restrict to display or something?
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    return false;
  }

  // make sure SDL quits in case of errors
  atexit(SDL_Quit);

  // initialize SDL_GL
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
//  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  display_ = SDL_SetVideoMode(scr_w_, scr_h_, 0, SDL_OPENGL);
  if (!display_) {
    std::cerr << "Couldn't get screen. SDL error: " << SDL_GetError()
              << std::endl;
    return false;
  }

  if (!initGl())
    return false;

  running_ = true;
  return true;
}

// prints statistics corresponding to the vector of runtimes v.
/*static void printStats(std::ostream& out, const std::vector<double>& v,
  const std::string& name)
{
  if (v.empty())
    return;

  // find average, min, max, and standard deviation
  double avg = 0;
  double avg2 = 0;
  double min = v[0];
  double max = 0;

  for (size_t i = 0; i < v.size(); ++i) {
    avg += v[i];
    avg2 += v[i]*v[i];
    if (v[i] > max)
      max = v[i];
    if (v[i] < min)
      min = v[i];
  }

  avg /= v.size();
  avg2 /= v.size();
  double std = std::sqrt (avg2 - avg*avg);

  out << "----------------------------------------" << std::endl;
  out << name << " statistics:" << std::endl;
  out << "   Min/Max dt: " << min*1000 << " / " << max*1000 << " ms."
      << std::endl; 
  out << "   Average dt: " << avg*1000 << " ms." << std::endl;
  out << "   Standard deviation: " << std*1000 << " ms." << std::endl;
}*/

void SdlGlApp::cleanup()
{
  SDL_Quit();

  // print some stats about runtimes
/*  printStats(std::cout, ehandling_times_, "Event handling");
  printStats(std::cout, loop_times_, "Loop routine");
  printStats(std::cout, render_times_, "Rendering");*/
}

void SdlGlApp::handleEvent(SDL_Event* event)
{
  if (event -> type == SDL_QUIT) {
    running_ = false;
  }
}

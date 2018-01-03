/** @file sdl_app.h
 *  @brief Defines a basic SDL OpenGL application. Users should inherit from
 *  this class to create their applications.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef SDL_APP_H_
#define SDL_APP_H_

#include <boost/thread.hpp>

#include "sdl/sdl_incs.h"
#include "glutils/gl_incs.h"

/** @brief A class defining an SDL OpenGL application.
 *
 *  This class provides the basic initialization code. Users should inherit
 *  from this class and redefine at least the @a render method in order to
 *  do their drawing. Custom initialization routines and event handlers can
 *  be added.
 */
class SdlGlApp {
 public:
  /// Construct.
  SdlGlApp() : running_(false), display_(0), scr_w_(640), scr_h_(480) {}
  /// Virtual destructor, needed for proper inheritance.
  virtual ~SdlGlApp() {}

  /** @brief Set the width of the application window.
   *
   *  Should be called before @a init.
   */
  void            setWidth(int w) { scr_w_ = w; }
  /** @brief Set the height of the application window.
   *
   *  Should be called before @a init.
   */
  void            setHeight(int h) { scr_h_ = h; }

  /** @brief Run the application.
   *
   *  Returns 0 upon success.
   */
  int             execute();

  /** @brief Initialize the application.
   *
   *  Override this to perform additional initialization. Call @a SdlGlApp::init
   *  first if you need SDL and OpenGL to be initialized. Note that this calls
   *  @a initGl.
   *
   *  Should return true upon success, false otherwise.
   */
  virtual bool    init();
  /** @brief Initialize the application.
   *
   *  Perform OpenGL-specific initialization. This gets called from @a init.
   *  Override this method to perform more OpenGL initialization.
   *
   *  Should return true upon success, false otherwise.
   */
  virtual bool    initGl() { return true; }
  /** @brief Clean up.
   *
   *  Override this to perform additional cleanup.
   */
  virtual void    cleanup();

  /** @brief Handle the event.
   *
   *  By default, this only handles the @a SDL_QUIT event which is generated
   *  when you try to close the application. Override this method to handle more
   *  events.
   */
  virtual void    handleEvent(SDL_Event* event);

  /** @brief Perform idle activities.
   *
   *  This is called between event-handling and rendering. By default, it
   *  does nothing. Override this method to implement your own processing.
   *  Keep in mind that while this method is running, no event handling is
   *  possible, so keep the runtime short to ensure good interactivity.
   */
  virtual void    loop() {}
  /** @brief Render to screen.
   *
   *  This should perform the actual drawing for the application. The default
   *  implementation does nothing. Override this method to do your drawing.
   *  Keep in mind that no event handling is possible during the execution of
   *  this method, so keep its runtime short to ensure good interactivity.
   */
  virtual void    render() {}

  /// Swap GL buffers.
  void swapBuffers() { SDL_GL_SwapBuffers(); }

  /// Sleep for the given number of microseconds.
  void microDelay(size_t micro) { boost::this_thread::sleep(
    boost::posix_time::microseconds(micro)); }

 protected:
  bool            running_;
  SDL_Surface*    display_;
  int             scr_w_, scr_h_;
};

#endif

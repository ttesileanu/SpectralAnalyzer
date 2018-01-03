/** @file appwrapper.h
 *  @brief Defines a generic wrapper for an OpenGL application. This can then
 *  be inherited from to use different backends, such as SDL or Qt.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef APPWRAPPER_H_
#define APPWRAPPER_H_

#include <boost/thread.hpp>

#include "glutils/gl_incs.h"

/// A wrapper for an OpenGL application.
class BasicApp {
 public:
  /// Virtual destructor, needed for proper inheritance.
  virtual ~BasicApp() {}

  /// Set the initial width of the application window.
  virtual void    setWidth(int w) = 0;
  /// Set the initial height of the application window.
  virtual void    setHeight(int h) = 0;

  /** @brief Run the application.
   *
   *  Returns 0 upon success.
   */
  virtual int     execute() = 0;

  /** @brief Initialize the application.
   *
   *  This should call @a initGl. Should return @a true upon success, @a false
   *  otherwise.
   */
  virtual bool    init() = 0;
  /** @brief OpenGL-specific initialization.
   *
   *  This should be called from @a init. Should return @a true upon success,
   *  @a false otherwise.
   */
  virtual bool    initGl() { return true; }
  /// Clean up.
  virtual void    cleanup() = 0;

  /** @brief Handle the event.
   *
   *  By default, this only handles the @a SDL_QUIT event which is generated
   *  when you try to close the application. Override this method to handle more
   *  events.
   */
  virtual void    handleEvent(SDL_Event* event);

 /** @brief Render to screen.
   *
   *  This should perform the actual drawing for the application. The default
   *  implementation does nothing. Override this method to do your drawing.
   *  Keep in mind that no event handling is possible during the execution of
   *  this method, so keep its runtime short to ensure good interactivity.
   */
  virtual void    render() {}

  /// Swap GL buffers.
  virtual void swapBuffers() = 0;

  /// Sleep for the given number of microseconds.
  void microDelay(size_t micro) { boost::this_thread::sleep(
    boost::posix_time::microseconds(micro)); }
};

#endif

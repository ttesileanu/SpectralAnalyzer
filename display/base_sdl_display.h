/** @file base_sdl_display.h
 *  @brief Defines the additional interface elements for an SDL-based display
 *  module.
 *
 *  @author Tiberiu Tesileanu
 */
#ifndef BASE_SDL_DISPLAY_H_
#define BASE_SDL_DISPLAY_H_

#include "display/base_display.h"
#include "sdl/sdl_incs.h"

/// An SDL-based display.
class BaseSdlDisplay : public BaseDisplay {
 public:
  /// Empty constructor.
  BaseSdlDisplay() {}

  /** @brief Attempts to handle the @a event.
   *
   *  Override this in descendants to do event handling. Should return
   *  @a false if the event should be passed on to other modules; @a true
   *  if it handled the event, and wants it to not affect other modules.
   */
  virtual bool handleEvent(SDL_Event* event) { return false; }
};

#endif

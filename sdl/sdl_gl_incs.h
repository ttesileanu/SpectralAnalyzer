/** @file sdl_gl_incs.h.
 *  @brief This does the appropriate includes for SDL/OpenGL.
 */
#ifndef SDL_GL_INCS_H_
#define SDL_GL_INCS_H_

#include "sdl/sdl_incs.h"

#define GL_GLEXT_PROTOTYPES 1

// Things are (of course) slightly different on Macs...
#ifndef __APPLE__
# include <SDL/SDL_opengl.h>
#else
# include <SDL_opengl.h>
#endif

#endif

/** @file gl_incs.h.
 *  @brief This does the appropriate includes for OpenGL.
 */
#ifndef GL_INCS_H_
#define GL_INCS_H_

#define GL_GLEXT_PROTOTYPES 1

#if defined(__MACOSX__) || defined(__APPLE__)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <OpenGL/glext.h>
#elif defined(__MACOS__)
# include <gl.h>
# include <glu.h>
# include <glext.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glext.h>
#endif

#endif

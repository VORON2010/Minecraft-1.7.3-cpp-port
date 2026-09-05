#pragma once

#include <glad/glad.h>

#if defined(B173_GL_TRACE)
#include "GLTrace.h"

#undef glAlphaFunc
#undef glBeginQuery
#undef glBindBuffer
#undef glBindTexture
#undef glBlendFunc
#undef glBufferData
#undef glCallList
#undef glCallLists
#undef glClear
#undef glClearColor
#undef glClearDepth
#undef glColor3f
#undef glColor4f
#undef glColorMask
#undef glColorMaterial
#undef glColorPointer
#undef glCullFace
#undef glDeleteLists
#undef glDeleteQueries
#undef glDeleteTextures
#undef glDepthFunc
#undef glDepthMask
#undef glDisable
#undef glDisableClientState
#undef glDrawArrays
#undef glEnable
#undef glEnableClientState
#undef glEndList
#undef glEndQuery
#undef glFogf
#undef glFogfv
#undef glFogi
#undef glFrustum
#undef glGenBuffers
#undef glGenLists
#undef glGenQueries
#undef glGenTextures
#undef glGetError
#undef glGetFloatv
#undef glGetQueryObjectuiv
#undef glGetString
#undef glLightModelfv
#undef glLightfv
#undef glLineWidth
#undef glLoadIdentity
#undef glMatrixMode
#undef glNewList
#undef glNormal3f
#undef glNormalPointer
#undef glOrtho
#undef glPixelStorei
#undef glPolygonOffset
#undef glPopMatrix
#undef glPushMatrix
#undef glReadPixels
#undef glRotatef
#undef glScaled
#undef glScalef
#undef glShadeModel
#undef glTexCoordPointer
#undef glTexImage2D
#undef glTexParameteri
#undef glTexSubImage2D
#undef glTranslatef
#undef glVertexPointer
#undef glViewport

namespace GLTrace
{
#define B173_TRACE_GL(result, name, parameters, driverArguments, trace) \
	inline result APIENTRY name parameters \
	{ \
		if (enabled()) \
		{ \
			trace; \
		} \
		return glad_##name driverArguments; \
	}

B173_TRACE_GL(void, glAlphaFunc, (GLenum func, GLfloat ref), (func, ref), record("glAlphaFunc", func, ref))
B173_TRACE_GL(void, glBeginQuery, (GLenum target, GLuint id), (target, id), record("glBeginQuery", target, id))
B173_TRACE_GL(void, glBindBuffer, (GLenum target, GLuint buffer), (target, buffer), (record("glBindBuffer", target, buffer), bindBuffer(target, buffer)))
B173_TRACE_GL(void, glBindTexture, (GLenum target, GLuint texture), (target, texture), record("glBindTexture", target, texture))
B173_TRACE_GL(void, glBlendFunc, (GLenum sfactor, GLenum dfactor), (sfactor, dfactor), record("glBlendFunc", sfactor, dfactor))
B173_TRACE_GL(void, glBufferData, (GLenum target, GLsizeiptr size, const void *data, GLenum usage), (target, size, data, usage), record("glBufferData", target, size, Values{data, GL_UNSIGNED_BYTE, size > 0 ? static_cast<std::size_t>(size) : 0}, usage))
B173_TRACE_GL(void, glCallList, (GLuint list), (list), record("glCallList", list))
B173_TRACE_GL(void, glCallLists, (GLsizei n, GLenum type, const void *lists), (n, type, lists), record("glCallLists", n, type, Values{lists, type, n > 0 ? static_cast<std::size_t>(n) : 0}))
B173_TRACE_GL(void, glClear, (GLbitfield mask), (mask), record("glClear", mask))
B173_TRACE_GL(void, glClearColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha), (red, green, blue, alpha), record("glClearColor", red, green, blue, alpha))
B173_TRACE_GL(void, glClearDepth, (GLdouble depth), (depth), record("glClearDepth", depth))
B173_TRACE_GL(void, glColor3f, (GLfloat red, GLfloat green, GLfloat blue), (red, green, blue), record("glColor3f", red, green, blue))
B173_TRACE_GL(void, glColor4f, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha), (red, green, blue, alpha), record("glColor4f", red, green, blue, alpha))
B173_TRACE_GL(void, glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha), (red, green, blue, alpha), record("glColorMask", red, green, blue, alpha))
B173_TRACE_GL(void, glColorMaterial, (GLenum face, GLenum mode), (face, mode), record("glColorMaterial", face, mode))
B173_TRACE_GL(void, glColorPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer), (size, type, stride, pointer), record("glColorPointer", size, type, stride, ArrayPointer{GL_COLOR_ARRAY, size, type, stride, pointer}))
B173_TRACE_GL(void, glCullFace, (GLenum mode), (mode), record("glCullFace", mode))
B173_TRACE_GL(void, glDeleteLists, (GLuint list, GLsizei range), (list, range), record("glDeleteLists", list, range))
B173_TRACE_GL(void, glDeleteQueries, (GLsizei n, const GLuint *ids), (n, ids), record("glDeleteQueries", n, Values{ids, GL_UNSIGNED_INT, n > 0 ? static_cast<std::size_t>(n) : 0}))
B173_TRACE_GL(void, glDeleteTextures, (GLsizei n, const GLuint *textures), (n, textures), record("glDeleteTextures", n, Values{textures, GL_UNSIGNED_INT, n > 0 ? static_cast<std::size_t>(n) : 0}))
B173_TRACE_GL(void, glDepthFunc, (GLenum func), (func), record("glDepthFunc", func))
B173_TRACE_GL(void, glDepthMask, (GLboolean flag), (flag), record("glDepthMask", flag))
B173_TRACE_GL(void, glDisable, (GLenum cap), (cap), record("glDisable", cap))
B173_TRACE_GL(void, glDisableClientState, (GLenum array), (array), (record("glDisableClientState", array), clientState(array, false)))
B173_TRACE_GL(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count), (mode, first, count), record("glDrawArrays", mode, first, count, DrawArrays{first, count}))
B173_TRACE_GL(void, glEnable, (GLenum cap), (cap), record("glEnable", cap))
B173_TRACE_GL(void, glEnableClientState, (GLenum array), (array), (record("glEnableClientState", array), clientState(array, true)))
B173_TRACE_GL(void, glEndList, (), (), record("glEndList"))
B173_TRACE_GL(void, glEndQuery, (GLenum target), (target), record("glEndQuery", target))
B173_TRACE_GL(void, glFogf, (GLenum pname, GLfloat param), (pname, param), record("glFogf", pname, param))
B173_TRACE_GL(void, glFogfv, (GLenum pname, const GLfloat *params), (pname, params), record("glFogfv", pname, Values{params, GL_FLOAT, pname == GL_FOG_COLOR ? 4u : 1u}))
B173_TRACE_GL(void, glFogi, (GLenum pname, GLint param), (pname, param), record("glFogi", pname, param))
B173_TRACE_GL(void, glFrustum, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar), (left, right, bottom, top, zNear, zFar), record("glFrustum", left, right, bottom, top, zNear, zFar))
B173_TRACE_GL(void, glGenBuffers, (GLsizei n, GLuint *buffers), (n, buffers), record("glGenBuffers", n, buffers == nullptr ? "null-output" : "output"))
B173_TRACE_GL(GLuint, glGenLists, (GLsizei range), (range), record("glGenLists", range))
B173_TRACE_GL(void, glGenQueries, (GLsizei n, GLuint *ids), (n, ids), record("glGenQueries", n, ids == nullptr ? "null-output" : "output"))
B173_TRACE_GL(void, glGenTextures, (GLsizei n, GLuint *textures), (n, textures), record("glGenTextures", n, textures == nullptr ? "null-output" : "output"))
B173_TRACE_GL(GLenum, glGetError, (), (), record("glGetError"))
B173_TRACE_GL(void, glGetFloatv, (GLenum pname, GLfloat *data), (pname, data), record("glGetFloatv", pname, data == nullptr ? "null-output" : "output"))
B173_TRACE_GL(void, glGetQueryObjectuiv, (GLuint id, GLenum pname, GLuint *params), (id, pname, params), record("glGetQueryObjectuiv", id, pname, params == nullptr ? "null-output" : "output"))
B173_TRACE_GL(const GLubyte *, glGetString, (GLenum name), (name), record("glGetString", name))
B173_TRACE_GL(void, glLightModelfv, (GLenum pname, const GLfloat *params), (pname, params), record("glLightModelfv", pname, Values{params, GL_FLOAT, pname == GL_LIGHT_MODEL_AMBIENT ? 4u : 1u}))
B173_TRACE_GL(void, glLightfv, (GLenum light, GLenum pname, const GLfloat *params), (light, pname, params), record("glLightfv", light, pname, Values{params, GL_FLOAT, lightCount(pname)}))
B173_TRACE_GL(void, glLineWidth, (GLfloat width), (width), record("glLineWidth", width))
B173_TRACE_GL(void, glLoadIdentity, (), (), record("glLoadIdentity"))
B173_TRACE_GL(void, glMatrixMode, (GLenum mode), (mode), record("glMatrixMode", mode))
B173_TRACE_GL(void, glNewList, (GLuint list, GLenum mode), (list, mode), record("glNewList", list, mode))
B173_TRACE_GL(void, glNormal3f, (GLfloat nx, GLfloat ny, GLfloat nz), (nx, ny, nz), record("glNormal3f", nx, ny, nz))
B173_TRACE_GL(void, glNormalPointer, (GLenum type, GLsizei stride, const void *pointer), (type, stride, pointer), record("glNormalPointer", type, stride, ArrayPointer{GL_NORMAL_ARRAY, 3, type, stride, pointer}))
B173_TRACE_GL(void, glOrtho, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar), (left, right, bottom, top, zNear, zFar), record("glOrtho", left, right, bottom, top, zNear, zFar))
B173_TRACE_GL(void, glPixelStorei, (GLenum pname, GLint param), (pname, param), (record("glPixelStorei", pname, param), pixelStore(pname, param)))
B173_TRACE_GL(void, glPolygonOffset, (GLfloat factor, GLfloat units), (factor, units), record("glPolygonOffset", factor, units))
B173_TRACE_GL(void, glPopMatrix, (), (), record("glPopMatrix"))
B173_TRACE_GL(void, glPushMatrix, (), (), record("glPushMatrix"))
B173_TRACE_GL(void, glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels), (x, y, width, height, format, type, pixels), record("glReadPixels", x, y, width, height, format, type, pixels == nullptr ? "null-output" : "output"))
B173_TRACE_GL(void, glRotatef, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z), (angle, x, y, z), record("glRotatef", angle, x, y, z))
B173_TRACE_GL(void, glScaled, (GLdouble x, GLdouble y, GLdouble z), (x, y, z), record("glScaled", x, y, z))
B173_TRACE_GL(void, glScalef, (GLfloat x, GLfloat y, GLfloat z), (x, y, z), record("glScalef", x, y, z))
B173_TRACE_GL(void, glShadeModel, (GLenum mode), (mode), record("glShadeModel", mode))
B173_TRACE_GL(void, glTexCoordPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer), (size, type, stride, pointer), record("glTexCoordPointer", size, type, stride, ArrayPointer{GL_TEXTURE_COORD_ARRAY, size, type, stride, pointer}))
B173_TRACE_GL(void, glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels), (target, level, internalformat, width, height, border, format, type, pixels), record("glTexImage2D", target, level, internalformat, width, height, border, format, type, Pixels{pixels, width, height, format, type}))
B173_TRACE_GL(void, glTexParameteri, (GLenum target, GLenum pname, GLint param), (target, pname, param), record("glTexParameteri", target, pname, param))
B173_TRACE_GL(void, glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels), (target, level, xoffset, yoffset, width, height, format, type, pixels), record("glTexSubImage2D", target, level, xoffset, yoffset, width, height, format, type, Pixels{pixels, width, height, format, type}))
B173_TRACE_GL(void, glTranslatef, (GLfloat x, GLfloat y, GLfloat z), (x, y, z), record("glTranslatef", x, y, z))
B173_TRACE_GL(void, glVertexPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer), (size, type, stride, pointer), record("glVertexPointer", size, type, stride, ArrayPointer{GL_VERTEX_ARRAY, size, type, stride, pointer}))
B173_TRACE_GL(void, glViewport, (GLint x, GLint y, GLsizei width, GLsizei height), (x, y, width, height), record("glViewport", x, y, width, height))

#undef B173_TRACE_GL
}

#define glAlphaFunc GLTrace::glAlphaFunc
#define glBeginQuery GLTrace::glBeginQuery
#define glBindBuffer GLTrace::glBindBuffer
#define glBindTexture GLTrace::glBindTexture
#define glBlendFunc GLTrace::glBlendFunc
#define glBufferData GLTrace::glBufferData
#define glCallList GLTrace::glCallList
#define glCallLists GLTrace::glCallLists
#define glClear GLTrace::glClear
#define glClearColor GLTrace::glClearColor
#define glClearDepth GLTrace::glClearDepth
#define glColor3f GLTrace::glColor3f
#define glColor4f GLTrace::glColor4f
#define glColorMask GLTrace::glColorMask
#define glColorMaterial GLTrace::glColorMaterial
#define glColorPointer GLTrace::glColorPointer
#define glCullFace GLTrace::glCullFace
#define glDeleteLists GLTrace::glDeleteLists
#define glDeleteQueries GLTrace::glDeleteQueries
#define glDeleteTextures GLTrace::glDeleteTextures
#define glDepthFunc GLTrace::glDepthFunc
#define glDepthMask GLTrace::glDepthMask
#define glDisable GLTrace::glDisable
#define glDisableClientState GLTrace::glDisableClientState
#define glDrawArrays GLTrace::glDrawArrays
#define glEnable GLTrace::glEnable
#define glEnableClientState GLTrace::glEnableClientState
#define glEndList GLTrace::glEndList
#define glEndQuery GLTrace::glEndQuery
#define glFogf GLTrace::glFogf
#define glFogfv GLTrace::glFogfv
#define glFogi GLTrace::glFogi
#define glFrustum GLTrace::glFrustum
#define glGenBuffers GLTrace::glGenBuffers
#define glGenLists GLTrace::glGenLists
#define glGenQueries GLTrace::glGenQueries
#define glGenTextures GLTrace::glGenTextures
#define glGetError GLTrace::glGetError
#define glGetFloatv GLTrace::glGetFloatv
#define glGetQueryObjectuiv GLTrace::glGetQueryObjectuiv
#define glGetString GLTrace::glGetString
#define glLightModelfv GLTrace::glLightModelfv
#define glLightfv GLTrace::glLightfv
#define glLineWidth GLTrace::glLineWidth
#define glLoadIdentity GLTrace::glLoadIdentity
#define glMatrixMode GLTrace::glMatrixMode
#define glNewList GLTrace::glNewList
#define glNormal3f GLTrace::glNormal3f
#define glNormalPointer GLTrace::glNormalPointer
#define glOrtho GLTrace::glOrtho
#define glPixelStorei GLTrace::glPixelStorei
#define glPolygonOffset GLTrace::glPolygonOffset
#define glPopMatrix GLTrace::glPopMatrix
#define glPushMatrix GLTrace::glPushMatrix
#define glReadPixels GLTrace::glReadPixels
#define glRotatef GLTrace::glRotatef
#define glScaled GLTrace::glScaled
#define glScalef GLTrace::glScalef
#define glShadeModel GLTrace::glShadeModel
#define glTexCoordPointer GLTrace::glTexCoordPointer
#define glTexImage2D GLTrace::glTexImage2D
#define glTexParameteri GLTrace::glTexParameteri
#define glTexSubImage2D GLTrace::glTexSubImage2D
#define glTranslatef GLTrace::glTranslatef
#define glVertexPointer GLTrace::glVertexPointer
#define glViewport GLTrace::glViewport
#endif

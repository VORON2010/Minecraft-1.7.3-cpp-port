#pragma once
#include <vulkan/vulkan.h>

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef float GLfloat;
typedef double GLdouble;
typedef unsigned char GLboolean;

#define GL_PROJECTION_MATRIX 0x0BA7
#define GL_MODELVIEW_MATRIX 0x0BA6
#define GL_LIGHTING 0x0B50
#define GL_LIGHT0 0x4000
#define GL_LIGHT1 0x4001
#define GL_COLOR_MATERIAL 0x0B57
#define GL_FRONT_AND_BACK 0x0408
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#define GL_POSITION 0x1203
#define GL_DIFFUSE 0x1201
#define GL_AMBIENT 0x1200
#define GL_SPECULAR 0x1202
#define GL_FLAT 0x1D00
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#define GL_DEPTH_TEST 0x0B71
#define GL_RESCALE_NORMAL 0x803A
#define GL_LINES 0x0001
#define GL_SAMPLES_PASSED 0x8914
#define GL_BLEND 0x0BE2
#define GL_ALPHA_TEST 0x0BC0
#define GL_TEXTURE_2D 0x0DE1
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_GREATER 0x0204
#define GL_LEQUAL 0x0203
#define GL_FOG 0x0B60
#define GL_FOG_MODE 0x0B65
#define GL_FOG_DENSITY 0x0B62
#define GL_FOG_START 0x0B63
#define GL_FOG_END 0x0B64
#define GL_FOG_COLOR 0x0B66
#define GL_LINEAR 0x2601
#define GL_EXP 0x0800
#define GL_CULL_FACE 0x0B44
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_QUADS 0x0007
#define GL_COMPILE 0x1300
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_NEAREST 0x2600
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_EQUAL 0x0202
#define GL_ONE 1
#define GL_ZERO 0
#define GL_SRC_COLOR 0x0300
#define GL_DST_COLOR 0x0306
#define GL_NICEST 0
#define GL_PERSPECTIVE_CORRECTION_HINT 0
#define GL_COLOR_CLEAR_VALUE 0
#define GL_COLOR_WRITEMASK 0
#define GL_VIEWPORT 0
#define GL_NORMALIZE 0
#define GL_POLYGON_OFFSET_FILL 0
#define GL_VERTEX_ARRAY 0
#define GL_COLOR_ARRAY 0
#define GL_TEXTURE_COORD_ARRAY 0
#define GL_SMOOTH 0x1D01
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define GL_OUT_OF_MEMORY 0x0505
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_BGR_EXT 0x80E0
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_RGB 0x1907
#define GL_GEQUAL 0x0206
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_UNSIGNED_INT 0x1405
#define GL_FOG_DISTANCE_MODE_NV 0x855A
#define GL_EYE_RADIAL_NV 0x855B
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_QUERY_RESULT 0x8866
#define GL_TRIANGLE_FAN 0x0006
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLE_STRIP 0x0005
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_ALWAYS 0x0207
#define GL_TEXTURE 0x1702

#include "pc/vulkan/GLState.h"

inline void glGetFloatv(GLenum pname, GLfloat *params) {}
inline void glEnable(GLenum cap) { GLState::enable(cap, true); }
inline void glDisable(GLenum cap) { GLState::enable(cap, false); }
inline void glColorMaterial(GLenum face, GLenum mode) {}
inline void glLightfv(GLenum light, GLenum pname, const GLfloat *params) { GLState::lightfv(light, pname, params); }
inline void glShadeModel(GLenum mode) {}
inline void glLightModelfv(GLenum pname, const GLfloat *params) { GLState::lightModelfv(pname, params); }
inline void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { GLState::color(red, green, blue, alpha); }
inline void glColor3f(GLfloat red, GLfloat green, GLfloat blue) { GLState::color(red, green, blue, 1.0f); }
inline void glClear(GLenum mask) { GLState::clear(mask); }
inline void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) { GLState::clearColor(red, green, blue, alpha); }
inline void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {}
inline void glClearDepth(GLdouble depth) {}
inline void glDepthFunc(GLenum func) { GLState::depthFunc(func); }
inline void glDepthMask(unsigned char flag) { GLState::depthMask(flag != 0); }
inline void glCullFace(GLenum mode) { GLState::cullFace(mode); }
inline void glBlendFunc(GLenum sfactor, GLenum dfactor) { GLState::blendFunc(sfactor, dfactor); }
inline void glAlphaFunc(GLenum func, GLfloat ref) { GLState::alphaFunc(func, ref); }
inline void glFogi(GLenum pname, GLint param) { GLState::fogi(pname, param); }
inline void glFogf(GLenum pname, GLfloat param) { GLState::fogf(pname, param); }
inline void glFogfv(GLenum pname, const GLfloat *params) { GLState::fogfv(pname, params); }
inline void glHint(GLenum target, GLenum mode) {}
inline void glLineWidth(GLfloat width) {}
inline void glLogicOp(GLenum opcode) {}
inline void glColorMask(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) { GLState::colorMask(red != 0, green != 0, blue != 0, alpha != 0); }
inline void glMatrixMode(GLenum mode) {}
inline void glLoadIdentity() {}
inline void glPushMatrix() {}
inline void glPopMatrix() {}
inline void glMultMatrixf(const GLfloat *m) {}
inline void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {}
inline void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {}
inline void glScalef(GLfloat x, GLfloat y, GLfloat z) {}
inline void glScaled(GLdouble x, GLdouble y, GLdouble z) {}
inline void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {}
inline void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {}
inline void glBegin(GLenum mode) { GLState::begin(mode); }
inline void glEnd() { GLState::end(); }
inline void glVertex3f(GLfloat x, GLfloat y, GLfloat z) { GLState::vertex(x, y, z); }
inline void glVertex3d(GLdouble x, GLdouble y, GLdouble z) { GLState::vertex(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)); }
inline void glTexCoord2f(GLfloat s, GLfloat t) { GLState::texCoord(s, t); }
inline void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) { GLState::normal(nx, ny, nz); }
inline void glGenTextures(GLsizei n, GLuint *textures) {}
extern uint32_t g_boundTextureId;
inline void glBindTexture(GLenum target, GLuint texture) { g_boundTextureId = texture; }
inline void glTexParameteri(GLenum target, GLenum pname, GLint param) {}
inline void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) { GLState::texImage(width, height, pixels); }
inline void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) { GLState::texSubImage(xoffset, yoffset, width, height, pixels); }
inline void glDeleteTextures(GLsizei n, const GLuint *textures) {}
inline GLuint glGenLists(GLsizei range) { return 1; }
inline void glNewList(GLuint list, GLenum mode) { GLState::newList(list); }
inline void glEndList() { GLState::endList(); }
inline void glCallList(GLuint list) { GLState::callList(list); }
inline void glCallLists(GLsizei n, GLenum type, const void *lists) { GLState::callLists(n, type, lists); }
inline void glDeleteLists(GLuint list, GLsizei range) { GLState::deleteLists(list, range); }
inline void glListBase(GLuint base) { GLState::listBase(base); }
inline void glBeginQuery(GLenum target, GLuint id) {}
inline void glEndQuery(GLenum target) {}
inline void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params) {}
inline void glGenQueries(GLsizei n, GLuint *ids) {}
inline void glDeleteQueries(GLsizei n, const GLuint *ids) {}
inline void glNormal3b(char nx, char ny, char nz) {}
inline void glPolygonOffset(GLfloat factor, GLfloat units) {}
inline void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {}
inline GLenum glGetError() { return GL_NO_ERROR; }
inline void glEnableClientState(GLenum array) {}
inline void glDisableClientState(GLenum array) {}
inline void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {}
inline void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {}
inline void glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer) {}
inline void glDrawArrays(GLenum mode, GLint first, GLsizei count) {}
inline void glPixelStorei(GLenum pname, GLint param) {}

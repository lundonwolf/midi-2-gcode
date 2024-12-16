#ifndef __gl3w_h_
#define __gl3w_h_

#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* gl3w api */
int gl3wInit(void);
int gl3wIsSupported(int major, int minor);
void *gl3wGetProcAddress(const char *proc);

/* OpenGL functions */
extern PFNGLACTIVETEXTUREPROC                            glActiveTexture;
extern PFNGLATTACHSHADERPROC                            glAttachShader;
extern PFNGLBINDBUFFERPROC                              glBindBuffer;
extern PFNGLBINDVERTEXARRAYPROC                         glBindVertexArray;
extern PFNGLBUFFERDATAPROC                              glBufferData;
extern PFNGLCLEARCOLORPROC                              glClearColor;
extern PFNGLCLEARPROC                                   glClear;
extern PFNGLCOMPILESHADERPROC                           glCompileShader;
extern PFNGLCREATEPROGRAMPROC                           glCreateProgram;
extern PFNGLCREATESHADERPROC                            glCreateShader;
extern PFNGLDELETEBUFFERSPROC                           glDeleteBuffers;
extern PFNGLDELETEPROGRAMPROC                           glDeleteProgram;
extern PFNGLDELETESHADERPROC                            glDeleteShader;
extern PFNGLDELETEVERTEXARRAYSPROC                      glDeleteVertexArrays;
extern PFNGLDETACHSHADERPROC                            glDetachShader;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC                 glEnableVertexAttribArray;
extern PFNGLGENBUFFERSPROC                              glGenBuffers;
extern PFNGLGENVERTEXARRAYSPROC                         glGenVertexArrays;
extern PFNGLGETPROGRAMINFOLOGPROC                       glGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC                            glGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC                        glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC                             glGetShaderiv;
extern PFNGLLINKPROGRAMPROC                             glLinkProgram;
extern PFNGLSHADERSOURCEPROC                            glShaderSource;
extern PFNGLUSEPROGRAMPROC                              glUseProgram;
extern PFNGLVERTEXATTRIBPOINTERPROC                     glVertexAttribPointer;
extern PFNGLVIEWPORTPROC                                glViewport;

#ifdef __cplusplus
}
#endif

#endif

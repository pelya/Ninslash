/* #if defined(__ANDROID__) */
#ifndef __ANDROID_STUB_GLEW_H__
#define __ANDROID_STUB_GLEW_H__

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#define GLEW_OK 0
#define GLEW_ERROR 1

static inline GLenum glewInit (void)
{
	EGLDisplay Display;
	EGLSurface SurfaceDraw;
	/*EGLSurface SurfaceRead;*/
	EGLContext Context;

	Display = eglGetCurrentDisplay();
	SurfaceDraw = eglGetCurrentSurface(EGL_DRAW);
	/*SurfaceRead = eglGetCurrentSurface(EGL_READ);*/
	Context = eglGetCurrentContext();
	return (SurfaceDraw != EGL_NO_SURFACE) && (Display != EGL_NO_DISPLAY) && (Context != EGL_NO_CONTEXT) ? GLEW_OK : GLEW_ERROR;
}

typedef GLuint GLhandleARB;
typedef GLchar GLcharARB;

#define	glCompileShaderARB			glCompileShader
#define	glGetUniformLocationARB		glGetUniformLocation
#define	glLinkProgramARB			glLinkProgram
#define	glShaderSourceARB			glShaderSource
#define	glUniform1iARB				glUniform1i
#define	glUniform1fARB				glUniform1f
#define	glUseProgramObjectARB		glUseProgram

#endif /* __ANDROID_STUB_GLEW_H__*/
/* #endif */ /* defined(__ANDROID__) */

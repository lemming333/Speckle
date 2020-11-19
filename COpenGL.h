#pragma once
#include <windef.h>
#include <GL/glcorearb.h>
#include <vector>
#include <iostream>
#include "OGLFunctionPointers.h"

using namespace std;

class COpenGL
{
public:
	COpenGL(HWND hWnd);
	virtual ~COpenGL() {}

	bool OpenOpenGLContext(HWND hWnd);
	void CloseOpenGLContext();
	virtual void	RenderTheWindow();
	virtual void*	GetAnyGLFuncAddress(const char* name);

private:
	HWND m_hWnd;        // the main window handle
	HDC m_hDC;          // the main window device context
	int m_pixelFormat;  // the pixel format for the main window
	HGLRC m_hGLRC;      // the OpenGL context handle
	CGlFunctions* m_pGlFunctions;
	HMODULE m_OpenGlModule;

	GLuint  CreateShader(GLenum eShaderType, const string& strShaderFile);
	GLuint  CreateProgram(const vector<GLuint>& shaderList);
};


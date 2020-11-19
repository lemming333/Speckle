// Speckle.cpp : Defines the entry point for the application.
//

#include <vector>
#include <algorithm>
#include <iostream>
#include <GL/glcorearb.h>
#include "framework.h"
#include "Speckle.h"

using namespace std;

constexpr auto MAX_LOADSTRING = 100;

// Global Variables:
HINSTANCE hInst = 0;                            // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd = 0;                                  // the main window handle
HDC hDC = 0;                                    // the main window device context
int pixelFormat = 0;                            // the pixel format for the main window
HGLRC hGLRC = 0;                                // the OpenGL context handle
bool bProgramReady = false;                     // Is the program in place?

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool                OpenOpenGLContext(HDC& _hDC, HWND _hWnd, int& _pixelFormat, HGLRC& _hGLRC);
void                CloseOpenGLContext(HDC _hDC, HGLRC _hGLRC);
void*               GetAnyGLFuncAddress(const char* name);
GLuint              CreateShader(GLenum eShaderType, const std::string& strShaderFile);
GLuint              CreateProgram(const std::vector<GLuint>& shaderList);
void                RenderTheWindow(HDC hdc, HWND hWnd);

// get functions
#include "OGLFunctionPointers.h"
CGlFunctions * pGlFunctions = NULL;
HMODULE OpenGlModule;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SPECKLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPECKLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    pGlFunctions->glUseProgram(0);
    CloseOpenGLContext(hDC, hGLRC);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPECKLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SPECKLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
 
   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   OpenOpenGLContext(hDC, hWnd, pixelFormat, hGLRC);

   // put up the main window
   RenderTheWindow(hDC, hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RenderTheWindow(hdc, hWnd);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

GLuint CreateShader(GLenum eShaderType, const std::string& strShaderFile)
{
    GLuint shader = pGlFunctions->glCreateShader(eShaderType);
    const char* strFileData = strShaderFile.c_str();
    pGlFunctions->glShaderSource(shader, 1, &strFileData, NULL);

    pGlFunctions->glCompileShader(shader);

    GLint status;
    pGlFunctions->glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        pGlFunctions->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* strInfoLog = new GLchar[(int)(infoLogLength + (GLint)1)];
        pGlFunctions->glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

        const char* strShaderType = NULL;
        switch (eShaderType)
        {
        case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
        case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
        case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
        }

        fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
        delete[] strInfoLog;
    }

    return shader;
}

GLuint CreateProgram(const std::vector<GLuint>& shaderList)
{
    GLuint program = pGlFunctions->glCreateProgram();

    for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
    {
        pGlFunctions->glAttachShader(program, shaderList[iLoop]);
    }

    pGlFunctions->glLinkProgram(program);

    GLint status;
    pGlFunctions->glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        pGlFunctions->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* strInfoLog = new GLchar[(int)(infoLogLength + (GLint)1)];
        pGlFunctions->glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }

    for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
        pGlFunctions->glDetachShader(program, shaderList[iLoop]);

    return program;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

bool OpenOpenGLContext(HDC& _hDC, HWND _hWnd, int& _pixelFormat, HGLRC& _hGLRC)
{
    // set up OpenGL context - must make a separate context for each thread (and its window)
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    hDC = GetDC(_hWnd);
    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0) { return false; }
    SetPixelFormat(hDC, pixelFormat, &pfd);
    hGLRC = wglCreateContext(hDC); // create openGL context
    wglMakeCurrent(hDC, hGLRC);
    pGlFunctions = new CGlFunctions();
    return true;
}

void CloseOpenGLContext(HDC _hDC, HGLRC _hGLRC)
{
    // delete OpenGL context
    wglMakeCurrent(_hDC, NULL);
    wglDeleteContext(_hGLRC);
}

void* GetAnyGLFuncAddress(const char* name)
{
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1))
    {
        if (OpenGlModule == 0)
        {
            OpenGlModule = LoadLibraryA("opengl32.dll");
        }

        if (OpenGlModule != 0)
        {
            p = (void*)GetProcAddress(OpenGlModule, name);
        }
        else
        {
            std::cerr << "Could not get pointer to OGL function " << name << std::endl;
        }
    }

    return p;
}

void RenderTheWindow(HDC hdc, HWND hWnd)
{
    // set viewport
    RECT clientRect{ 0,0,0,0 };
    bool success = GetClientRect(hWnd, &clientRect);
    pGlFunctions->glViewport(0, 0, clientRect.right, clientRect.bottom);

    //////////////////////////////////////////////
    // first draw call - clear display buffer
    //////////////////////////////////////////////
    pGlFunctions->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pGlFunctions->glClear(GL_COLOR_BUFFER_BIT);

    //////////////////////////////////////////////
    // second draw call - draw a triangle
    //////////////////////////////////////////////
    // allocate vertices
    const float vertexPositions[] = {
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
    };
    unsigned int positionBufferObject = 0;
    pGlFunctions->glGenBuffers(1, &positionBufferObject);
    pGlFunctions->glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    pGlFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
    pGlFunctions->glBindBuffer(GL_ARRAY_BUFFER, 0);

    // load vertex data
    pGlFunctions->glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    pGlFunctions->glEnableVertexAttribArray(0);
    pGlFunctions->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Add vertex list to draw call program
    pGlFunctions->glDrawArrays(GL_TRIANGLES, 0, 3);

    // load shaders
    std::vector<GLuint> shaderList;
#include "VertexShader_copy.h"
    shaderList.push_back(CreateShader(GL_VERTEX_SHADER, strVertexShader_copy));
#include "PixelShader_copy.h"
    shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, strPixelShader_copy));
    unsigned int program = CreateProgram(shaderList);

    // run second draw call program instead of default
    pGlFunctions->glUseProgram(program);

    // swap result to display
    SwapBuffers(hdc);

    // clean up
    std::for_each(shaderList.begin(), shaderList.end(), pGlFunctions->glDeleteShader);
    pGlFunctions->glDisableVertexAttribArray(0);
}

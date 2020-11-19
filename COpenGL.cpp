#include <algorithm>
#include "framework.h"
#include "COpenGL.h"

// public methods
COpenGL::COpenGL(HWND hWnd)
    : m_hWnd(hWnd)
    , m_hDC(0)
    , m_pixelFormat(0)
    , m_hGLRC(0)
    , m_pGlFunctions(NULL)
    , m_OpenGlModule(0)
{}

bool COpenGL::OpenOpenGLContext(HWND hWnd)
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
    m_hDC = GetDC(hWnd);
    m_pixelFormat = ChoosePixelFormat(m_hDC, &pfd);
    if (m_pixelFormat == 0) { return false; }
    SetPixelFormat(m_hDC, m_pixelFormat, &pfd);
    m_hGLRC = wglCreateContext(m_hDC); // create openGL context
    wglMakeCurrent(m_hDC, m_hGLRC);
    m_pGlFunctions = new CGlFunctions(this);
    return true;
}

void COpenGL::CloseOpenGLContext()
{
    // delete OpenGL context
    m_pGlFunctions->glUseProgram(0);
    wglMakeCurrent(m_hDC, NULL);
    wglDeleteContext(m_hGLRC);
}

void COpenGL::RenderTheWindow()
{
    // set viewport
    RECT clientRect{ 0,0,0,0 };
    bool success = GetClientRect(m_hWnd, &clientRect);
    m_pGlFunctions->glViewport(0, 0, clientRect.right, clientRect.bottom);

    //////////////////////////////////////////////
    // first draw call - clear display buffer
    //////////////////////////////////////////////
    m_pGlFunctions->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    m_pGlFunctions->glClear(GL_COLOR_BUFFER_BIT);

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
    m_pGlFunctions->glGenBuffers(1, &positionBufferObject);
    m_pGlFunctions->glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    m_pGlFunctions->glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
    m_pGlFunctions->glBindBuffer(GL_ARRAY_BUFFER, 0);

    // load vertex data
    m_pGlFunctions->glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
    m_pGlFunctions->glEnableVertexAttribArray(0);
    m_pGlFunctions->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Add vertex list to draw call program
    m_pGlFunctions->glDrawArrays(GL_TRIANGLES, 0, 3);

    // load shaders
    std::vector<GLuint> shaderList;
#include "VertexShader_copy.h"
    shaderList.push_back(CreateShader(GL_VERTEX_SHADER, strVertexShader_copy));
#include "PixelShader_copy.h"
    shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, strPixelShader_copy));
    unsigned int program = CreateProgram(shaderList);

    // run second draw call program instead of default
    m_pGlFunctions->glUseProgram(program);

    // swap result to display
    SwapBuffers(m_hDC);

    // clean up
    std::for_each(shaderList.begin(), shaderList.end(), m_pGlFunctions->glDeleteShader);
    m_pGlFunctions->glDisableVertexAttribArray(0);
}

void* COpenGL::GetAnyGLFuncAddress(const char* name)
{
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1))
    {
        if (m_OpenGlModule == 0)
        {
            m_OpenGlModule = LoadLibraryA("opengl32.dll");
        }

        if (m_OpenGlModule != 0)
        {
            p = (void*)GetProcAddress(m_OpenGlModule, name);
        }
        else
        {
            std::cerr << "Could not get pointer to OGL function " << name << std::endl;
        }
    }

    return p;
}

// private methods
GLuint COpenGL::CreateShader(GLenum eShaderType, const std::string& strShaderFile)
{
    GLuint shader = m_pGlFunctions->glCreateShader(eShaderType);
    const char* strFileData = strShaderFile.c_str();
    m_pGlFunctions->glShaderSource(shader, 1, &strFileData, NULL);

    m_pGlFunctions->glCompileShader(shader);

    GLint status;
    m_pGlFunctions->glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        m_pGlFunctions->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* strInfoLog = new GLchar[(int)(infoLogLength + (GLint)1)];
        m_pGlFunctions->glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

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

GLuint COpenGL::CreateProgram(const std::vector<GLuint>& shaderList)
{
    GLuint program = m_pGlFunctions->glCreateProgram();

    for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
    {
        m_pGlFunctions->glAttachShader(program, shaderList[iLoop]);
    }

    m_pGlFunctions->glLinkProgram(program);

    GLint status;
    m_pGlFunctions->glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        m_pGlFunctions->glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar* strInfoLog = new GLchar[(int)(infoLogLength + (GLint)1)];
        m_pGlFunctions->glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }

    for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
        m_pGlFunctions->glDetachShader(program, shaderList[iLoop]);

    return program;
}


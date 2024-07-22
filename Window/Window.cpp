#include "Window.h"

bool left = false;

Window::Window(unsigned int width, unsigned int height)
    : m_Width(width), m_Height(height)
{
    /* Initialize the library */
    if (!glfwInit())
        throw std::runtime_error("error initializing glfw");

    /* Create a windowed mode window and its OpenGL context */
    //window = glfwCreateWindow(640, 480, "Mouse Tracker", NULL, NULL);
    m_GLFWwindow = glfwCreateWindow(m_Width, m_Height, "CSImage", NULL, NULL);
    if (!m_GLFWwindow)
    {
        glfwTerminate();
        throw std::runtime_error("error creating window\n");
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(m_GLFWwindow);

    if (glewInit() != GLEW_OK)
    {
        throw std::runtime_error("error initializing glew\n");
    }

    glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(m_GLFWwindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwGetCursorPos(m_GLFWwindow, &m_Cursor.x, &m_Cursor.y);

    float pos[] = { m_Cursor.x, m_Cursor.y };

    // move inside brush class
    // generate buffer and bind it to the GL_SHADER_STORAGE_BUFFER binding point
    glGenBuffers(1, &m_CursorBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_CursorBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(float), pos, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_CursorBuffer);
}

Window::~Window()
{
    if(m_Image != NULL)
        fclose(m_Image);

    glDeleteBuffers(1, &m_CursorBuffer);
}

void Window::Update()
{
    // if cursor enters the window disable it

    glfwGetCursorPos(m_GLFWwindow, &m_Cursor.x, &m_Cursor.y);

    if (m_Cursor.x <= 0.0 || m_Cursor.x >= (float)(m_Width - 1) || m_Cursor.y <= 0.0 || m_Cursor.y >= (float)(m_Height - 1))
    {
        if (!left)
        {
            glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPos(m_GLFWwindow, m_Cursor.x + 1.0, m_Cursor.y + 1.0);
        }

        left = true;
    }
    else
    {
        glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(m_GLFWwindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        left = false;
    }

    printf("x: %f\ty: %f\n", m_Cursor.x, m_Cursor.y);

    float pos[] = { m_Cursor.x, m_Cursor.y };

    // better if the buffer is mapped before the main application loop and the only its value on the application side is modified, this modification through teh mapping will reflect on the GPU
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(float), pos);

    if(glfwGetKey(m_GLFWwindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_GLFWwindow, GLFW_TRUE);

    //if (glfwGetWindowAttrib(m_GLFWwindow, GLFW_HOVERED))
    //{
    //    printf("entered\n");
    //}
    //else
    //{
    //    printf("left\n");
    //}
}

GLFWwindow* Window::GetGLFWwindow()
{
    return m_GLFWwindow;
}

unsigned int Window::GetWidth()
{
    return m_Width;
}

unsigned int Window::GetHeight()
{
    return m_Height;
}

cursor Window::GetCursor()
{
    return m_Cursor;
}
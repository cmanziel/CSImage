#include "Window.h"

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

    double xPos, yPos;

    glfwGetCursorPos(m_GLFWwindow, &xPos, &yPos);

    float pos[] = { xPos, yPos };

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
    // retrieve cursor position and pass it to the compute shader
    double xPos, yPos;

    glfwGetCursorPos(m_GLFWwindow, &xPos, &yPos);

    float pos[] = { xPos, yPos };

    // better if the buffer is mapped before the main application loop and the only its value on the application side is modified, this modification through teh mapping will reflect on the GPU
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(float), pos);
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
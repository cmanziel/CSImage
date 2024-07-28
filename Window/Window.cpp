#include "Window.h"

bool left = false;

Window::Window(char* path)
    : m_Path(path), m_State(STATE_INACTIVE)
{
    m_Image = fopen(path, "rb");
    unsigned char* idat_data, * image_pixel_data;

    if (m_Image == NULL)
    {
        m_ImageData = NULL;
        m_Width = 2000;
        m_Height = 1000;
    }
    else
    {
        m_Width = image_get_width(m_Image);
        m_Height = image_get_height(m_Image);

        unsigned char* filteredData = decompress_image(m_Image);

        // m_ImageData still containts the filter method before every scanline of pixels, so concatenate just the pixel channels' data into one array
        m_ImageData = concatenate_filtered_data(filteredData, m_Width, m_Height, CHANNELS_PER_PIXEL);

        free(filteredData);
    }

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

    glfwSetWindowUserPointer(m_GLFWwindow, this);
    glfwSetKeyCallback(m_GLFWwindow, key_callback);

    if (glewInit() != GLEW_OK)
    {
        throw std::runtime_error("error initializing glew\n");
    }

    glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(m_GLFWwindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwGetCursorPos(m_GLFWwindow, &m_Cursor.x, &m_Cursor.y);

    float pos[] = { m_Cursor.x, m_Cursor.y };

    m_Brush = new Brush(m_Cursor.x, m_Cursor.y);

    // move inside brush class
    // generate buffer and bind it to the GL_SHADER_STORAGE_BUFFER binding point
    glGenBuffers(1, &m_CursorBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_CursorBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(float), pos, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_CursorBuffer);

    if (m_Image != NULL)
        fclose(m_Image);
}

Window::~Window()
{
    if(m_Image != NULL)
        fclose(m_Image);

    free(m_ImageData);

    delete m_Brush;

    glDeleteBuffers(1, &m_CursorBuffer);
}

void Window::Update()
{
    CursorMovement();
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

unsigned char* Window::GetImageData()
{
    return m_ImageData;
}

cursor Window::GetCursor()
{
    return m_Cursor;
}

Brush* Window::GetBrush()
{
    return m_Brush;
}

void Window::CursorMovement()
{
    //get the last cursor positions
    glfwGetCursorPos(m_GLFWwindow, &m_Cursor.x, &m_Cursor.y);

    // cursor outside window's client area
    if (m_Cursor.x <= 0.0 || m_Cursor.x >= (float)(m_Width - 1) || m_Cursor.y <= 0.0 || m_Cursor.y >= (float)(m_Height - 1))
    {
        m_State = STATE_CURSOR_OUTSIDE;

        if (!left)
        {
            glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPos(m_GLFWwindow, m_Cursor.x + 1.0, m_Cursor.y + 1.0);
        }

        left = true;

        return;
    }
    else
    {
        m_State = STATE_CURSOR_INSIDE;

        glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(m_GLFWwindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        left = false;
    }

    float pos[] = { m_Cursor.x, m_Cursor.y };

    // bind the buffer first
    // better if the buffer is mapped before the main application loop and the only its value on the application side is modified, this modification through teh mapping will reflect on the GPU
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_CursorBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(float), pos);
}

void Window::KeyCallback(int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_D:
            m_Brush->ChangeState(STATE_DRAW);
            break;
        case GLFW_KEY_E:
            m_Brush->ChangeState(STATE_ERASE);
            break;
        case GLFW_KEY_UP:
            m_Brush->SetRadius(+1);
            break;
        case GLFW_KEY_DOWN:
            m_Brush->SetRadius(-1);
            break;
        case GLFW_KEY_P:
            TakeSnapshot();
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(m_GLFWwindow, GLFW_TRUE);
        }
    }
}

void Window::TakeSnapshot()
{
    // retrieve data from the texture object modified in the compute shader

}
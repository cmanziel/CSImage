#include "Window.h"

bool left = false;

Window::Window(char* path)
    : m_Path(path), m_State(STATE_INACTIVE)
{
    InitWindow();

    glfwGetCursorPos(m_GLFWwindow, &m_Cursor.x, &m_Cursor.y);

    float pos[] = { m_Cursor.x, m_Cursor.y };

    m_Brush = new Brush(m_Cursor.x, m_Cursor.y);

    // move inside brush class
    // generate buffer and bind it to the GL_SHADER_STORAGE_BUFFER binding point
    glGenBuffers(1, &m_CursorBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_CursorBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(float), pos, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_CursorBuffer);

    if (m_Image != NULL)
        fclose(m_Image);
}

void Window::InitWindow()
{
    m_Image = fopen(m_Path, "rb");
    unsigned char* idat_data, * image_pixel_data;

    m_Width = 1800;
    m_Height = 1000;

    if (m_Image != NULL)
    {
        unsigned int imgWidth = image_get_width(m_Image);
        unsigned int imgHeight = image_get_height(m_Image);

        m_ImageWidth = imgWidth;
        m_ImageHeight = imgHeight;

        unsigned char* filteredData = decompress_image(m_Image);

        // error handling for wrong decompression
        if (filteredData != NULL)
        {
            uint8_t channels_per_pixel = image_get_channels_per_pixel(m_Image);
            uint8_t bit_depth = image_get_bit_depth(m_Image);

            SetCanvasTextureData(channels_per_pixel, bit_depth);

            reconstruct_filtered_data(filteredData, imgWidth, imgHeight, channels_per_pixel, bit_depth);

            // m_ImageData still containts the filter method before every scanline of pixels, so concatenate just the pixel channels' data into one array
            m_ImageData = concatenate_filtered_data(filteredData, imgWidth, imgHeight, channels_per_pixel, bit_depth);

            free(filteredData);
        }
        else
            m_ImageData = NULL;
    }
    else
        m_ImageData = NULL;

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

    //glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //if (glfwRawMouseMotionSupported())
    //    glfwSetInputMode(m_GLFWwindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    InitRenderArea();
}

void Window::InitRenderArea()
{
    m_RenderArea.width = m_ImageWidth;
    m_RenderArea.height = m_ImageHeight;

    m_RenderArea.x = m_ImageWidth > m_Width ? 1.0 / 8.0 * m_Width : (m_Width - m_ImageWidth) / 2;
    m_RenderArea.y = m_ImageHeight > m_Height ? (m_Height - m_ImageHeight) * 1.5f : (m_Height - m_ImageHeight) / 2; // y coordinate from coordinate system with origin in bottom-left corner of the window

    glViewport(m_RenderArea.x, m_RenderArea.y, m_RenderArea.width, m_RenderArea.height);
}

render_area Window::GetRenderArea()
{
    return m_RenderArea;
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

int Window::GetWidth()
{
    return m_Width;
}

int Window::GetHeight()
{
    return m_Height;
}

int Window::GetImageWidth()
{
    return m_ImageWidth;
}

int Window::GetImageHeight()
{
    return m_ImageHeight;
}

unsigned char* Window::GetImageData()
{
    return m_ImageData;
}

void Window::SetCanvasTextureData(uint8_t cpp, uint8_t bit_depth)
{
    switch (cpp)
    {
    case 1:
        m_CanvasData.pixel_format = GL_RED; // red meaning a one channel pixel
        break;
    case 3:
        m_CanvasData.pixel_format = GL_RGB;
        break;
    case 4:
        m_CanvasData.pixel_format = GL_RGBA;
        break;
    }

    switch (bit_depth)
    {
    case 8:
        m_CanvasData.pixel_type = GL_UNSIGNED_BYTE;
        break;
    case 16:
        m_CanvasData.pixel_type = GL_UNSIGNED_SHORT;
        break;
    case 32:
        m_CanvasData.pixel_type = GL_UNSIGNED_INT;
        break;
    }
}

canvas_data Window::GetCanvasTextureData()
{
    return m_CanvasData;
}

cursor Window::GetCursor()
{
    return m_Cursor;
}

Brush* Window::GetBrush()
{
    return m_Brush;
}

int Window::GetState()
{
    return m_State;
}

void Window::CursorMovement()
{
    //get the last cursor positions
    glfwGetCursorPos(m_GLFWwindow, &m_Cursor.x, &m_Cursor.y); 

    // cursor outside window's client area
    if (m_Cursor.x <= (float)m_RenderArea.x || m_Cursor.x >= (float)(m_RenderArea.x + m_RenderArea.width)
        || m_Cursor.y >= m_Height - (float)m_RenderArea.y || m_Cursor.y <= m_Height - (float)(m_RenderArea.y + m_RenderArea.height))
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

        //glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        //if (glfwRawMouseMotionSupported())
        //    glfwSetInputMode(m_GLFWwindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        left = false;
    }

    float pos[] = { m_Cursor.x - m_RenderArea.x, m_Cursor.y - (m_Height - m_RenderArea.y - m_RenderArea.height) };

    // bind the buffer first
    // better if the buffer is mapped before the main application loop and the only its value on the application side is modified, this modification through teh mapping will reflect on the GPU
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_CursorBuffer);
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
        case GLFW_KEY_S:
            m_Brush->ChangeState(STATE_SOBEL);
            break;
        case GLFW_KEY_B:
            m_Brush->ChangeState(STATE_BLUR);
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
    unsigned char* raw_data = (unsigned char*)malloc(4 * OUTPUT_IMAGE_CHANNELS * m_ImageWidth * m_ImageHeight);

    glReadPixels(0, 0, m_ImageWidth, m_ImageHeight, GL_RGBA, GL_UNSIGNED_BYTE, raw_data);

    // output image uses three channels per pixel
    unsigned int row_size = OUTPUT_IMAGE_CHANNELS * m_ImageWidth * 4 + 1;

    // allcate 4 bytes per channel
    unsigned char* filtered_data = (unsigned char*)malloc(row_size * m_ImageHeight);

    if (filtered_data == NULL)
    {
        printf("error allocating the output image\n");
        return;
    }

    for (unsigned int y = 0; y < m_ImageHeight; y++)
    {
        unsigned int row_start = y * row_size;

        unsigned char filter_method = 0x00;

        filtered_data[row_start] = filter_method;

        for (int x = 0; x < m_ImageWidth * OUTPUT_IMAGE_CHANNELS * 4; x++)
        {
            filtered_data[row_start + 1 + x] = raw_data[y * (row_size - 1) + x];
        }
    }

    // edit image path appending _edited to its name
    strcpy(m_Path + strlen(m_Path) - strlen(".png"), "\0");

    const char* path = strcat(m_Path, "_edited.png");

    create_image(filtered_data, path, m_ImageWidth, m_ImageHeight, OUTPUT_IMAGE_CHANNELS, 8);

    free(filtered_data);
}

/*
void Window::TakeSnapshot()
{
    // modify GL_PACK_ALIGNMENT so the image in client data is stored in contiguous rows
    // match the format with the texture's image internal format
    // do the conversion
    // write to file

    // try using glReadPixels before

    canvas_data cd = m_CanvasData;
    int cpp = image_get_channels_per_pixel(m_Image);

    //if (cpp == 1)
    //    cd.pixel_format = GL_RED_INTEGER;
    //else if (cpp == 3)
    //    cd.pixel_format = GL_RGB_INTEGER;
    //else
    //    cd.pixel_format = GL_RGBA_INTEGER;

    // desired format: GL_RGB_INTEGER

    // retrieve data from the texture object modified in the compute shader
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB_INTEGER, GL_UNSIGNED_BYTE, m_ImageData);

    // output image uses three channels per pixel
    unsigned int row_size = OUTPUT_IMAGE_CHANNELS * m_ImageWidth + 1;

    unsigned char* filtered_data = (unsigned char*)malloc(m_ImageWidth * m_ImageHeight * OUTPUT_IMAGE_CHANNELS + m_ImageHeight);

    if (filtered_data == NULL)
    {
        printf("error allocating the output image\n");
        return;
    }

    for (unsigned int y = 0; y < m_ImageHeight; y++)
    {
        unsigned int row_start = y * row_size;

        unsigned char filter_method = 0x00;

        filtered_data[row_start] = filter_method;

        for (int x = 0; x < m_ImageWidth; x++)
        {
            filtered_data[row_start + 1 + x] = m_ImageData[y * (row_size - 1) + x];
        }
    }

    // edit image path appending _edited to its name
    strcpy(m_Path + strlen(m_Path) - strlen(".png"), "\0");

    const char* path = strcat(m_Path, "_edited.png");

    create_image(filtered_data, path, m_ImageWidth, m_ImageHeight, OUTPUT_IMAGE_CHANNELS);

    free(filtered_data);
}
*/
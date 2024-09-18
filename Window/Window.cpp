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
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);

    // make sure the render texture is the last one bind to the GL_TEXTURE_2D target
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, m_ImageData);

    unsigned int row_size = OUTPUT_IMAGE_CHANNELS * m_ImageWidth + 1;

    unsigned int filt_index = 0;
    unsigned char* filtered_data = (unsigned char*)malloc(row_size * m_ImageHeight);

    if (filtered_data == NULL)
    {
        printf("error allocating the output image\n");
        return;
    }

    unsigned int i = 0;
    while(i < m_ImageWidth * 4 * m_ImageHeight)
    {
        if (i % (m_ImageWidth * 4) == 0) // index of the first pixel of a row of raw data
            filtered_data[filt_index++] = 0x00;

        filtered_data[filt_index++] = m_ImageData[i++];
        filtered_data[filt_index++] = m_ImageData[i++];
        filtered_data[filt_index++] = m_ImageData[i++];
        i++; // skip alpha channel
    }

    // edit image path appending _edited to its name
    strcpy(m_Path + strlen(m_Path) - strlen(".png"), "\0");

    const char* path = strcat(m_Path, "_edited.png");

    create_image(filtered_data, path, m_ImageWidth, m_ImageHeight, OUTPUT_IMAGE_CHANNELS, 8);

    free(filtered_data);
}

//void Window::TakeSnapshot()
//{
//    // define the pixel pack store state and alignment
//    glPixelStorei(GL_PACK_ALIGNMENT, 1); //byte alignment, row of pixels start on multiple of 1 so whichever byte is possible
//
//    glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);
//
//    // allocate memory for a 32 bits per channel rgba image
//    unsigned int fBytesSize = m_ImageWidth * 4 * sizeof(float) * m_ImageHeight; // numebr of bytes in the rgab32f image
//    float* fBytes = (float*)malloc(fBytesSize);
//
//    //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, fBytes);
//    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, m_ImageData);
//
//    unsigned int row_size = OUTPUT_IMAGE_CHANNELS * m_ImageWidth + 1; 
//
//    // allcate 4 bytes per channel
//    unsigned char* filtered_data = (unsigned char*)malloc(row_size * m_ImageHeight);
//
//    if (filtered_data == NULL)
//    {
//        printf("error allocating the output image\n");
//        return;
//    }
//
//    // for every channel convert it to an uint8_t, store it in an array and send it to pnglib
//    for (unsigned int i = 0; i < fBytesSize; i++)
//    {
//        if (i % 3 == 0)
//            continue;
//
//        uint8_t uiByte = fBytes[i] * 255;
//
//        filtered_data[i - i / 4] = uiByte;
//    }
//
//    // edit image path appending _edited to its name
//    strcpy(m_Path + strlen(m_Path) - strlen(".png"), "\0");
//
//    const char* path = strcat(m_Path, "_edited.png");
//
//    create_image(filtered_data, path, m_ImageWidth, m_ImageHeight, OUTPUT_IMAGE_CHANNELS, 8);
//
//    free(filtered_data);
//    free(fBytes);
//}
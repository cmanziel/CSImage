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
    glfwSetMouseButtonCallback(m_GLFWwindow, mouse_button_callback);

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

    m_Brush->SetPosition(m_Cursor.x, m_Cursor.y);

    //cursor curs = m_Brush->GetPosition();
    //printf("cx: %f\tcy: %f\n", curs.x, curs.y);

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
            m_Brush->ChangeDrawState(STATE_DRAW);
            break;
        case GLFW_KEY_S:
            m_Brush->ChangeDrawState(STATE_SOBEL);
            break;
        case GLFW_KEY_B:
            m_Brush->ChangeDrawState(STATE_BLUR);
            break;
        case GLFW_KEY_E:
            m_Brush->ChangeDrawState(STATE_ERASE);
            break;
        case GLFW_MOUSE_BUTTON_1:
            m_Brush->ChangeMouseState(STATE_DRAG);
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

    if (action == GLFW_RELEASE)
    {
        switch (key)
        {
        case GLFW_MOUSE_BUTTON_1:
            m_Brush->ChangeMouseState(STATE_RELEASED);
            break;
        }
    }
}

void Window::MouseButtonCallback(int button, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_1:
            m_Brush->ChangeMouseState(STATE_DRAG);
            break;
        }
    }

    if (action == GLFW_RELEASE)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_1:
            m_Brush->ChangeMouseState(STATE_RELEASED);
            break;
        }
    }
}

void Window::TakeSnapshot()
{
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);

    // make sure the render texture is the last one bind to the GL_TEXTURE_2D target
    //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, m_ImageData); // returns the canvas image flipped and whitout the edits drawn onto it
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageData); // returns the actual canvas modified, rows go from bottom to top, whereas in a png file the data is stored top to bottom, so fill the data row-by-row reversly

    unsigned int row_size = OUTPUT_IMAGE_CHANNELS * m_ImageWidth + 1;

    unsigned int filt_index = 0;
    unsigned char* filtered_data = (unsigned char*)malloc(row_size * m_ImageHeight);

    if (filtered_data == NULL)
    {
        printf("error allocating the output image\n");
        return;
    }

    //rows go from bottom to top as returned from the texture
    // whereas in a png file the data is stored top to bottom, so fill the data row by row reversely
    unsigned int i = 0;
    while (i < m_ImageWidth * 4 * m_ImageHeight)
    {
        if (i % (m_ImageWidth * 4) == 0) // index of the first pixel of a row of raw data
            filtered_data[filt_index++] = 0x00;

        unsigned int start = m_ImageWidth * 4 * (m_ImageHeight - 1 - i / (m_ImageWidth * 4));

        filtered_data[filt_index++] = m_ImageData[start + i++ % (m_ImageWidth * 4)];
        filtered_data[filt_index++] = m_ImageData[start + i++ % (m_ImageWidth * 4)];
        filtered_data[filt_index++] = m_ImageData[start + i++ % (m_ImageWidth * 4)];
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
//    glPixelStorei(GL_PACK_ALIGNMENT, 1);
//    glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);
//
//    // pack data as 32 bit float, multiply for 255 and round to uint8_t integer, then write to array of unsigned chars to send to pnglib
//
//    // allocate array of floats to store texture image data
//    unsigned int fBytesSize = 4 * sizeof(float) * m_ImageWidth * m_ImageHeight;
//    float* fBytes = (float*)malloc(fBytesSize);
//
//    // make sure the render texture is the last one bind to the GL_TEXTURE_2D target
//    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, fBytes);
//
//    unsigned int row_size = OUTPUT_IMAGE_CHANNELS * m_ImageWidth + 1;
//
//    unsigned int filt_index = 0;
//    unsigned char* filtered_data = (unsigned char*)malloc(row_size * m_ImageHeight);
//
//    if (filtered_data == NULL)
//    {
//        printf("error allocating the output image\n");
//        return;
//    }
//
//    unsigned int i = 0;
//    while (i < m_ImageWidth * 4 * m_ImageHeight)
//    {
//        if (i % (m_ImageWidth * 4) == 0)
//            filtered_data[filt_index++] = 0x00;
//
//        filtered_data[filt_index++] = m_ImageData[i++] * 255;
//        filtered_data[filt_index++] = m_ImageData[i++] * 255;
//        filtered_data[filt_index++] = m_ImageData[i++] * 255;
//        i++; // skip alpha channel
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
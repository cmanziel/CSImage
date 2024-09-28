#include "Window.h"

bool left = false;

Window::Window(char* path)
    : m_Path(path), m_State(STATE_CURSOR_OUTSIDE)
{
    m_Width = 1900;
    m_Height = 1200;

    InitWindow();

    m_Editables.push_back(new Editable(path, m_Width, m_Height, 0));

    m_CurrentEditable = m_Editables[0];

    m_Brush = new Brush(0.0f, 0.0f);

    InitRenderArea();
}

void Window::InitWindow()
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
}

void Window::InitRenderArea()
{
    // initiate the viewport with dimensions of the first image opened
    render_area currentRA = m_CurrentEditable->GetRenderArea();

    glViewport(currentRA.x, currentRA.y, currentRA.width, currentRA.height);
}

Editable* Window::GetCurrentEditable()
{
    return m_CurrentEditable;
}

std::vector<Editable*> Window::GetEditables()
{
    return m_Editables;
}

Window::~Window()
{
    if(m_Image != NULL)
        fclose(m_Image);

    for (Editable* edit : m_Editables)
    {
        delete edit;
    }

    m_Editables.clear();

    delete m_Brush;
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

    // flip y coordinate
    m_Cursor.y = m_Height - m_Cursor.y;

    // pick the closest image to the user if the images overlap, in that case the cursor hovers over multiple render areas, the one to use is the one in the foreground relative to the other ones
    Editable* closer = NULL;
    for (unsigned int i = 0; i < m_Editables.size(); i++)
    {
        Editable* ed = m_Editables[i];

        if (!ed->IsCursorInside(m_Cursor))
            continue;

        if (i == 0)
            closer = ed;
        else if (ed->GetWeight() <= closer->GetWeight())
            closer = ed;
    }

    m_CurrentEditable = closer; // if no editable is hovered m_CurrentEditable becomes NULL

    //m_Brush->SetPosition(m_Cursor.x - m_RenderArea.x, (m_RenderArea.y + m_RenderArea.height) - m_Cursor.y);
    m_Brush->SetPosition(m_Cursor.x, m_Cursor.y); // position relative to the window's client area

    if (m_Brush->GetMouseState() == STATE_DRAG && m_CurrentEditable != NULL)
        m_CurrentEditable->Move(m_Brush->GetPosition());
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

void Window::MouseButtonCallback(int button, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_1:
        {
            // if an editable was hovered
            if (m_CurrentEditable != NULL)
            {
                m_Brush->ChangeMouseState(STATE_DRAG);

                //m_CurrentEditable->Move(m_Brush->GetPosition());
            }
        } break;
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

}

//void Window::TakeSnapshot()
//{
//    glPixelStorei(GL_PACK_ALIGNMENT, 1);
//    glPixelStorei(GL_PACK_SWAP_BYTES, GL_TRUE);
//
//    // make sure the render texture is the last one bind to the GL_TEXTURE_2D target
//    //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, m_ImageData); // returns the canvas image flipped and whitout the edits drawn onto it
//    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageData); // returns the actual canvas modified, rows go from bottom to top, whereas in a png file the data is stored top to bottom, so fill the data row-by-row reversly
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
//    //rows go from bottom to top as returned from the texture
//    // whereas in a png file the data is stored top to bottom, so fill the data row by row reversely
//    unsigned int i = 0;
//    while (i < m_ImageWidth * 4 * m_ImageHeight)
//    {
//        if (i % (m_ImageWidth * 4) == 0) // index of the first pixel of a row of raw data
//            filtered_data[filt_index++] = 0x00;
//
//        unsigned int start = m_ImageWidth * 4 * (m_ImageHeight - 1 - i / (m_ImageWidth * 4));
//
//        filtered_data[filt_index++] = m_ImageData[start + i++ % (m_ImageWidth * 4)];
//        filtered_data[filt_index++] = m_ImageData[start + i++ % (m_ImageWidth * 4)];
//        filtered_data[filt_index++] = m_ImageData[start + i++ % (m_ImageWidth * 4)];
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
//}
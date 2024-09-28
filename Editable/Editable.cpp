#include "Editable.h"

void setCanvasData(canvas_data* cd, uint8_t cpp, uint8_t bit_depth);

Editable::Editable(char* image_path, unsigned int win_width, unsigned int win_height, int weight)
{
    SetUpImage(image_path);
    m_WinWidth = win_width;
    m_WinHeight = win_height;
    m_Weight = weight;
    InitRenderArea();
    
    glGenTextures(1, &m_RenderTexture);
    glBindTexture(GL_TEXTURE_2D, m_RenderTexture);
    // set format based on the image channels per pixel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_Image.width, m_Image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // for texture completeness
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (m_Image.pixel_data != NULL)
    {
        glGenTextures(1, &m_CanvasTexture);
        glBindTexture(GL_TEXTURE_2D, m_CanvasTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_Image.width, m_Image.height, 0, m_CanvasData.pixel_format, m_CanvasData.pixel_type, m_Image.pixel_data);
        // for texture completeness
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

Editable::~Editable()
{
    glDeleteTextures(1, &m_RenderTexture);
    glDeleteTextures(1, &m_CanvasTexture);
}

void Editable::SetUpImage(char* image_path)
{
    m_Image.p_file = fopen(image_path, "rb");

    if (m_Image.p_file != NULL)
    {
        m_Image.width = image_get_width(m_Image.p_file);
        m_Image.height = image_get_height(m_Image.p_file);

        unsigned char* filteredData = decompress_image(m_Image.p_file);

        // error handling for wrong decompression
        if (filteredData != NULL)
        {
            uint8_t channels_per_pixel = image_get_channels_per_pixel(m_Image.p_file);
            uint8_t bit_depth = image_get_bit_depth(m_Image.p_file);

            setCanvasData(&m_CanvasData, channels_per_pixel, bit_depth);

            reconstruct_filtered_data(filteredData, m_Image.width, m_Image.height, channels_per_pixel, bit_depth);

            // m_ImageData still containts the filter method before every scanline of pixels, so concatenate just the pixel channels' data into one array
            m_Image.pixel_data = concatenate_filtered_data(filteredData, m_Image.width, m_Image.height, channels_per_pixel, bit_depth);

            free(filteredData);
        }
        else
            m_Image.pixel_data = NULL;
    }
    else
        m_Image.pixel_data = NULL;
}

void Editable::InitRenderArea()
{
    m_RenderArea.width = m_Image.width;
    m_RenderArea.height = m_Image.height;

    m_RenderArea.x = m_Image.width > m_WinWidth ? 1.0 / 8.0 * m_WinWidth : (m_WinWidth - m_Image.width) / 2;
    m_RenderArea.y = m_Image.height > m_WinHeight ? (m_WinHeight - m_Image.height) * 1.5f : (m_WinHeight - m_Image.height) / 2;

    m_RenderArea.ndc_width = (float)m_RenderArea.width / m_WinWidth * 2.0f;
    m_RenderArea.ndc_height = (float)m_RenderArea.height / m_WinHeight * 2.0f;
}

bool Editable::IsCursorInside(cursor curs)
{
    // make sure to pass curs.y as coordinates in positive-y-up coordinate system
    int cursX = curs.x; // float to int implicit cast
    int cursY = curs.y;

    if (cursX < m_RenderArea.x || cursX > m_RenderArea.x + m_RenderArea.width
        || cursY < m_RenderArea.y || cursY > m_RenderArea.y + m_RenderArea.height)
        return false;

    WindowToRenderArea(curs);

    return true;
}

void Editable::Move(cursor curs)
{
    m_RenderArea.x += (int)curs.drag_delta_x;
    m_RenderArea.y += (int)curs.drag_delta_y;
    
    // x and y in ndc coordinates: [-1.0, 1.0]
    m_RenderArea.ndc_x = (float)m_RenderArea.x / m_RenderArea.width * 2.0f - 1.0f;
    m_RenderArea.ndc_y = (float)m_RenderArea.y / m_RenderArea.height * 2.0f - 1.0f;

    //float t = m_RenderArea.ndc_y + m_RenderArea.ndc_height;
    //float b = m_RenderArea.ndc_y;
    //float l = m_RenderArea.ndc_x;
    //float r = m_RenderArea.ndc_x + m_RenderArea.ndc_width;

    //// change the rendering quad coordinates
    //m_RenderingQuad[0] = l;
    //m_RenderingQuad[1] = t;
    //m_RenderingQuad[4] = l;
    //m_RenderingQuad[5] = b;
    //m_RenderingQuad[8] = r;
    //m_RenderingQuad[9] = b;
    //m_RenderingQuad[12] = l;
    //m_RenderingQuad[13] = t;
    //m_RenderingQuad[16] = r;
    //m_RenderingQuad[17] = b;
    //m_RenderingQuad[20] = r;
    //m_RenderingQuad[21] = t;

    float ndc_delta_x = curs.drag_delta_x / m_RenderArea.width * 2.0f;
    float ndc_delta_y = curs.drag_delta_y / m_RenderArea.height * 2.0f;

    m_RenderingQuad[0] += ndc_delta_x;
    m_RenderingQuad[1] += ndc_delta_y;
    m_RenderingQuad[4] += ndc_delta_x;
    m_RenderingQuad[5] += ndc_delta_y;
    m_RenderingQuad[8] += ndc_delta_x;
    m_RenderingQuad[9] += ndc_delta_y;
    m_RenderingQuad[12] += ndc_delta_x;
    m_RenderingQuad[13] += ndc_delta_y;
    m_RenderingQuad[16] += ndc_delta_x;
    m_RenderingQuad[17] += ndc_delta_y;
    m_RenderingQuad[20] += ndc_delta_x;
    m_RenderingQuad[21] += ndc_delta_y;
}

void Editable::WindowToRenderArea(cursor curs)
{
    //m_CursorInRA.x = curs.x - m_RenderArea.x;
    //m_CursorInRA.y = curs.y - m_RenderArea.y;

    m_RenderArea.cursor_x = curs.x - m_RenderArea.x;
    m_RenderArea.cursor_y = curs.y - m_RenderArea.y;
}

editable_image Editable::GetImage()
{
    return m_Image;
} 

render_area Editable::GetRenderArea()
{
    return m_RenderArea;
}

int Editable::GetWeight()
{
    return m_Weight;
}

float* Editable::GetRenderingQuad()
{
    return m_RenderingQuad;
}

GLuint Editable::GetRenderTexture()
{
    return m_RenderTexture;
}

GLuint Editable::GetCanvasTexture()
{
    return m_CanvasTexture;
}

void setCanvasData(canvas_data* cd, uint8_t cpp, uint8_t bit_depth)
{
    switch (cpp)
    {
    case 1:
        cd->pixel_format = GL_RED; // red meaning a one channel pixel
        break;
    case 3:
        cd->pixel_format = GL_RGB;
        break;
    case 4:
        cd->pixel_format = GL_RGBA;
        break;
    }

    switch (bit_depth)
    {
    case 8:
        cd->pixel_type = GL_UNSIGNED_BYTE;
        break;
    case 16:
        cd->pixel_type = GL_UNSIGNED_SHORT;
        break;
    case 32:
        cd->pixel_type = GL_UNSIGNED_INT;
        break;
    }
}
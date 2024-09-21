#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "pnglib.h"
#include <stdexcept>
#include "../Brush/Brush.h"

#include <Windows.h>
#include <WinUser.h>

#define LAST_CURSOR_POS_NUM 10
#define OUTPUT_IMAGE_CHANNELS 3

enum window_state
{
	STATE_CURSOR_INSIDE, STATE_CURSOR_OUTSIDE, STATE_CURSOR_SNAPSHOT
};

typedef struct {
	int tex_internal_format;
	int pixel_format;
	int pixel_type;
} canvas_data;

typedef struct {
	int x; // coordinates of the bottom-left conrner in window space: (0,0) is top-left corner
	int y;
	int width;
	int height;
	
	// in ndc space coordinates
	float ndc_x;
	float ndc_y;
	float ndc_width;
	float ndc_height;
} render_area;

class Window
{
public:
	Window(char* path);
	~Window();

	void InitWindow();
	void InitRenderArea();
	render_area GetRenderArea();

	void Update();

	GLFWwindow* GetGLFWwindow();
	int GetWidth();
	int GetHeight();

	unsigned char* GetImageData();
	int GetImageWidth();
	int GetImageHeight();

	canvas_data GetCanvasTextureData();
	void SetCanvasTextureData(uint8_t cpp, uint8_t bit_depth);

	cursor GetCursor();
	Brush* GetBrush();
	int GetState();

	//void KeyInput();
	//void WindowResize();
	void CursorMovement();
	void TakeSnapshot();

	virtual void key_callback(int key, int scancode, int action, int mods)
	{
		KeyCallback(key, scancode, action, mods);
	}

	void KeyCallback(int key, int scancode, int action, int mods);

	virtual void mouse_button_callback(int button, int action, int mods)
	{
		MouseButtonCallback(button, action, mods);
	}

	void MouseButtonCallback(int button, int action, int mods);

private:
	GLFWwindow* m_GLFWwindow;
	int m_Width;
	int m_Height;
	int m_ImageWidth;
	int m_ImageHeight;

	cursor m_Cursor;
	GLuint m_CursorBuffer;

	Brush* m_Brush;
	uint8_t m_State;
	render_area m_RenderArea;

	FILE* m_Image; // file pointer to the image file currently being edited
	char* m_Path;
	unsigned char* m_ImageData;
	canvas_data m_CanvasData;

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (win)
			win->key_callback(key, scancode, action, mods);
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (win)
			win->mouse_button_callback(button, action, mods);
	}
};

#endif // !WINDOW_H
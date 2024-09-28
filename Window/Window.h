#ifndef WINDOW_H
#define WINDOW_H


#include <stdexcept>
#include <vector>
#include "../Editable/Editable.h"

#include <Windows.h>
#include <WinUser.h>

#define LAST_CURSOR_POS_NUM 10
#define OUTPUT_IMAGE_CHANNELS 3

enum window_state
{
	STATE_CURSOR_INSIDE, STATE_CURSOR_OUTSIDE, STATE_CURSOR_SNAPSHOT
};

class Window
{
public:
	Window(char* path);
	~Window();

	void InitWindow();
	void InitRenderArea();
	Editable* GetCurrentEditable();
	std::vector<Editable*> GetEditables();

	void Update();

	GLFWwindow* GetGLFWwindow();
	int GetWidth();
	int GetHeight();

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

	cursor m_Cursor;

	Brush* m_Brush;
	uint8_t m_State;

	Editable* m_CurrentEditable; // the image being currently edited
	std::vector<Editable*> m_Editables; // vector that holds each Editable instace

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
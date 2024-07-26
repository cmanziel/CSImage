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

enum window_state
{
	STATE_CURSOR_INSIDE, STATE_CURSOR_OUTSIDE, STATE_CURSOR_SNAPSHOT
};

class Window
{
public:
	Window(char* path);
	~Window();

	void Update();

	GLFWwindow* GetGLFWwindow();
	unsigned int GetWidth();
	unsigned int GetHeight();

	cursor GetCursor();
	Brush* GetBrush();

	//void KeyInput();
	//void WindowResize();
	void CursorMovement();
	void TakeSnapshot();

	virtual void key_callback(int key, int scancode, int action, int mods)
	{
		KeyCallback(key, scancode, action, mods);
	}

	void KeyCallback(int key, int scancode, int action, int mods);

private:
	GLFWwindow* m_GLFWwindow;
	unsigned int m_Width;
	unsigned int m_Height;

	cursor m_Cursor;
	GLuint m_CursorBuffer;

	Brush* m_Brush;
	uint8_t m_State;

	FILE* m_Image; // file pointer to the image file currently being edited
	char* m_Path;

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (win)
			win->key_callback(key, scancode, action, mods);
	}
};

#endif // !WINDOW_H
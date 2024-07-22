#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "pnglib.h"
#include <stdexcept>

typedef struct {
	double x;
	double y;
} cursor;

class Window
{
public:
	Window(unsigned int width, unsigned int height);
	~Window();

	void Update();

	GLFWwindow* GetGLFWwindow();
	unsigned int GetWidth();
	unsigned int GetHeight();

	cursor GetCursor();

	//void KeyInput();
	void WindowResize();
	void CursorMovement();

	void TakeSnapshot();

private:
	GLFWwindow* m_GLFWwindow;
	unsigned int m_Width;
	unsigned int m_Height;

	cursor m_Cursor;
	GLuint m_CursorBuffer;

	FILE* m_Image; // file pointer to the image file currently being edited
	char* m_Path;
};

#endif // !WINDOW_H
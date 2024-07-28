#ifndef RENDERER_H

#define RENDERER_H
#include "../Shader/ComputeShader.h"
#include "../Shader/VFShader.h"
#include "../Window/Window.h"

class Renderer
{
public:
	Renderer(Window* win);
	~Renderer();

	void Draw();

private:
	Window* m_Window;

	ComputeShader m_ComputeShader;
	VFShader m_VFShader;
	ComputeShader m_CanvasShader;

	GLuint m_RenderTexture;
	GLuint m_Canvas; // image to be drawn onto

	//GLuint m_Framebuffer;
	GLuint m_ScreenQuadBuffer;
};

#endif // !1
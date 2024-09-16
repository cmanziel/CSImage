#ifndef RENDERER_H
#define RENDERER_H

#include "../Shader/VFShader.h"
#include "../Shader/SobelShader.h"
#include "../Shader/BlurShader.h"
#include "../Shader/DrawEraseShader.h"
#include "../Shader/CanvasShader.h"
#include "../Window/Window.h"

class Renderer
{
public:
	Renderer(Window* win);
	~Renderer();

	void SelectShader();

	void Draw();

private:
	Window* m_Window;

	DrawEraseShader m_DrawEraseShader;
	VFShader m_VFShader;
	CanvasShader m_CanvasShader;
	SobelShader m_SobelShader;
	BlurShader m_BlurShader;

	ComputeShader* m_CurrentShader;

	GLuint m_RenderTexture;
	GLuint m_Canvas; // image to be drawn onto

	//GLuint m_Framebuffer;
	GLuint m_ScreenQuadBuffer;
	float m_RenderingQuad[24]; // coordinates for the quad to which the texture will be mapped
};

#endif // !1
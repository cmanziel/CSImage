#ifndef RENDERER_H
#define RENDERER_H

#include "../Shader/VFShader.h"
#include "../Shader/SobelShader.h"
#include "../Shader/BlurShader.h"
#include "../Shader/DrawEraseShader.h"
#include "../Shader/CanvasShader.h"
#include "../Window/Window.h"

#define RENDER_TEXTURE_UNIT 0
#define CANVAS_TEXTURE_UNIT 1
#define CURSOR_BUFFER_UNIT 2

class Renderer
{
public:
	Renderer(Window* win);
	~Renderer();

	void SelectEditable();
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
	Editable* m_CurrentEditable;

/*	GLuint m_RenderTexture;
	GLuint m_Canvas; */// image to be drawn onto

	//GLuint m_Framebuffer;
	GLuint m_ScreenQuadBuffer;
	GLuint m_CursorBuffer;

	float* m_RenderingQuad; // ndc coordinates of the current editable being renderered
};

#endif // !1
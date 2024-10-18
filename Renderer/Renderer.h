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

	void SelectEditable();
	void SelectShader();

	void Draw();

private:
	Window* m_Window;

	DrawEraseShader m_DrawEraseShader;
	CanvasShader m_CanvasShader;
	SobelShader m_SobelShader;
	BlurShader m_BlurShader;

	VFShader m_VFShader;
	VFShader m_RACShader; // render area container shader

	ComputeShader* m_CurrentShader;
	Editable* m_CurrentEditable;

/*	GLuint m_RenderTexture;
	GLuint m_Canvas; */// image to be drawn onto

	GLuint m_ScreenQuadBuffer;
	GLuint m_CursorBuffer;
	GLuint m_RACBuffer;

	// these point to the current editable's respective fields
	float* m_RenderingQuad; // ndc coordinates of the current editable being renderered
	float* m_RenderingContainer; // ndc coordinates for the outline of the editable being rendered, rendered with GL_LINES drawing mode
};

#endif // !1
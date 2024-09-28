#ifndef EDITABLE_H
#define EDITABLE_H

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "pnglib.h"
#include "../Brush/Brush.h"

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

	float cursor_x;
	float cursor_y;
} render_area;

typedef struct
{
	unsigned int width;
	unsigned int height;
	FILE* p_file; // pointer to the image file
	unsigned char* pixel_data;
} editable_image;

typedef struct {
	int tex_internal_format;
	int pixel_format;
	int pixel_type;
} canvas_data;

class Editable
{
public:
	Editable(char* image_path, unsigned int win_width, unsigned int win_height, int weight);
	~Editable();
	void SetUpImage(char* imgPath);

	editable_image GetImage();
	render_area GetRenderArea();
	int GetWeight();
	float* GetRenderingQuad();
	GLuint GetRenderTexture();
	GLuint GetCanvasTexture();

	void InitRenderArea();

	bool IsCursorInside(cursor curs);
	void Move(cursor curs);
	void WindowToRenderArea(cursor curs);
private:
	editable_image m_Image;
	render_area m_RenderArea;
	canvas_data m_CanvasData;

	// weigth: acoording to its weight an image occupies a level within the background and foreground
	int m_Weight;
	unsigned int m_WinWidth;
	unsigned int m_WinHeight;

	// for the shaders to draw properly keeping track of the changes
	GLuint m_CanvasTexture;
	GLuint m_RenderTexture;

	float m_RenderingQuad[24] = {
		// 2 floats for postions coordinates, 2 for texture coordinates
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	}; // set of vertices to be passed to the shader, with the proper texture coordinates for each vertex
};

#endif // !EDITABLE_H

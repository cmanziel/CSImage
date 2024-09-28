#pragma once

#include <stdint.h>
#include <math.h>
#include <stdio.h>

#define CHANNELS_PER_PIXEL 3
#define BRUSH_RADIUS 50
#define MAX_BRUSH_RADIUS 100

typedef struct
{
	double x;
	double y;

	double drag_start_x;
	double drag_start_y;

	double drag_delta_x;
	double drag_delta_y;
} cursor;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} color;

#define DELTA_AREA_RADIUS 0

enum draw_state
{
	STATE_DRAW, STATE_ERASE, STATE_INACTIVE, STATE_SOBEL, STATE_BLUR
};

enum mouse_state
{
	STATE_DRAG, STATE_RELEASED
};

class Brush
{
public:
	Brush(double cursor_x, double cursor_y);
	~Brush();

	cursor GetPosition();
	void SetPosition(double xPos, double yPos);

	//void ChangeState(uint8_t state);

	void ChangeDrawState(uint8_t state);
	void ChangeMouseState(uint8_t state);

	//uint8_t GetState();

	uint8_t GetDrawState();
	uint8_t GetMouseState();

	int GetRadius();

	// delta milliseconds as argument
	void SetRadius(int mod);

	color GetColor();

	bool IsPointInside(int grid_point_x, int grid_point_y);
private:
	int m_Radius;

	cursor m_Cursor;

	//cursor m_DragStart;
	//cursor m_DragDelta;

	//uint8_t m_State;
	uint8_t m_DrawState; // handles the different drawing states, if the mouse button is not down the brush doens't draw anything
	uint8_t m_MouseState; // handles the "dragging" state, when the mouse left button is hold down
	color m_Color;
};


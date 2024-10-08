#include "Brush.h"

float vector_length(cursor v);

Brush::Brush(double cursor_x, double cursor_y)
	: m_DrawState(STATE_INACTIVE), m_MouseState(STATE_RELEASED)
{
	m_Cursor.x = cursor_x;
	m_Cursor.y = cursor_y;
	m_Cursor.drag_start_x = cursor_x;
	m_Cursor.drag_start_y = cursor_y;
	m_Cursor.drag_delta_x = 0.0;
	m_Cursor.drag_delta_y = 0.0;

	m_Radius = BRUSH_RADIUS;

	//start with red color
	m_Color.r = 255;
	m_Color.g = 0;
	m_Color.b = 0;
}

Brush::~Brush()
{

}

cursor Brush::GetPosition()
{
	return m_Cursor;
}

void Brush::SetPosition(double xPos, double yPos)
{
	//m_Cursor.x = xPos;
	//m_Cursor.y = yPos;

	if (m_MouseState == STATE_DRAG && m_DrawState == STATE_INACTIVE)
	{
		//m_Cursor.drag_delta_x = m_Cursor.x - m_Cursor.drag_start_x;
		//m_Cursor.drag_delta_y = m_Cursor.y - m_Cursor.drag_start_y;

		m_Cursor.drag_delta_x = xPos - m_Cursor.x;
		m_Cursor.drag_delta_y = yPos - m_Cursor.y;
	}

	m_Cursor.x = xPos;
	m_Cursor.y = yPos;

	//printf("cx: %f\tcy: %f\n", m_Cursor.x, m_Cursor.y);
}

int Brush::GetRadius()
{
	return m_Radius;
}

//void Brush::SetRadius(int mod)
//{
//	m_Radius = m_Radius == 0 && mod < 0 ? m_Radius : m_Radius + mod;
//}

void Brush::SetRadius(int mod)
{
	//float delta_velocity = vector_length(m_Velocity) - vector_length(m_LastVelocity);
	//float delta_velocity = m_Velocity - m_LastVelocity;

	//int scaling_factor = delta_velocity * 5 > 50 ? 50 : delta_velocity * 5;

	//m_Radius = m_Radius + scaling_factor < 0 ? 0 : m_Radius + scaling_factor;

	m_Radius = m_Radius == 0 ? 1 : m_Radius + mod;
}

color Brush::GetColor()
{
	return m_Color;
}

bool Brush::IsPointInside(int grid_point_x, int grid_point_y)
{
	// the cursor is the brush's grid's center, the argument is a point of the grid that needs to be evaluated if it's inside the brush's radius
	// cursor position in the window grid coordinates
	int grid_centre_y = m_Cursor.y;
	int grid_centre_x = m_Cursor.x;

	// point coordinates relative to the brush's position
	float brush_grid_x = grid_point_x - grid_centre_x;
	float brush_grid_y = grid_point_y - grid_centre_y;

	if (sqrt(brush_grid_x * brush_grid_x + brush_grid_y * brush_grid_y) <= m_Radius)
		return true;

	return false;
}

//void Brush::ChangeState(uint8_t state)
//{
//	// to this function are passed either STATE_DRAW or STATE_ERASE, if the current state is equal to the argument then the action was to stop drawing or erasing
//	if (state == m_State)
//		m_State = STATE_INACTIVE;
//	else
//		m_State = state;
//}

void Brush::ChangeDrawState(uint8_t state)
{
	// to this function are passed either STATE_DRAW or STATE_ERASE, if the current state is equal to the argument then the action was to stop drawing or erasing
	if (state == m_DrawState)
		m_DrawState = STATE_INACTIVE;
	else
		m_DrawState = state;
}

void Brush::ChangeMouseState(uint8_t state)
{
	// if state changes from STATE_RELEASED to STATE_DRAG, set the cursor position as the m_DragStart field
	if (m_MouseState == STATE_RELEASED && state == STATE_DRAG)
	{
		m_Cursor.drag_start_x = m_Cursor.x;
		m_Cursor.drag_start_y = m_Cursor.y;

		m_Cursor.drag_delta_x = 0.0;
		m_Cursor.drag_delta_y = 0.0;
	}

	m_MouseState = state;
}

//uint8_t Brush::GetState()
//{
//	return m_State;
//}

uint8_t Brush::GetDrawState()
{
	return m_DrawState;
}

uint8_t Brush::GetMouseState()
{
	return m_MouseState;
}

float vector_length(cursor v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}
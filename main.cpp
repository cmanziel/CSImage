#include <stdio.h>
#include <stdlib.h>

#include "Window/Window.h"
#include "Renderer/Renderer.h"

/* TODO:
	* brush radius as shader input variable
	* cursor pos input variable
	* two ways to dispatch:
		- dispatch just one work group of local size also 1, do a for loop inside it that does imageStore on all the pixels around the cursor pos inside a BRUSH_RADIUS area 
		- dispatch BRUSH_RADIUS ^ 2 work groups and calculate uv coordinates based on which invocation of the shader is running
		Base uv coord is vec2(cursor_pos.x - BRUSH_RADIUS, cursor_pos.y - BRUSH_RADIUS), the next ones are baseuv.x + invoc_num, baseuv.y + invoc_num, invoc_num could be gl_GlobalInvocationID
*/

/* TODO: compute shader
	* input variables:
		- the grid of points, the compute shader should be able to modify them
		- brush position and dimension, some kind of structure for the brush buffer
		- brush structure should include the color for the points drawn
		- buffer object for the neighbouring pixels for constructing the matrices for effects like blur and edge detection

	* maybe map the grid of points with glMapBuffer so that the grid data comes to the shader stage modified without more calls to gl functions
	* some "screen refresh" functionality so that the brush could also not leave a trace of points modified
*/

int main()
{
	// Renderer sets up the shader in its constructor
	char path[] = "images/CSimage_0.png";

	Window window(path);
	GLFWwindow* glfwWin = window.GetGLFWwindow();

	Renderer renderer(&window);

	while (!glfwWindowShouldClose(glfwWin))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		window.Update();

		renderer.Draw();

		/* Swap front and back buffers */
		glfwSwapBuffers(glfwWin);

		/* Poll for and process events */
		glfwWaitEvents();
		//glfwPollEvents();
	}

	return 0;
}
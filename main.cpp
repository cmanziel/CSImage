#include <stdio.h>
#include <stdlib.h>

#include "Window/Window.h"
#include "Renderer/Renderer.h"

/* TODO:
	* image load store extension required?
	* iamge variable binding points?
	* float rgba format
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

	Window window(512, 512);
	GLFWwindow* glfwWin = window.GetGLFWwindow();

	Renderer renderer(&window);

	while (!glfwWindowShouldClose(glfwWin))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer.Draw();

		/* Swap front and back buffers */
		glfwSwapBuffers(glfwWin);

		/* Poll for and process events */
		glfwPollEvents();
	}

	return 0;
}
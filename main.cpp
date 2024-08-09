#include <stdio.h>
#include <stdlib.h>

#include "Window/Window.h"
#include "Renderer/Renderer.h"

/* TODO: different programs for different brush states
	* state machine for the renderer
	* keep a current shader field so that its use function will be called accordingly
*/


/* TODO: edge detection on edited image
	* the problem with doing edge detection on the edited image is that imageStore operations get the value of a pixel that has just modified this dispatch of the program
	* whereas it should be done on the result edited image from the last dispatch
	SOLUTION?
	* splitting the process in two: in the first half all the colors for the pixels in the brush area are calculated based on the current state of the image
	* then a second dispatch of the shader does all the imageStore function calls with the calculated values, so that no pixel is calculated based on other ones modified in the current dispatch
	* a shader storage buffer is needed: modify it in the first part and access it in the second
	* buffer mapping?
*/

int main()
{
	// Renderer sets up the shader in its constructor
	//char path[] = "images/drawing_0.png";
	char path[] = "images/block_pointed.png";

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
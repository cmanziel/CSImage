#include <stdio.h>
#include <stdlib.h>

#include "Window/Window.h"
#include "Renderer/Renderer.h"

/* TODO:
	* blur writes to the texture in the same way the sobelShader does: stores in a temporary canvas the last version of the edited image, calcualtes the brush area modified pixels on the temporary canvas, and the modifies every point in the brush area with the values calculated
	* so in the blur shader use ther same brush grid storage buffer and temporary canvas used in the sobelShader by binding to their same binding points
	* for clarity declare the temporary canvas texture and brush grid buffer in the Renderer constructor, or in the ComputeShader base class constructor (which is called by every derived compute shader's constructor so check if they have already been initialized)
*/

int main()
{
	// Renderer sets up the shader in its constructor
	//char path[] = "images/drawing_0.png";
	//char path[] = "images/block_pointed.png";
	char path[] = "images/terrain.png";
	//char path[] = "images/spheres.png";
	//char path[] = "images/screenshot_0.png";
	//char path[] = "images/screenshot_1.png";
	//char path[] = "images/screenshot_2.png";

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
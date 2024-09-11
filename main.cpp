#include <stdio.h>
#include <stdlib.h>

#include "Window/Window.h"
#include "Renderer/Renderer.h"

/* TODO:
	* quad in the center of the window that serves as the "editing area"
	* quad resizes as window resize, till certain dimension
	* map image full size to quad, if it overflows the edges, just map part of the image, if the image is smaller than the quad resize the quad and map full size
	* 
	* do the same but without resizing the quad and just callin glViewport to resize
*/

int main()
{
	// Renderer sets up the shader in its constructor
	//char path[] = "images/drawing_0.png";
	//char path[] = "images/block_pointed.png";
	//char path[] = "images/spheres.png";
	char path[] = "images/screenshot_0.png";
	//char path[] = "images/screenshot_1.png";

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
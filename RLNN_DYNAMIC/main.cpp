
// RLNN_DYNAMIC.cpp : Defines the entry point for the console application.
//
#include "GLAbstraction.h"

#include "Camera/CameraTrack.h"
#include "Camera/CameraManager.h"
#include "Camera/Camera.h"

#include "NeuralNet.h"
//#include "PerspectiveNet.h"

#include "StyleTransferNet.h"


GLFWwindow* window;

double cameraDistance = 2.0;
double zoomIntertia = 0.0;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void initGLFW();
void initGLEW();

void updateCamera();

uint64_t getTimeMilliseconds();


int main()
{
	initGLFW();
	initGLEW();

	StyleTransferNet net;

	while (!glfwWindowShouldClose(window))
	{
		updateCamera();

		net.forwardPass();
		net.render();
		
		GLFramebuffer::flipFlop = !GLFramebuffer::flipFlop;

		glfwSwapBuffers(window);
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
	}

	glfwTerminate();
	return 0;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0) zoomIntertia += std::max(cameraDistance / 50.0, 0.01);
	if (yoffset < 0) zoomIntertia -= std::max(cameraDistance / 100.0, 0.01);
}

void initGLFW()
{
	if (!glfwInit()) exit(0);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1920, 1080, "RLNN Visualizer", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(0);
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetScrollCallback(window, scroll_callback);
}

void initGLEW()
{
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		exit(0);
	}
}

void updateCamera()
{
	cameraDistance += zoomIntertia;
	cameraDistance = std::max(cameraDistance, Camera::z_near * 1.01);
	cameraDistance = std::min(cameraDistance, Camera::z_far * 0.99);
	zoomIntertia /= 1.1;
	Camera::update(window, cameraDistance);

	ProjectionMatrix = Camera::getProjectionMatrix();
	ViewMatrix = Camera::getViewMatrix();
	ModelMatrix = glm::mat4(1.0);
	MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
}

uint64_t getTimeMilliseconds()
{
	return std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
}
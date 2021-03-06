#include "GLAPI.h"

#include "GLFW\glfw3.h"
#include "glm\glm.hpp"

#include "Camera/CameraTrack.h"
#include "Camera/CameraManager.h"
#include "Camera/Camera.h"

#include <chrono>
#include <algorithm>

#include "NeuralArchitecture.h"


GLfloat layerColors[LAYERS * 3] =
{
	1.0, 0.0, 0.0,
	1.0, 1.0, 0.0,
	1.0, 0.0, 1.0,
	0.0, 1.0, 1.0,
	0.0, 1.0, 0.0,
	0.0, 0.0, 1.0,
};

GLfloat quadVertices[18] =
{
	-1.0, -1.0, 0.0,
	 1.0, -1.0, 0.0,
	-1.0,  1.0, 0.0,
	-1.0,  1.0, 0.0,
	 1.0, -1.0, 0.0,
	 1.0,  1.0, 0.0,
};

GLfloat pointVertex[3] =
{
	0.0, 0.0, 0.0
};

GLfloat offsets[3 * MAX_NEURONS];
GLfloat colors[3 * MAX_NEURONS];

GLfloat connectionData[3 * MAX_CONNECTIONS * MAX_NEURONS * 2];

GLuint loadShader(const char * vertex_file_path, const char * fragment_file_path);

GLFWwindow* window;

double cameraDistance = 2.0;
double zoomIntertia = 0.0;

NeuralArchitecture arch;

void buildArchitecture()
{
	NeuralNet* net = arch.createNet();
	NeuralRegion* region = net->createRegion();
	
	NeuralLayerParams layerParams;

	WireCondition neuralWiring = []() -> bool
	{

	};

	WireCondition layerWiring = []() -> bool
	{

	};

	WireCondition regionWiring = []() -> bool
	{

	};

	WireCondition netWiring = []() -> bool
	{

	};

	for (int i = 0; i < 12; i++)
	{
		layerParams.numNeurons = 512 / i;
		region->createLayer(layerParams);
	}

	

	arch.build();
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

glm::mat4 ProjectionMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ModelMatrix;
glm::mat4 MVP;

GLFloatBuffer pointBuffer{ pointVertex, sizeof(pointVertex) };
GLFloatBuffer offsetBuffer{ offsets, sizeof(offsets) };
GLFloatBuffer colorBuffer{ colors, sizeof(colors) };
GLFloatBuffer quadBuffer{ quadVertices, sizeof(quadVertices) };
GLFloatBuffer connectionBuffer{connectionData, sizeof(connectionData)};

GLShader firstShader{ "firstShader" };
GLShader fadeShader{ "fadeShader" };
GLShader lastShader{ "lastShader" };
GLShader synapseShader{ "synapseShader" };

GLFramebufferT<2> firstFramebuffer{ 1920, 1080 };
GLFramebufferT<2> fadeFramebuffer{ 1920, 1080 };

GLComponent firstComponent;
GLComponent fadeComponent;
GLComponent lastComponent;
GLComponent synapseComponent;

void initGLData()
{
	GLVertexArray placeholder;

	{
		pointBuffer.init();
		offsetBuffer.init();
		colorBuffer.init();
		quadBuffer.init();

		connectionBuffer.init();
	}

	{
		firstShader.addVar("MVP").code = [&]() {glUniformMatrix4fv(firstShader.getVarId("MVP"), 1, GL_FALSE, &MVP[0][0]);};

		firstShader.init();
	}

	{
		synapseShader.addVar("MVP").code = [&]() {glUniformMatrix4fv(firstShader.getVarId("MVP"), 1, GL_FALSE, &MVP[0][0]); };

		synapseShader.init();
	}

	{
		fadeShader.addVar("newData").code = [&]() {glUniform1i(fadeShader.getVarId("newData"), 0); };
		fadeShader.addVar("oldData").code = [&]() {glUniform1i(fadeShader.getVarId("oldData"), 1); };

		fadeShader.init();
	}

	{
		lastShader.addVar("framebuffer_data").code = [&]() {glUniform1i(firstShader.getVarId("framebuffer_data"), 0); };

		lastShader.init();
	}

	{
		firstFramebuffer.init();
		fadeFramebuffer.init();
	}

	{
		firstComponent.shader = &firstShader;
		firstComponent.framebuffer = &firstFramebuffer;

		firstComponent.addAttrib(pointBuffer.getId(), false);
		firstComponent.addAttrib(offsetBuffer.getId(), true);
		firstComponent.addAttrib(colorBuffer.getId(), true);

		firstComponent.postDraw = [&](int64_t index)
		{
			glDrawArraysInstanced(GL_POINTS, 0, 3, MAX_NEURONS);
		};
	}
	{
		synapseComponent.shader = &synapseShader;
		synapseComponent.framebuffer = &firstFramebuffer;

		synapseComponent.addAttrib(connectionBuffer.getId(), false);

		synapseComponent.postDraw = [&](int64_t index)
		{ 
			glDrawArrays(GL_LINES, 0, MAX_NEURONS * MAX_CONNECTIONS * 2);
		};
	}
	{
		fadeComponent.shader = &fadeShader;
		fadeComponent.framebuffer = &fadeFramebuffer;
		fadeComponent.prepTextures = [&]()
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, firstFramebuffer.currentOutput());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, fadeFramebuffer.previousOutput());
		};

		fadeComponent.addAttrib(quadBuffer.getId(),false);
	}

	{
		lastComponent.shader = &lastShader;
		lastComponent.prepTextures = [&]()
		{
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fadeFramebuffer.currentOutput());
		};

		lastComponent.addAttrib(quadBuffer.getId(),false);

	}

	{
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glEnable(GL_BLEND);
		glPointSize(4);
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
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

int main()
{
	buildArchitecture();

	initGLFW();
	initGLEW();
	initGLData();

	GLFramebuffer::flipFlop = false;

	while (!glfwWindowShouldClose(window))
	{
		updateCamera();

		glDisable(GL_DEPTH_TEST);
		synapseComponent.draw(GLFramebuffer::flipFlop, true);
		firstComponent.draw(GLFramebuffer::flipFlop);// , true);
		glEnable(GL_DEPTH_TEST);
		fadeComponent.draw(GLFramebuffer::flipFlop);
		lastComponent.draw(0, true);
		
		GLFramebuffer::flipFlop = !GLFramebuffer::flipFlop;

		glfwSwapBuffers(window);

		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
	}

	glfwTerminate();
	return 0;
}

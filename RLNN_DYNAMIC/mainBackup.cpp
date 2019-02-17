#if 0

// RLNN_DYNAMIC.cpp : Defines the entry point for the console application.
//
#include "GLAbstraction.h"

#include "Camera/CameraTrack.h"
#include "Camera/CameraManager.h"
#include "Camera/Camera.h"

#include "NeuralNet.h"
#include "PerspectiveNet.h"


void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stdout, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}


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

GLFWwindow* window;

double cameraDistance = 2.0;
double zoomIntertia = 0.0;

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
GLFloatBuffer connectionBuffer{ connectionData, sizeof(connectionData) };

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
		firstShader.addVar("MVP").code = [&]() {glUniformMatrix4fv(firstShader.getVarId("MVP"), 1, GL_FALSE, &MVP[0][0]); };

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

		fadeComponent.addAttrib(quadBuffer.getId(), false);
	}

	{
		lastComponent.shader = &lastShader;
		lastComponent.prepTextures = [&]()
		{

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fadeFramebuffer.currentOutput());
		};

		lastComponent.addAttrib(quadBuffer.getId(), false);

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

////////////////////////////////////////////////////////////////////////////////////
int main()
{
	/*NeuralNet net;
	net.generate((uint32_t) getTimeMilliseconds());
	net.wire();

	for (int i = 0; i < MAX_NEURONS; i++)
	{
		offsets[i * 3 + 0] = (GLfloat) net.neurons[i].x;
		offsets[i * 3 + 1] = (GLfloat) net.neurons[i].y;
		offsets[i * 3 + 2] = (GLfloat) net.neurons[i].z;

		colors[i * 3 + 0] = layerColors[net.neurons[i].layer * 3 + 0];
		colors[i * 3 + 1] = layerColors[net.neurons[i].layer * 3 + 1];
		colors[i * 3 + 2] = layerColors[net.neurons[i].layer * 3 + 2];

		Neuron* n1 = &net.neurons[i];
		for (int j = 0; j < MAX_CONNECTIONS; j++)
		{
			Neuron* n2 = n1->connections[j];

			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 0] = n1->x;
			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 1] = n1->y;
			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 2] = n1->z;

			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 3] = n2->x;
			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 4] = n2->y;
			connectionData[i * MAX_CONNECTIONS * 6 + j * 6 + 5] = n2->z;
		}
	}*/

	initGLFW();
	initGLEW();
	initGLData();

	PerspectiveNet net;

	while (!glfwWindowShouldClose(window))
	{
		updateCamera();

		/*glDisable(GL_DEPTH_TEST);
		synapseComponent.draw(GLFramebuffer::flipFlop, true);
		firstComponent.draw(GLFramebuffer::flipFlop);// , true);
		glEnable(GL_DEPTH_TEST);
		fadeComponent.draw(GLFramebuffer::flipFlop);
		lastComponent.draw(0, true);

		GLFramebuffer::flipFlop = !GLFramebuffer::flipFlop;*/

		glfwSwapBuffers(window);

		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
	}

	glfwTerminate();
	return 0;
}


#endif

// RLNN_DYNAMIC.cpp : Defines the entry point for the console application.
//
#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include "glm\glm.hpp"

#include "Camera/CameraTrack.h"
#include "Camera/CameraManager.h"
#include "Camera/Camera.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <algorithm>

#include "NeuralNet.h"

GLfloat g_vertex_buffer_data[3] = { 0.0, 0.0, 0.0 };

GLfloat layerColors[LAYERS * 3] =
{
	1.0, 0.0, 0.0,
	1.0, 1.0, 0.0,
	1.0, 0.0, 1.0,
	0.0, 1.0, 1.0,
	0.0, 1.0, 0.0,
	0.0, 0.0, 1.0,
};

GLfloat offsets[3 * MAX_NEURONS];
GLfloat colors[3 * MAX_NEURONS];

GLuint loadShader(const char * vertex_file_path, const char * fragment_file_path);

//CameraManager* cam_manager;
GLFWwindow* window;

double radius = 2.0;
double inertia = 0.0;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0) inertia += std::max(radius / 50.0, 0.01);
	if (yoffset < 0) inertia -= std::max(radius / 100.0, 0.01);
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
GLuint instanceColorVBO;
GLuint instanceVBO;
GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint firstShader;
GLuint lastShader;
GLuint ViewMatrixID;
GLuint MatrixID;
GLuint ModelMatrixID;
GLuint composite_fbo;
GLuint composite_text_output_texture;
GLuint composite_text_inputID;
GLuint quad_vertexbuffer;
GLuint firstShaderInputTexID;

glm::mat4 ProjectionMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ModelMatrix;
glm::mat4 MVP;

void initGLData()
{
	/*for (int i = 0; i < 3; i++)
	{
		double angle = i * 120.0 + 90.0;

		double x = cos(angle * 3.14159265358979323846 / 180.0) / 10.0;
		double y = sin(angle * 3.14159265358979323846 / 180.0) / 10.0;

		g_vertex_buffer_data[i * 3 + 0] = x;
		g_vertex_buffer_data[i * 3 + 1] = y;
		g_vertex_buffer_data[i * 3 + 2] = 0.0;
	}*/

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(offsets), offsets, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &instanceColorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceColorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	{
		firstShader = loadShader("shaders/firstShader.vert", "shaders/firstShader.frag");

		MatrixID = glGetUniformLocation(firstShader, "MVP");
		ViewMatrixID = glGetUniformLocation(firstShader, "V");
		ModelMatrixID = glGetUniformLocation(firstShader, "M");
		
		firstShaderInputTexID = glGetUniformLocation(firstShader, "framebuffer_data");
	}
	{
		lastShader = loadShader("shaders/lastShader.vert", "shaders/lastShader.frag");
		composite_text_inputID = glGetUniformLocation(lastShader, "framebuffer_data");
	}
	{
		static const GLbyte quad_vertexbuffer_data[] =
		{
			-1, -1, 0,
			 1, -1, 0,
			-1,  1, 0,
			-1,  1, 0,
			 1, -1, 0,
			 1,  1, 0,
		};

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertexbuffer_data), quad_vertexbuffer_data, GL_STATIC_DRAW);
	}
	glViewport(0, 0, 1920, 1080);

	glEnable(GL_DEPTH_TEST);

	glGenFramebuffers(1, &composite_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, composite_fbo);

	glGenTextures(1, &composite_text_output_texture);
	glBindTexture(GL_TEXTURE_2D, composite_text_output_texture);
	std::vector<uint8_t> zero_init(1920 * 1080 * 3, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, &zero_init[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, composite_text_output_texture, 0);

	GLenum raw_DB = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &raw_DB);

}

void updateCamera()
{
	radius += inertia;
	radius = std::max(radius, Camera::z_near * 1.01);
	radius = std::min(radius, Camera::z_far * 0.99);
	inertia /= 1.1;
	Camera::update(window, radius);

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
	uint64_t numNeurons = MAX_NEURONS;

	NeuralNet net;
	net.generate(getTimeMilliseconds());

	for (int i = 0; i < numNeurons; i++)
	{
		//offsets[i * 3 + 0] = -1.0 + (float(rand()) / float(RAND_MAX)) * 2.0;
		//offsets[i * 3 + 1] = -1.0 + (float(rand()) / float(RAND_MAX)) * 2.0;
		//offsets[i * 3 + 2] = -1.0 + (float(rand()) / float(RAND_MAX)) * 2.0;

		offsets[i * 3 + 0] = net.neurons[i].x;
		offsets[i * 3 + 1] = net.neurons[i].y;
		offsets[i * 3 + 2] = net.neurons[i].z;

		colors[i * 3 + 0] = layerColors[net.neurons[i].layer * 3 + 0];
		colors[i * 3 + 1] = layerColors[net.neurons[i].layer * 3 + 1];
		colors[i * 3 + 2] = layerColors[net.neurons[i].layer * 3 + 2];
	}

	initGLFW();
	initGLEW();
	initGLData();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		
		updateCamera();

		static bool draw = true;

		//if (draw)
		{
			glUseProgram(firstShader);
			glBindFramebuffer(GL_FRAMEBUFFER, composite_fbo);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, composite_text_output_texture);
			glUniform1i(composite_text_inputID, 0);

			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glVertexAttribDivisor(1, 1);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, instanceColorVBO);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glVertexAttribDivisor(2, 1);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glPointSize(1.5);
			glDrawArraysInstanced(GL_POINTS, 0, 3, numNeurons); // Starting from vertex 0; 3 vertices total -> 1 triangle
			glDisableVertexAttribArray(0);
		}

		draw = !draw;

		{
			glUseProgram(lastShader);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, composite_text_output_texture);
			glUniform1i(composite_text_inputID, 0);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
			glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, 0, (void*)0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(0);
		}
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
	}

	glfwTerminate();
	return 0;
}

GLuint loadShader(const char * vertex_file_path, const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
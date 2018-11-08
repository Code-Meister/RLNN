
// RLNN_DYNAMIC.cpp : Defines the entry point for the console application.
//
#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include "glm\glm.hpp"

#include "Camera/CameraTrack.h"
#include "Camera/CameraManager.h"
#include "Camera/Camera.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <functional>

#include "NeuralNet.h"

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

GLuint loadShader(const char * vertex_file_path, const char * fragment_file_path);

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

#define genBuffer(buffer, bufferData, size) \
glGenBuffers(1, &buffer);\
glBindBuffer(GL_ARRAY_BUFFER, buffer);\
glBufferData(GL_ARRAY_BUFFER, size, bufferData, GL_STATIC_DRAW);\
glBindBuffer(GL_ARRAY_BUFFER, 0)

class GLVertexArray
{
	GLuint vertexArrayID;
public:
	GLVertexArray()
	{
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);
	}
};

class GLFloatBuffer
{
	bool _init = false;
	GLuint bufferId;
	GLfloat* buffer = nullptr;
	size_t size;

public:
	void init()
	{
		_init = true;
		genBuffer(bufferId, buffer, size);
	}

	GLFloatBuffer(GLfloat* buffer, size_t size) : buffer(buffer), size(size) {}

	~GLFloatBuffer()
	{
		if (_init) glDeleteBuffers(1, &bufferId);
	}

	GLuint getId()
	{
		return bufferId;
	}
};

class GLFramebuffer
{
protected:
	bool _init = false;
	GLsizei width;
	GLsizei height;

public:
	static bool flipFlop;

	GLFramebuffer(GLsizei width, GLsizei height) : width(width), height(height) {}

	GLsizei getWidth()
	{
		return width;
	}

	GLsizei getHeight()
	{
		return height;
	}

	virtual GLuint currentOutput()
	{
		return 0;
	}

	virtual GLuint previousOutput()
	{
		return 0;
	}

	virtual ~GLFramebuffer() {}

	virtual void init() {}

	virtual GLuint getFboId(int64_t index)
	{
		return -1;
	}

	virtual size_t getCount()
	{
		return 0;
	}
};

bool GLFramebuffer::flipFlop = false;

template <uint8_t count>
class GLFramebufferT : public GLFramebuffer
{
	GLuint fboId[count];
	GLuint outputTextureId[count];
	GLuint drawBufferId[count];
public:
	GLFramebufferT(GLsizei width, GLsizei height) : GLFramebuffer(width, height) {}

	virtual ~GLFramebufferT()
	{
		if (!_init) return;

		for (int i = 0; i < count; i++)
		{
			glDeleteFramebuffers(1, &fboId[i]);
			glDeleteTextures(1, &outputTextureId[i]);
		}

	}

	virtual void init()
	{
		_init = true;

		for (int i = 0; i < count; i++)
		{
			glGenFramebuffers(1, &fboId[i]);
			glBindFramebuffer(GL_FRAMEBUFFER, fboId[i]);

			glGenTextures(1, &outputTextureId[i]);
			glBindTexture(GL_TEXTURE_2D, outputTextureId[i]);
			std::vector<uint8_t> zero_init(width * height * 3, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &zero_init[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, outputTextureId[i], 0);

			drawBufferId[i] = GL_COLOR_ATTACHMENT0;
			glDrawBuffers(1, &drawBufferId[i]);
		}
	}

	virtual GLuint getFboId(int64_t index) override
	{
		return fboId[index];
	}

	virtual size_t getCount() override
	{
		return count;
	}

	virtual GLuint currentOutput() override
	{
		if (count == 1) return outputTextureId[0];
		
		return flipFlop ? outputTextureId[0] : outputTextureId[1];
	}

	virtual GLuint previousOutput() override
	{
		if (count == 1) return outputTextureId[0];
		//std::cout << outputTextureId[flipFlop] << ", " << outputTextureId[!flipFlop] << std::endl;
		return flipFlop ? outputTextureId[1] : outputTextureId[0];
	}
};

class GLShader
{
	bool _init = false;

	std::string name;

	struct ShaderVar
	{
		GLuint uniformID;
		std::string name;

		std::function<void()> code = []() {};

		ShaderVar(std::string name) : name(name) {}
	};

	std::vector<ShaderVar> shaderVars;
	

	GLuint shaderId;

public:
	GLShader(std::string name) : name(name) {}

	void init()
	{
		_init = true;

		shaderId = loadShader(std::string("shaders/" + name + ".vert").c_str(), std::string("shaders/" + name + ".frag").c_str());

		for (ShaderVar& var : shaderVars)
		{
			var.uniformID = glGetUniformLocation(shaderId, var.name.c_str());

			std::cout << var.name.c_str() << std::endl;

			volatile int i = 0;

			//std::cout << i;
		}
	}

	ShaderVar& addVar(std::string name)
	{
		if (!_init) shaderVars.push_back(ShaderVar(name));
		for (ShaderVar& var : shaderVars)
		{
			if (var.name == name) return var;
		}
	}

	GLuint getVarId(std::string name)
	{
		for (ShaderVar& var : shaderVars)
		{
			if (var.name == name)
			{
				volatile GLuint id = var.uniformID;
				return id;
			}
		}

		return -1;
	}

	GLuint getId()
	{
		return shaderId;
	}

	void runVars()
	{
		for (ShaderVar& var : shaderVars)
		{
			var.code();
		}
	}
};

class GLAttribArray
{
	GLuint index;
	GLint size = 3;
	GLenum type = GL_FLOAT;
	GLboolean normalized = GL_FALSE;
	GLsizei stride = 3 * sizeof(GLfloat);
	GLuint division = 1;
	const GLvoid* pointer = nullptr;

	GLuint bufferId;

	GLboolean instanced;
public:
	GLAttribArray(GLuint index, GLuint bufferId, bool instanced) : index(index), bufferId(bufferId), instanced(instanced) {}
	
	void run()
	{
		glEnableVertexAttribArray(index);
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
		glVertexAttribPointer(index, size, type, normalized, stride, pointer);
		if (instanced) glVertexAttribDivisor(index,division);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void stop()
	{
		glDisableVertexAttribArray(index);
	}

	GLuint getId()
	{
		return bufferId;
	}
};

class GLTexture
{
	GLuint textureId;
	GLuint index;

public:
	GLTexture(GLuint index, GLuint textureId) : index(index), textureId(textureId) {}

	GLuint getId()
	{
		return textureId;
	}

	void run()
	{
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, textureId);
	}
};

struct GLComponent
{
	GLShader* shader = nullptr;
	GLFramebuffer* framebuffer = nullptr;

	std::function<void(int64_t)> postDraw = [](int64_t index) {glDrawArrays(GL_TRIANGLES, 0, 6); };
	std::function<void()> prepTextures = []() {};
	
	std::vector<GLAttribArray> attribArrays;
	//std::vector<GLTexture> textures;

	GLAttribArray& addAttrib(GLuint bufferId, bool instanced)
	{
		GLuint index = (GLuint) attribArrays.size();
		attribArrays.push_back(GLAttribArray{index, bufferId, instanced});

		for (GLAttribArray& attribArray : attribArrays)
		{
			if (attribArray.getId() == bufferId) return attribArray;
		}

	}

	/*GLTexture& addInputTexture(GLuint textureId)
	{
		GLuint index = textures.size();
		textures.push_back(GLTexture{ index, textureId });

		for (GLTexture& texture : textures)
		{
			if (textures.getId() == textureId) return texture;
		}
	}*/

	void draw(int64_t index, bool clear = false)
	{
		if (framebuffer)
		{
			volatile int64_t count = framebuffer->getCount();
			if (index >= count)
			{
				std::cout << "Error. Framebuffer index out of bounds";
				exit(0);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->getFboId(index));
			glViewport(0, 0, framebuffer->getWidth(), framebuffer->getHeight());
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, 1920, 1080);
		}

		if (clear) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (shader)
		{
			glUseProgram(shader->getId());

			shader->runVars();
		}

		prepTextures();

		for (GLAttribArray& attribArray : attribArrays)
		{
			attribArray.run();
		}

		postDraw(index);

		for (GLAttribArray& attribArray : attribArrays)
		{
			attribArray.stop();
		}
	}
};

GLFloatBuffer pointBuffer{ pointVertex, sizeof(pointVertex) };
GLFloatBuffer offsetBuffer{ offsets, sizeof(offsets) };
GLFloatBuffer colorBuffer{ colors, sizeof(colors) };
GLFloatBuffer quadBuffer{ quadVertices, sizeof(quadVertices) };

GLShader firstShader{ "firstShader" };
GLShader fadeShader{ "fadeShader" };
GLShader lastShader{ "lastShader" };

GLFramebufferT<2> firstFramebuffer{ 1920, 1080 };
GLFramebufferT<2> fadeFramebuffer{ 1920, 1080 };

GLComponent firstComponent;
GLComponent fadeComponent;
GLComponent lastComponent;

void initGLData()
{
	GLVertexArray placeholder;

	{
		pointBuffer.init();
		offsetBuffer.init();
		colorBuffer.init();
		quadBuffer.init();
	}

	{
		firstShader.addVar("MVP").code = [&]() {glUniformMatrix4fv(firstShader.getVarId("MVP"), 1, GL_FALSE, &MVP[0][0]);};

		firstShader.init();
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
		glEnable(GL_BLEND);
		glPointSize(1.5);
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
	NeuralNet net;
	net.generate((uint32_t) getTimeMilliseconds());

	for (int i = 0; i < MAX_NEURONS; i++)
	{
		offsets[i * 3 + 0] = (GLfloat) net.neurons[i].x;
		offsets[i * 3 + 1] = (GLfloat) net.neurons[i].y;
		offsets[i * 3 + 2] = (GLfloat) net.neurons[i].z;

		colors[i * 3 + 0] = layerColors[net.neurons[i].layer * 3 + 0];
		colors[i * 3 + 1] = layerColors[net.neurons[i].layer * 3 + 1];
		colors[i * 3 + 2] = layerColors[net.neurons[i].layer * 3 + 2];
	}

	initGLFW();
	initGLEW();
	initGLData();

	GLFramebuffer::flipFlop = false;

	while (!glfwWindowShouldClose(window))
	{
		updateCamera();
		firstComponent.draw(GLFramebuffer::flipFlop, true);
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

////////////////////////////////////////////////////////////////////////////////////

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
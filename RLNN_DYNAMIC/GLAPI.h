#pragma once
#include "GL\glew.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <functional>

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
		if (instanced) glVertexAttribDivisor(index, division);
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
		GLuint index = (GLuint)attribArrays.size();
		attribArrays.push_back(GLAttribArray{ index, bufferId, instanced });

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
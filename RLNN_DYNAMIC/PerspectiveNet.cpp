#include "PerspectiveNet.h"

#include "GLAbstraction.h"

#pragma warning(disable:4996)

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

GLFloatBuffer pointBuffer{ pointVertex, sizeof(pointVertex) };
GLFloatBuffer quadBuffer{ quadVertices, sizeof(quadVertices) };

GLShader errorShader{ "errorShader" };
GLShader renderShader{ "renderShader" };

GLFramebufferT<1> internalRenderFramebuffer{ 300, 300 };
GLFramebufferT<2> renderFramebuffer{ 300, 300 };
GLFramebufferT<1> errorFramebuffer{ 300, 300 };

GLComponent internalRenderComponent;
GLComponent renderComponent;
GLComponent errorComponent;

void PerspectiveNet::initGLData()
{
	GLVertexArray placeholder;

	{
		pointBuffer.init();
		quadBuffer.init();
	}

	{
		renderShader.addVar("MVP").code = [&]() {glUniformMatrix4fv(renderShader.getVarId("MVP"), 1, GL_FALSE, &MVP[0][0]); };

		renderShader.init();
	}

	{
		errorShader.addVar("MVP").code = [&]() {glUniformMatrix4fv(errorShader.getVarId("MVP"), 1, GL_FALSE, &MVP[0][0]); };

		errorShader.init();
	}

	{
		renderComponent.shader = &renderShader;
		renderComponent.framebuffer = &renderFramebuffer;

		renderComponent.addAttrib(pointBuffer.getId(), false);
		//renderComponent.addAttrib(offsetBuffer.getId(), true);
		//renderComponent.addAttrib(colorBuffer.getId(), true);

		renderComponent.postDraw = [&](int64_t index)
		{
			//glDrawArraysInstanced(GL_POINTS, 0, 3, MAX_NEURONS);
		};
	}

	{
		internalRenderComponent.shader = &renderShader;
		internalRenderComponent.framebuffer = &internalRenderFramebuffer;

		internalRenderComponent.addAttrib(pointBuffer.getId(), false);
		//internalRenderComponent.addAttrib(offsetBuffer.getId(), true);
		//internalRenderComponent.addAttrib(colorBuffer.getId(), true);

		internalRenderComponent.postDraw = [&](int64_t index)
		{
			//glDrawArraysInstanced(GL_POINTS, 0, 3, MAX_NEURONS);
		};
	}

	{
		errorComponent.shader = &errorShader;
		errorComponent.framebuffer = &errorFramebuffer;

		errorComponent.postDraw = [&](int64_t index)
		{
			//glDrawArraysInstanced(GL_POINTS, 0, 3, MAX_NEURONS);
		};
	}

	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glPointSize(1);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
	}
}

PerspectiveNet::PerspectiveNet()
{
	const String base = "C:\\Users\\Austin\\Desktop\\3d vision\\Blender\\images\\";

	for (int i = 1; i <= 540; i++)
	{
		char fileNumber[5];
		sprintf(fileNumber, "%04d", i);

		String filename = base + "frame" + String(fileNumber) + ".bmp";

		Perspective<300,300>* perspective = new Perspective<300, 300>{ filename };

		perspectives.push_back(perspective);
	}

	initGLData();
}

PerspectiveNet::~PerspectiveNet()
{
	for (Perspective<300, 300>* perspective : perspectives)
	{
		delete perspective;
	}
}

void PerspectiveNet::forwardPass()
{
	for (int i = 0; i < 2; i++)
	{
		internalRenderComponent.draw(0, true);
		errorComponent.draw(0, true);
	}
}

void PerspectiveNet::render()
{
	for (int i = 0; i < 2; i++)
	{
		renderComponent.draw(GLFramebuffer::flipFlop, true);
	}
}

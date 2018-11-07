#pragma once
#include <glm\gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>

namespace Camera
{
	extern glm::vec3 position;
	extern glm::mat4 view_matrix;

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	const float z_near = 0.001f;
	const float z_far = 1000.0f;

	extern float aspect_ratio;

	extern float fov;
	extern float speed;
	extern float mouse_speed;

	void setAspectRatio(float aspect_ratio);

	void update(GLFWwindow* window, double radius);
};


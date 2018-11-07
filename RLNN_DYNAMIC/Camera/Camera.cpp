#include "Camera.h"
#include "glm/gtx/rotate_vector.hpp"
#include "iostream"

float Camera::fov = 45.0;
float Camera::speed = 0.3f;
float Camera::mouse_speed = 0.5f;
float Camera::aspect_ratio = 16.0f / 9.0f;

glm::vec3 Camera::position;
glm::mat4 Camera::view_matrix;

glm::mat4 Camera::getViewMatrix()
{
	return view_matrix;
}
glm::mat4 Camera::getProjectionMatrix()
{
	static glm::mat4 ret = glm::perspective(fov, aspect_ratio, z_near, z_far);
	return ret;
}

void Camera::setAspectRatio(float aspect_ratio)
{
	Camera::aspect_ratio = aspect_ratio;
}

void Camera::update(GLFWwindow* window, double radius)
{
	static double horAngle = 0.0;
	static double verAngle = 90.0;
	static const double degToRad = 3.14159265 / 180.0;

	int width = 0;
	int height = 0;

	glfwGetWindowSize(window, &width, &height);

	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	static bool init = false;

	if (!init)
	{
		glfwSetCursorPos(window, width / 2, height / 2);
		init = true;
	}
	
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwSetCursorPos(window, width / 2, height / 2);

	horAngle += -1.0f;// mouse_speed * float(width / 2 - xpos);
	verAngle += mouse_speed * float(height / 2 - ypos);

	glm::vec3 up = glm::vec3(0, 1, 0);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) verAngle += speed;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) verAngle -= speed;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) horAngle -= speed;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) horAngle += speed;
	
	horAngle = fmod(horAngle + 360.0, 360.0);
	verAngle = (verAngle > 179.9) ? 179.9 : verAngle;
	verAngle = (verAngle < 0.01) ? 0.01 : verAngle;

	position.z = radius * sin(verAngle * degToRad) * cos(horAngle * degToRad);
	position.x = radius * sin(verAngle * degToRad) * sin(horAngle * degToRad);
	position.y = radius * cos(verAngle * degToRad);

	//std::cout << horAngle << ", " << verAngle << std::endl;

	view_matrix = glm::lookAt(position, glm::vec3(0.0f), up);

	lastTime = currentTime;
}

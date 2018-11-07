#pragma once
#include <glm/glm.hpp>
#include <vector>

class CameraTrack
{
public:
	CameraTrack();
	CameraTrack(std::vector<glm::vec3> points);
	~CameraTrack();

	void addPoint(glm::vec3 point);
	void addPoints(std::vector<glm::vec3> points);
	glm::vec3 getInterpLoc(float percent);
	float getDistance();

private:
	std::vector<glm::vec3> bezier_points;
	void calculateDistance();
	float distance = 0.0f;
};


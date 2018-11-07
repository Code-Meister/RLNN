#include "CameraTrack.h"
#include <functional>

CameraTrack::CameraTrack() {}

CameraTrack::CameraTrack(std::vector<glm::vec3> points) {addPoints(points);}

CameraTrack::~CameraTrack(){}

void CameraTrack::addPoint(glm::vec3 point)
{
	bezier_points.push_back(point);
	calculateDistance();
}

void CameraTrack::addPoints(std::vector<glm::vec3> points)
{
	for (glm::vec3 point : points)
	{
		bezier_points.push_back(point);
	}

	calculateDistance();
}

glm::vec3 CameraTrack::getInterpLoc(float percent)
{
	if (bezier_points.size() <= 1) return glm::vec3();

	std::function<std::vector<glm::vec3>(std::vector<glm::vec3>)> recursive = [&](std::vector<glm::vec3> points) -> std::vector<glm::vec3>
	{
		if (points.size() == 1) return points;
		std::vector<glm::vec3> new_points;
		for (size_t i = 0; i < points.size() - 1; i++)
		{
			glm::vec3 p1 = points.at(i);
			glm::vec3 p2 = points.at(i + 1);

			new_points.push_back(p1 + (p2 - p1)*percent);
		}

		return recursive(new_points);
	};

	return recursive(bezier_points).at(0);
}

float CameraTrack::getDistance()
{
	return distance;
}

void CameraTrack::calculateDistance()
{
	distance = 0.0f;
	if (bezier_points.empty()) return;

	uint16_t samples = 128;
	std::vector<glm::vec3> points;

	for (size_t i = 0; i < samples; i++)
	{
		glm::vec3 point = getInterpLoc(float(i) / samples);
		points.push_back(point);
	}

	for (size_t i = 0; i < points.size() - 1; i++)
	{
		glm::vec3 p1 = points.at(i);
		glm::vec3 p2 = points.at(i + 1);
		distance += sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2) + pow(p2.z - p1.z, 2));
	}
}
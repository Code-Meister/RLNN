#include "CameraManager.h"

#include "Camera.h"

#include <algorithm>
#include <chrono>

CameraManager::CameraManager() {}
CameraManager::~CameraManager() {}

void CameraManager::update()
{
	if (moving)
	{
		uint64_t elapsed = getTimeMilliseconds() - start_time;
		if (elapsed > duration_ms)
		{
			moving = false;
			elapsed = duration_ms;
		}
		float percent = (!forward)+getDistance(std::min(float(elapsed) / duration_ms,1.0f))*(forward? 1:-1);
		Camera::position = pos_track->getInterpLoc(percent);
		//Camera::direction = look_track->getInterpLoc(percent);
	}
}

void CameraManager::setPositionTrack(CameraTrack* track)
{
	pos_track = track;
}

void CameraManager::setLookTrack(CameraTrack* track)
{
	look_track = track;
}

void CameraManager::initCamera()
{
	Camera::position = pos_track->getInterpLoc(0);
	//Camera::direction = look_track->getInterpLoc(0);
}

void CameraManager::setDuration(uint64_t duration)
{
	this->duration_ms = duration;
}

void CameraManager::move(bool forward)
{
	this->forward = forward;
	moving = true;
	start_time = getTimeMilliseconds();
}
uint64_t CameraManager::getTimeMilliseconds()
{
	return std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
}

void CameraManager::setMode(Mode mode)
{
	this->mode = mode;
}

float CameraManager::getDistance(float elapsed_percent)
{
	return pow(elapsed_percent, 4);
}

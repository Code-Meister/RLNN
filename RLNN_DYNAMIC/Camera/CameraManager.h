#pragma once
#include "CameraTrack.h"

class CameraManager
{
public:
	CameraManager();
	~CameraManager();

	void update();

	CameraTrack* pos_track = nullptr;
	CameraTrack* look_track = nullptr;

	void setPositionTrack(CameraTrack* track);
	void setLookTrack(CameraTrack* track);

	void initCamera();
	void setDuration(uint64_t duration);
	void move(bool forward);

	uint64_t getTimeMilliseconds();

	enum Mode
	{
		FIXED,
		FREE,
		TRACKED
	} mode;
	void setMode(Mode mode);

	float getDistance(float elapsed_percent);
	
private:
	float distance = 0.0f;
	bool moving = false;
	bool forward = true;

	uint64_t duration_ms = 4000;
	uint64_t start_time = 0;
};


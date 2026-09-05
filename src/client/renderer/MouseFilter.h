#pragma once

class MouseFilter
{
private:
	float accumulated = 0.0f;
	float applied = 0.0f;
	float smoothed = 0.0f;

public:
	float smooth(float value, float amount);
};

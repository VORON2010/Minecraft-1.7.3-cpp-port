#include "client/renderer/MouseFilter.h"

float MouseFilter::smooth(float value, float amount)
{
	accumulated += value;
	value = (accumulated - applied) * amount;
	smoothed += (value - smoothed) * 0.5f;
	if ((value > 0.0f && value > smoothed) || (value < 0.0f && value < smoothed))
		value = smoothed;
	applied += value;
	return value;
}

#pragma once

class ITaskMonitor
{
public:
	virtual bool isWorking() {
		return getProgress() != 1.0;
	}
	virtual float getProgress() = 0;
};
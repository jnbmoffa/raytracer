#pragma once
#include "Thread.h"
#include <iostream>

class ProgressThread : public Thread
{
	double Total;

public:
	atomic_int PROGRESS;
	
	ProgressThread(const double& Total) : Total(Total), PROGRESS(0) {}

	virtual void Main() override
	{
		 // Status bar
		while (PROGRESS < Total)
		{
			std::cout << std::fixed << (double)(PROGRESS*100)/Total << "\%\xd"; std::cout.flush();
			SleepMicro(50000);
		}
	}
};
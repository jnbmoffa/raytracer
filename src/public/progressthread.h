#pragma once
#include "Thread.h"
#include <iostream>

// Shows a percentage based on the Total and the PROGRESS variable
class ProgressThread : public Thread
{
	double Total;

public:
	atomic_int PROGRESS;
	
	explicit ProgressThread(const double& Total) : Total(Total), PROGRESS(0) {}

	virtual void Main() override
	{
		 // Status bar
		std::cout.precision(2);
		while (PROGRESS < Total)
		{
			std::cout << std::fixed << (double)(PROGRESS*100)/Total << "\%\xd"; std::cout.flush();
			SleepMicro(50000);
		}
	}
};
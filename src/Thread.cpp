#include "Thread.h"

void Thread::Yield( unsigned int TimesToYield )
{
	unsigned int TimesYielded = 0;
	while (TimesYielded++ < TimesToYield)
	{
		std::this_thread::yield();
	}
}

Thread::Thread()
{
}

Thread::~Thread()
{
	Join();
	delete ActualThread;
}

void Thread::Start()
{
	ActualThread = new std::thread( &Thread::Main, this );
}

void Thread::Join()
{
	if ( ActualThread->joinable() )
	{
		ActualThread->join();
	}
}

ThreadID Thread::ID()
{
	return ActualThread->get_id();
}

void SleepMicro(unsigned int Microseconds)
{
	std::this_thread::sleep_for(std::chrono::microseconds(Microseconds));
}
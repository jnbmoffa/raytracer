#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>

using atomic_int = std::atomic<int>;

using ThreadID = std::thread::id;

// Mutual exclusion
using MutexLock = std::mutex;
using OwnerLock = std::recursive_mutex;

// Synchronization
using CondLock = std::condition_variable_any;

using ScopeLock = std::unique_lock<MutexLock>;
using ScopeOwnerLock = std::unique_lock<OwnerLock>;

// Number of available cores
inline unsigned int GetNumCores()
{
	return std::thread::hardware_concurrency();
}

// Wrapper around C++11 threads that makes them nicer to use
class Thread
{
	std::thread* ActualThread;

protected:
	void Yield( unsigned int TimesToYield = 1 );

public:
	Thread();
	Thread( Thread& T ) = delete;
	~Thread();

	/** The function that runs in the thread */
	virtual void Main() = 0;
	void Start();
	void Join();
	ThreadID ID();

	Thread& operator=( Thread& T ) = delete;
};

/** Construct and start a new thread of the provided type */
template<typename ThreadType, typename... Args>
std::unique_ptr<ThreadType> CreateThread(Args&&... args)
{
	std::unique_ptr<ThreadType> NewThread(
		std::make_unique<ThreadType>(std::forward<decltype(args)>(args)...)
		);
	NewThread->Start();
	return NewThread;
}

// The current thread will sleep for the provided number of microseconds
void SleepMicro(unsigned int Microseconds);

// Used for testing thread features
class TestThread : public Thread
{
	int a, b, c;
public:
	TestThread(int a, int b, int c) : a(a), b(b), c(c) {}

	virtual void Main() override
	{
		Yield(100);
		//std::cout << "Test";
	}
};
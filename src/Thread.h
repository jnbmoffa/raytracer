#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

typedef std::atomic<int> atomic_int;

typedef std::thread::id ThreadID;

// Mutual exclusion
typedef std::mutex MutexLock;
typedef std::recursive_mutex OwnerLock;

// Synchronization
typedef std::condition_variable_any CondLock;

typedef std::lock_guard<MutexLock> ScopeLock;
typedef std::unique_lock<OwnerLock> ScopeOwnerLock;

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

	template<typename ThreadType, typename... Args>
	friend ThreadType* MakeThread(Args... args);
};

/** Construct and start a new thread of the provided type */
template<typename ThreadType, typename... Args>
ThreadType* CreateThread(Args... args)
{
	ThreadType* NewThread = new ThreadType(args...);
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
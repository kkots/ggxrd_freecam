#pragma once
#include <mutex>

class RecursiveMutex
{
public:
	RecursiveMutex() = default;
	RecursiveMutex(const RecursiveMutex& other) = delete;
	RecursiveMutex& operator=(const RecursiveMutex& other) = delete;
	RecursiveMutex(RecursiveMutex&& other) = delete;
	RecursiveMutex& operator=(RecursiveMutex&& other) = delete;
	void lock();
	void unlock();
	std::mutex mutex;
private:
	DWORD threadId = 0;
	bool isLocked = false;  // don't rely on this value to actually tell if a mutex is locked (by another thread)
	int lockCount = 0;
};

class RecursiveMutexGuard {
public:
	RecursiveMutexGuard(RecursiveMutex& recursiveMutex);
	~RecursiveMutexGuard();
	RecursiveMutexGuard(const RecursiveMutexGuard& other) = delete;
	RecursiveMutexGuard& operator=(const RecursiveMutexGuard& other) = delete;
	RecursiveMutexGuard(RecursiveMutexGuard&& other) noexcept;
	RecursiveMutexGuard& operator=(RecursiveMutexGuard&& other) noexcept;
	void lock();
	void unlock();
private:
	void destructorCode();
	void moveFrom(RecursiveMutexGuard& other) noexcept;
	bool isLocked = false;
	RecursiveMutex* mutex = nullptr;
};

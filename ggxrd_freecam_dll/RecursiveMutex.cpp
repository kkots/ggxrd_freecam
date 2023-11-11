#include "pch.h"
#include "RecursiveMutex.h"

void RecursiveMutex::lock() {
	DWORD currentThreadId = GetCurrentThreadId();
	if (isLocked && threadId == currentThreadId) {
		// reasoning for no thread safety on these checks is they will fail anyway
		// if it's not the same thread as the one that captured the mutex earlier
		++lockCount;
		return;
	}
	mutex.lock();
	++lockCount;
	isLocked = true;
	threadId = currentThreadId;
}

void RecursiveMutex::unlock() {
	DWORD currentThreadId = GetCurrentThreadId();
	if (isLocked && threadId == currentThreadId) {
		--lockCount;
		if (lockCount == 0) {
			threadId = 0;
			isLocked = false;
			mutex.unlock();
		}
	} else {
		mutex.unlock();  // go ahead, make my day
	}
}

RecursiveMutexGuard::RecursiveMutexGuard(RecursiveMutex& recursiveMutex) : mutex(&recursiveMutex) {
	isLocked = true;
	recursiveMutex.lock();
}

RecursiveMutexGuard::~RecursiveMutexGuard() {
	destructorCode();
}

void RecursiveMutexGuard::destructorCode() {
	if (!mutex) return;
	if (isLocked) mutex->unlock();
}

RecursiveMutexGuard::RecursiveMutexGuard(RecursiveMutexGuard&& other) noexcept {
	moveFrom(other);
}

void RecursiveMutexGuard::moveFrom(RecursiveMutexGuard& other) noexcept {
	mutex = other.mutex;
	other.mutex = nullptr;
	isLocked = other.isLocked;
}

RecursiveMutexGuard& RecursiveMutexGuard::operator=(RecursiveMutexGuard&& other) noexcept {
	destructorCode();
	moveFrom(other);
	return *this;
}

void RecursiveMutexGuard::lock() {
	mutex->lock();
}

void RecursiveMutexGuard::unlock() {
	mutex->unlock();
}

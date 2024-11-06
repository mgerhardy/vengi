/**
 * @file
 */

#include "Concurrency.h"
#include "core/Common.h"
#include "core/Log.h"
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_platform.h>
#include <SDL3/SDL_cpuinfo.h>

#if defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS) || defined(SDL_PLATFORM_IOS)
#include <dlfcn.h>
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT nullptr
#endif
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(SDL_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h> // mbstowcs
#endif

namespace core {

bool setThreadName(const char *name) {
#if defined(SDL_PLATFORM_LINUX)
	int (*ppthread_setname_np)(pthread_t, const char*) = nullptr;
	void *fn = dlsym(RTLD_DEFAULT, "pthread_setname_np");
	ppthread_setname_np = (int(*)(pthread_t, const char*)) fn;
	if (ppthread_setname_np != nullptr) {
		const int err = ppthread_setname_np(pthread_self(), name);
		if (err == ERANGE) {
			Log::error("Thread name is too long - max 15 chars, got %s", name);
		} else if (err != 0) {
			Log::error("Can't set thread name: %i", err);
		}
		return err == 0;
	}
#elif defined(SDL_PLATFORM_MACOS)
	int (*ppthread_setname_np)(const char*) = nullptr;
	void *fn = dlsym(RTLD_DEFAULT, "pthread_setname_np");
	ppthread_setname_np = (int(*)(const char*)) fn;
	if (ppthread_setname_np != nullptr) {
		const int err = pthread_setname_np(name);
		if (err == ERANGE) {
			Log::error("Thread name is too long - max 15 chars, got %s", name);
		} else if (err != 0) {
			Log::error("Can't set thread name: %i", err);
		}
		return err == 0;
	}
#elif defined(SDL_PLATFORM_WINDOWS)
	typedef HRESULT (WINAPI *pfnSetThreadDescription)(HANDLE, PCWSTR);
	static pfnSetThreadDescription pSetThreadDescription = nullptr;
	static HMODULE kernel32 = nullptr;

	if (kernel32 == nullptr) {
		kernel32 = LoadLibraryW(L"kernel32.dll");
		if (kernel32 != nullptr) {
			pSetThreadDescription = (pfnSetThreadDescription)GetProcAddress(kernel32, "SetThreadDescription");
		}
	}

	if (pSetThreadDescription != nullptr) {
		wchar_t wname[512] = L"";
		mbstowcs(wname, name, 512);
		pSetThreadDescription(GetCurrentThread(), wname);
		return true;
	}
#endif
	return false;
}

void setThreadPriority(ThreadPriority prio) {
#if defined(SDL_PLATFORM_LINUX)
	int value;
	if (prio == ThreadPriority::Low) {
		value = 19;
	} else if (prio == ThreadPriority::High) {
		value = -20;
	} else {
		value = 0;
	}
	setpriority(PRIO_PROCESS, syscall(SYS_gettid), value);
#elif defined(SDL_PLATFORM_WINDOWS)
	int value;

	if (prio == ThreadPriority::Low) {
		value = THREAD_PRIORITY_LOWEST;
	} else if (prio == ThreadPriority::High) {
		value = THREAD_PRIORITY_TIME_CRITICAL;
	} else {
		value = THREAD_PRIORITY_NORMAL;
	}
	SetThreadPriority(GetCurrentThread(), value);
#elif not defined(__EMSCRIPTEN__)
	struct sched_param sched;
	int policy;
	pthread_t thread = pthread_self();

	if (pthread_getschedparam(thread, &policy, &sched) != 0) {
		Log::error("pthread_getschedparam() failed");
		return;
	}
	if (prio == ThreadPriority::Low) {
		sched.sched_priority = sched_get_priority_min(policy);
	} else if (prio == ThreadPriority::High) {
		sched.sched_priority = sched_get_priority_max(policy);
	} else {
		int min_priority = sched_get_priority_min(policy);
		int max_priority = sched_get_priority_max(policy);
		sched.sched_priority = (min_priority + (max_priority - min_priority) / 2);
	}
	if (pthread_setschedparam(thread, policy, &sched) != 0) {
		Log::error("pthread_setschedparam() failed");
	}
#endif
}

uint32_t cpus() {
	return core_max(1, SDL_GetNumLogicalCPUCores());
}

uint32_t halfcpus() {
	return core_max(1, SDL_GetNumLogicalCPUCores() / 2u);
}

}

/**
 * @file
 */

#include "Process.h"
#include "core/ArrayLength.h"
#include "core/Log.h"
#include "io/Stream.h"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_platform.h>

#include <fcntl.h>
#include <string.h>

#if defined(SDL_PLATFORM_WINDOWS)
#include <SDL3/SDL.h>
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <conio.h>
#include <direct.h>
#include <io.h>
#include <shellapi.h>
#include <shlobj.h>
#elif defined(SDL_PLATFORM_LINUX) or defined(SDL_PLATFORM_MACOS)
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace core {

int Process::exec(const core::String &command, const core::DynamicArray<core::String> &arguments,
				  const char *workingDirectory, io::WriteStream *stream) {
#if defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_MACOS)
	int stdoutfd[2];
	if (::pipe(stdoutfd) < 0) {
		Log::error("pipe failed: %s", strerror(errno));
		return -1;
	}

	int stderrfd[2] = {0, 0};
	if (pipe(stderrfd)) {
		close(stdoutfd[STDIN_FILENO]);
		close(stdoutfd[STDOUT_FILENO]);
		Log::error("pipe failed: %s", strerror(errno));
		return -1;
	}

	sigset_t blockMask, origMask;
	sigemptyset(&blockMask);
	sigaddset(&blockMask, SIGCHLD);
	// TODO: probably (usually) need to use pthread_sigmask() instead
	sigprocmask(SIG_BLOCK, &blockMask, &origMask);

	struct sigaction saIgnore, saOrigQuit, saOrigInt, saDefault;
	saIgnore.sa_handler = SIG_IGN;
	saIgnore.sa_flags = 0;
	sigemptyset(&saIgnore.sa_mask);
	sigaction(SIGINT, &saIgnore, &saOrigInt);
	sigaction(SIGQUIT, &saIgnore, &saOrigQuit);

	const pid_t childPid = ::fork();
	if (childPid < 0) {
		sigprocmask(SIG_SETMASK, &origMask, NULL);
		sigaction(SIGINT, &saOrigInt, NULL);
		sigaction(SIGQUIT, &saOrigQuit, NULL);
		Log::error("fork failed: %s", strerror(errno));
		return -1;
	}

	if (childPid == 0) {
		const char *argv[512];
		int argc = 0;
		argv[argc++] = command.c_str();
		for (const core::String &arg : arguments) {
			argv[argc++] = arg.c_str();
			if (argc >= lengthof(argv) - 1) {
				break;
			}
		}
		argv[argc] = nullptr;

		saDefault.sa_handler = SIG_DFL;
		saDefault.sa_flags = 0;
		sigemptyset(&saDefault.sa_mask);

		if (saOrigInt.sa_handler != SIG_IGN)
			sigaction(SIGINT, &saDefault, NULL);
		if (saOrigQuit.sa_handler != SIG_IGN)
			sigaction(SIGQUIT, &saDefault, NULL);

		sigprocmask(SIG_SETMASK, &origMask, NULL);

		// the child process isn't reading anything, so close
		// stdin and the read end of the pipes
		::close(STDIN_FILENO);
		::close(stdoutfd[STDIN_FILENO]);
		// dup stdout and stderr to the write end of the pipes,
		// so the parent process can read them
		::dup2(stdoutfd[STDOUT_FILENO], STDOUT_FILENO);
		::dup2(stderrfd[STDOUT_FILENO], STDERR_FILENO);
		if (workingDirectory != nullptr) {
			const int retVal = ::chdir(workingDirectory);
			if (retVal == -1) {
				Log::warn("Could not change current working dir to %s", workingDirectory);
			}
		}
		// we are the child
		::execv(command.c_str(), const_cast<char *const *>(argv));

		// this should never get called
		Log::error("failed to run '%s' with %i parameters: %s (%i)", command.c_str(), (int)arguments.size(),
				   strerror(errno), (int)errno);
		::exit(errno);
	}

	// the parent isn't writing anything to the child, so close the write
	// end of the pipes
	close(stdoutfd[STDOUT_FILENO]);
	close(stderrfd[STDOUT_FILENO]);
	if (stream != nullptr) {
		char output[1024];
		for (;;) {
			const int n = (int)::read(stdoutfd[STDIN_FILENO], output, sizeof(output));
			if (n <= 0) {
				break;
			}
			stream->write(output, n);
		}
		for (;;) {
			const int n = (int)::read(stderrfd[STDIN_FILENO], output, sizeof(output));
			if (n <= 0) {
				break;
			}
			stream->write(output, n);
		}
	}
	// we are the parent and are blocking until the child stopped
	int status;
	const pid_t pid = ::wait(&status);
#ifdef DEBUG
	core_assert(pid == childPid);
#else
	(void)pid;
#endif

	// check for success
	if (!WIFEXITED(status)) {
		Log::warn("child process exists with error");
		return -1;
	}

	Log::debug("child process returned with code %d", WEXITSTATUS(status));
	if (WEXITSTATUS(status) != 0) {
		return -1;
	}

	sigprocmask(SIG_SETMASK, &origMask, NULL);
	sigaction(SIGINT, &saOrigInt, NULL);
	sigaction(SIGQUIT, &saOrigQuit, NULL);

	// success
	return 0;
#elif SDL_PLATFORM_WINDOWS
	core::String cmd = command;
	if (!arguments.empty()) {
		cmd.append(" ");
	}
	for (const core::String &argument : arguments) {
		cmd.append(argument);
		cmd.append(" ");
	}

	STARTUPINFO startupInfo{};
	SECURITY_ATTRIBUTES secattr{};
	PROCESS_INFORMATION processInfo{};

	startupInfo.cb = sizeof(startupInfo);
	startupInfo.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
	startupInfo.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
	startupInfo.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;

	secattr.nLength = sizeof(secattr);
	secattr.bInheritHandle = TRUE;
	secattr.lpSecurityDescriptor = NULL;

	char *commandPtr = SDL_strdup(cmd.c_str());
	if (!CreateProcess(nullptr, (LPSTR)commandPtr, &secattr, &secattr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
					   &startupInfo, &processInfo)) {
		DWORD errnum = ::GetLastError();
		LPTSTR errmsg = nullptr;
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						nullptr, errnum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errmsg, 0, nullptr);
		if (errmsg == nullptr) {
			Log::error("%d - Unknown error", (int)errnum);
		} else {
			Log::error("%d - %s", (int)errnum, errmsg);
			::LocalFree(errmsg);
		}
		SDL_free(commandPtr);
		return -1;
	}
	SDL_free(commandPtr);

	WaitForSingleObject(processInfo.hProcess, INFINITE);

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	return 0;
#else
	Log::error("Process::exec is not implemented for this platform");
	return 1;
#endif
}
} // namespace core

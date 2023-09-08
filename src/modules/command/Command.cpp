/**
 * @file
 */

#include "Command.h"
#include "core/StringUtil.h"
#include "core/ArrayLength.h"
#include "core/Tokenizer.h"
#include "core/Log.h"

namespace command {

Command::CommandMap Command::_cmds;
core::ReadWriteLock Command::_lock("Command");
core::DynamicArray<core::String> Command::_delayedTokens;
double Command::_delaySeconds = 0.0;
size_t  Command::_sortedCommandListSize = 0u;
Command* Command::_sortedCommandList[4096] {};

ActionButtonCommands& ActionButtonCommands::setHelp(const char* help) {
	Command::getCommand(first)->setHelp(help);
	Command::getCommand(second)->setHelp(help);
	return *this;
}

Command& Command::registerCommand(const char* name, FunctionType&& func) {
	const core::String cname(name);
	const Command c(cname, std::forward<FunctionType>(func));
	core::ScopedWriteLock lock(_lock);
	_cmds.put(cname, c);
	updateSortedList();
	return (Command&)_cmds.find(cname)->value;
}

bool Command::unregisterCommand(const char* name) {
	const core::String cname(name);
	core::ScopedWriteLock lock(_lock);
	const bool removed = _cmds.remove(cname);
	if (removed) {
		updateSortedList();
	}
	return removed;
}

ActionButtonCommands Command::registerActionButton(const core::String& name, ActionButton& button, const char *help) {
	core::ScopedWriteLock lock(_lock);
	Command cPressed(COMMAND_PRESSED + name, [&] (const command::CmdArgs& args) {
		const int32_t key = args.size() >= 1 ? args[0].toInt() : 0;
		const double seconds = args.size() >= 2 ? core::string::toDouble(args[1]) : 0.0;
		button.handleDown(key, seconds);
	});
	cPressed.setHelp(help);
	_cmds.put(cPressed.name(), cPressed);
	Command cReleased(COMMAND_RELEASED + name, [&] (const command::CmdArgs& args) {
		const int32_t key = args.size() >= 1 ? args[0].toInt() : 0;
		const double seconds = args.size() >= 2 ? core::string::toDouble(args[1]) : 0.0;
		button.handleUp(key, seconds);
	});
	cReleased.setHelp(help);
	_cmds.put(cReleased.name(), cReleased);
	updateSortedList();
	return ActionButtonCommands(COMMAND_PRESSED + name, COMMAND_RELEASED + name);
}

bool Command::unregisterActionButton(const core::String& name) {
	core::ScopedWriteLock lock(_lock);
	const core::String downB(COMMAND_PRESSED + name);
	const core::String upB(COMMAND_RELEASED + name);
	int amount = _cmds.remove(downB);
	amount += _cmds.remove(upB);
	updateSortedList();
	return amount == 2;
}

int Command::complete(const core::String& str, core::DynamicArray<core::String>& matches) const {
	if (!_completer) {
		return 0;
	}
	return _completer(str, matches);
}

int Command::update(double deltaFrameSeconds) {
	if (_delaySeconds <= 0.0) {
		return 0;
	}
	Log::trace("Waiting %f seconds", _delaySeconds);
	if (deltaFrameSeconds > _delaySeconds) {
		_delaySeconds = 0.0;
	} else {
		_delaySeconds -= deltaFrameSeconds;
	}
	if (_delaySeconds > 0.0) {
		return 0;
	}
	// make a copy - it might get modified inside the execute call
	core::DynamicArray<core::String> copy = _delayedTokens;
	_delayedTokens.clear();
	int executed = 0;
	for (const core::String& fullCmd : copy) {
		Log::debug("execute %s", fullCmd.c_str());
		executed += execute(fullCmd);
	}

	return executed;
}

void Command::updateSortedList() {
	core_assert((int)_cmds.size() < lengthof(_sortedCommandList));
	_sortedCommandListSize = 0u;
	for (auto i = _cmds.begin(); i != _cmds.end(); ++i) {
		_sortedCommandList[_sortedCommandListSize++] = &i->value;
	}
	SDL_qsort(_sortedCommandList, _sortedCommandListSize, sizeof(Command*), [] (const void *v1, const void *v2) {
		return SDL_strcmp((*(const Command*const *)v1)->name(), (*(const Command*const *)v2)->name());
	});
}

int Command::execute(const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	char buf[4096];
	SDL_vsnprintf(buf, sizeof(buf), msg, args);
	buf[sizeof(buf) - 1] = '\0';
	const int cmds = execute(core::String(buf));
	va_end(args);
	return cmds;
}

int Command::execute(const core::String& command) {
	int executed = 0;
	core::TokenizerConfig cfg;
	cfg.skipComments = false;
	core::Tokenizer commandLineTokenizer(cfg, command, ";\n");
	while (commandLineTokenizer.hasNext()) {
		const core::String& fullCmd = commandLineTokenizer.next();
		if (fullCmd.empty()) {
			continue;
		}
		if (fullCmd[0] == '#') {
			continue;
		}
		if (fullCmd.size() >= 2 && fullCmd[0] == '/' && fullCmd[1] == '/') {
			continue;
		}
		if (_delaySeconds > 0.0) {
			Log::debug("add command %s to delayed buffer", fullCmd.c_str());
			_delayedTokens.push_back(fullCmd);
			continue;
		}
		Log::debug("full command: '%s'", fullCmd.c_str());
		core::Tokenizer commandTokenizer(cfg, fullCmd, " ");
		if (!commandTokenizer.hasNext()) {
			continue;
		}
		const core::String& c = commandTokenizer.next();
		Log::debug("command: '%s'", c.c_str());
		core::DynamicArray<core::String> args;
		while (commandTokenizer.hasNext()) {
			args.push_back(commandTokenizer.next());
			Log::debug("arg: '%s'", args.back().c_str());
		}
		if (execute(c, args)) {
			++executed;
		}
	}
	return executed;
}

bool Command::execute(const core::String& command, const CmdArgs& args) {
	if (command == "wait") {
		if (args.size() == 1) {
			_delaySeconds += core_max(1, args[0].toInt());
		} else {
			_delaySeconds += 1.0;
		}
		return true;
	}
	if ((command[0] == COMMAND_PRESSED[0] || command[0] == COMMAND_RELEASED[0]) && args.empty()) {
		Log::warn("Skip execution of %s - no arguments provided", command.c_str());
		return false;
	}
	Command cmd;
	{
		core::ScopedReadLock scoped(_lock);
		auto i = _cmds.find(command);
		if (i == _cmds.end()) {
			Log::debug("could not find command callback for %s", command.c_str());
			return false;
		}
		if (_delaySeconds > 0.0) {
			core::String fullCmd = command;
			for (const core::String& arg : args) {
				fullCmd.append(" ");
				fullCmd.append(arg);
			}
			Log::debug("delay %s", fullCmd.c_str());
			_delayedTokens.push_back(fullCmd);
			return true;
		}
		cmd = i->second;
	}
	Log::debug("execute %s with %i arguments", command.c_str(), (int)args.size());
	cmd._func(args);
	return true;
}

void Command::shutdown() {
	core::ScopedWriteLock lock(_lock);
	_cmds.clear();
}

Command& Command::setHelp(const char* help) {
	if (help == nullptr) {
		_help = "";
	} else {
		_help = help;
	}
	return *this;
}

Command& Command::setBoolCompleter() {
	return setArgumentCompleter([] (const core::String& str, core::DynamicArray<core::String>& matches) -> int {
		if (str[0] == 't') {
			matches.emplace_back("true");
			return 1;
		}
		if (str[0] == 'f') {
			matches.emplace_back("false");
			return 1;
		}
		matches.emplace_back("true");
		matches.emplace_back("false");
		return 2;
	});
}

}

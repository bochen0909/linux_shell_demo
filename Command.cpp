/*
 * Command.cpp
 *
 *  Created on: Sep 28, 2015
 *      Author: Lizhen
 */

#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>

#include "Util.h"
#include "MyShell.h"
#include "Command.h"
#include "SimpleCommand.h"
#include "CompositeCommand.h"

bool Command::is_special(std::string& s) {
	return s == "|" || s == "<" || s == ">" || s == "&";
}
Command* Command::parse(const std::string& line, MyShell* shell = NULL) {
	std::vector<std::string> vec = Util::splitcmdline(line);
	//Util::printvec(vec);
	if (vec.size() == 0) {
		return NULL;
	}
	bool isCompisite = false;
	bool badCommand = false;
	std::string errmsg;
	for (size_t i = 0; i < vec.size(); i++) {
		if (vec[i] == "|" || vec[i] == "<" || vec[i] == ">") {
			if (i == 0 || i + 1 == vec.size() || is_special(vec.at(i + 1))) {
				badCommand = true;
				errmsg = "|<> cannot be at beginning and must follow something";
				break;
			}
		}
		if (vec[i] == "&") {
			if (i + 1 < vec.size()) {
				badCommand = true;
				errmsg = "& must be at end";
				break;
			}
		}
		if (vec[i] == "|") {
			isCompisite = true;
		}
	}
	if (badCommand) {
		throw std::runtime_error(errmsg.c_str());
	}
	Command* command = NULL;
	if (isCompisite) {
		command = new CompositeCommand(vec, shell);
	} else {
		command = new SimpleCommand(vec, shell);
	}
	if (command->isBad()) {
		errmsg = command->error();
		delete command;
		throw std::runtime_error(errmsg);
	}
	return command;
}

Command::Command(MyShell* shell = NULL) :
		isBG(false), bBad(false), context(shell) {
}

Command::~Command() {
}

std::string Command::error() {
	return errmsg;
}
bool Command::isBackGround() {
	return isBG;
}

bool Command::isBad() {
	return bBad;
}

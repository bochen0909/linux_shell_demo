/*
 * MyShell.cpp
 *
 *  Created on: Sep 27, 2015
 *      Author: LiZhen
 */

#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "Util.h"
#include "MyShell.h"
#include "Command.h"
#include <stdexcept>
#include <setjmp.h>
#include <signal.h>

static char* command_names[] = { (char*) "help", (char*) "cd", (char*) "pwd",
		(char*) "exit", (char*) "set", (char*) 0 };
static char* command_docs[] = {
		(char*) "Display information about builtin commands.",
		(char*) "Change the shell working directory.",
		(char*) "Print the name of the current working directory.",
		(char*) "Exit the shell.", (char*) "Set or display MyShell variables",
		(char*) 0 };

int MyShell::com_help(std::vector<std::string> args) {
	if (args.size() > 1) {
		fprintf(stderr, "usage: help [command]");
		return -1;
	}

	register int i;
	for (i = 0; command_names[i]; i++) {
		if (args.empty() || (strcmp(args[0].c_str(), command_names[i]) == 0)) {
			printf("%s\t\t%s\n", command_names[i], command_docs[i]);
		}
	}
	return (0);
}

int MyShell::com_cd(std::vector<std::string> arg) {
	if (arg.size() != 1) {
		fprintf(stderr, "usage: cd <dir>\n");
		return 1;
	}
	if (chdir(arg[0].c_str()) == -1) {
		perror(arg[0].c_str());
		return 1;
	}
	return (0);
}

int MyShell::com_set(std::vector<std::string> arg) {
	if (arg.size() == 0) {
		std::map<std::string, std::string>::iterator itor;
		for (itor = variables.begin(); itor != variables.end(); itor++) {
			fprintf(stdout, "%s=%s\n", itor->first.c_str(),
					itor->second.c_str());
		}
	} else if (arg.size() == 1) {
		std::map<std::string, std::string>::iterator itor;
		std::vector<std::string> vec = Util::splitByDeli(arg[0], '=');
		std::string varname = vec[0];
		if (vec.size() == 1) {
			for (itor = variables.begin(); itor != variables.end(); itor++) {
				if (Util::startsWith(itor->first, varname))
					fprintf(stdout, "%s=%s\n", itor->first.c_str(),
							itor->second.c_str());
			}
		} else if (vec.size() == 2) {
			variables.erase(varname);
		} else {
			variables[varname] = vec[2];
		}
	} else {
		fprintf(stderr, "usage: set or set <varname>=<value>");
		return -1;
	}
	return (0);
}

int MyShell::com_pwd(std::vector<std::string> ignore) {
	char dir[1024], *s;

	s = getcwd(dir, sizeof(dir) - 1);
	if (s == 0) {
		printf("Error getting pwd: %s\n", dir);
		return 1;
	}

	printf("%s\n", dir);
	return 0;
}

/* The user wishes to quit using this program.  Just set DONE non-zero. */
int MyShell::com_quit(std::vector<std::string> arg) {
	done = 1;
	return 0;
}

std::string MyShell::getvar(const std::string& varname) {
	if (variables.find(varname) != variables.end()) {
		return variables[varname];
	}
	return Util::getvar(varname);
}

char * command_generator(const char *text, int state) {
	static int list_index, len;
	char *name;

	/* If this is a new word to complete, initialize now.  This includes
	 saving the length of TEXT for efficiency, and initializing the index
	 variable to 0. */
	if (!state) {
		list_index = 0;
		len = strlen(text);
	}

	/* Return the next name which partially matches from the command list. */
	while ((name = command_names[list_index++])) {
		if (strncmp(name, text, len) == 0)
			return (Util::dupstr(name));
	}

	/* If no names matched, then return NULL. */
	return ((char *) NULL);
}

char ** myshell_completion(const char* text, int start, int end) {
	char **matches;

	matches = (char **) NULL;

	/* If this word is at the start of the line, then it is a command
	 to complete.  Otherwise it is the name of a file in the current
	 directory. */
	if (start == 0)
		matches = rl_completion_matches(text, command_generator);

	return (matches);
}

MyShell* MyShell::m_instance = (MyShell*) NULL;
MyShell::MyShell(const char* progname) :
		done(0), line(NULL) {
	this->m_progname = progname;
	this->variables["SHELL"] = "MyShell";
}

MyShell::~MyShell() {
	if (line != NULL)
		free(line);
}

sigjmp_buf ctrlc_buf;

void sig_handler(int signo) {

	if (signo == SIGINT) {
		fprintf(stdout, "\n");
		siglongjmp(ctrlc_buf, 1);
	}
}

int MyShell::run() {
	rl_readline_name = "MyShell";
	rl_attempted_completion_function = myshell_completion;

	 rl_catch_signals = 1;
	 while ( sigsetjmp( ctrlc_buf, 1 ) != 0);
	 signal(SIGINT, sig_handler);


	/* Loop reading and executing lines until the user quits. */
	for (; done == 0;) {
		char* line = readline("$ ");

		if (!line)
			break;

		string s = Util::trim(line);
		if (!s.empty()) {
			add_history(s.c_str());
			execute_line(s);
		}
	}
	return 0;
}

int MyShell::execute_line(const string& l) {
	Command * commandObj = NULL;
	try {
		commandObj = Command::parse(l, this);
	} catch (std::runtime_error& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}
	if (!commandObj) {
		fprintf(stderr, "Parse command line failed.\n");
		return -1;
	} else {
		int exit_code = 0;
		exit_code = commandObj->run();
		if (exit_code != 0) {
			fprintf(stderr, "%s\n", commandObj->error().c_str());
		}
		delete commandObj;
		return exit_code;
	}
}

MyShell& MyShell::instance() {
	if (!MyShell::m_instance) {
		MyShell::m_instance = new MyShell("MyShell");
	}
	return *m_instance;
}

bool MyShell::isBuiltin(const std::string& name) {
	return name == "cd" || name == "pwd" || name == "help" || name == "?"
			|| name == "exit" || name == "set";
}

int MyShell::runBuiltin(const std::vector<std::string>& vec) {
	string command = vec[0];

	std::vector<std::string> args;
	for (size_t j = 1; j < vec.size(); j++) {
		args.push_back(vec[j]);
	}

	/* Call the function. */
	int exit_code = 0;
	if (command == "cd") {
		exit_code = this->com_cd(args);
	} else if (command == "pwd") {
		exit_code = this->com_pwd(args);
	} else if (command == "help" || command == "?") {
		exit_code = this->com_help(args);
	} else if (command == "exit") {
		exit_code = this->com_quit(args);
	} else if (command == "set") {
		exit_code = this->com_set(args);
	} else {
		fprintf(stderr, "Command is not builtin: %s\n", command.c_str());
		exit_code = -1;
	}
	return exit_code;
}

std::string MyShell::findExternalCommandPath(const std::string& comname) {
	std::string value = this->getvar("MYPATH");
	std::vector<std::string> vecpath;
	if (!value.empty()) {
		vecpath = Util::splitByDeli(value, ':');
	}
	value = this->getvar("PATH");
	if (!value.empty()) {
		std::vector<std::string> vecpath2 = Util::splitByDeli(value, ':');
		vecpath.insert(vecpath.end(), vecpath2.begin(), vecpath2.end());
	}
	for (size_t i = 0; i < vecpath.size(); i++) {
		if (Util::isExcutableIn(comname, vecpath[i])) {
			return vecpath[i] + "/" + comname;
		}
	}
	return "";
}


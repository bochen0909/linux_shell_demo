/*
 * CompositeCommand.cpp
 *
 *  Created on: Sep 28, 2015
 *      Author: Lizhen
 */

#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "Util.h"
#include "MyShell.h"
#include "SimpleCommand.h"
#include "CompositeCommand.h"

CompositeCommand::CompositeCommand(std::vector<std::string> &vec,
		MyShell* shell) :
		Command(shell), pipes(0) {
	size_t n = vec.size();
	if (vec[n - 1] == "&") {
		isBG = true;
		n = n - 1;
	}
	this->args = std::vector<std::string>(vec.begin(), vec.begin() + n);

	std::vector<std::string> subargs;
	for (size_t i = 0; i < n; i++) {
		if (args[i] == "|") {
			SimpleCommand* command = new SimpleCommand(subargs, context);
			bBad |= command->isBad();
			errmsg += command->error();
			this->commands.push_back(command);
			subargs.clear();
		} else {
			subargs.push_back(args[i]);
		}
	}
	if (!subargs.empty()) {
		SimpleCommand* command = new SimpleCommand(subargs, context);
		bBad |= command->isBad();
		errmsg += command->error();
		this->commands.push_back(command);
	}
}

CompositeCommand::~CompositeCommand() {
	for (size_t i = 0; i < commands.size(); i++) {
		delete commands.at(i);
	}
}

int CompositeCommand::preparePipe() {
	int n = this->commands.size();
	this->pipes = new int*[n];
	for (int i = 0; i < n; i++) {
		pipes[i] = new int[2];
		pipe(pipes[i]);
	}
	return 0;
}

int CompositeCommand::cleanPipe() {
	int n = this->commands.size();
	for (int i = 0; i < n; i++) {
		close(pipes[i][0]);
		close(pipes[i][1]);
		delete[] pipes[i];
	}
	delete[] pipes;
	return 0;
}

int g_waited_pid = -1;
void child_exit2(int signo) {
	int wstat;
	pid_t pid;

	while (true) {
		pid = wait3(&wstat, WNOHANG, (struct rusage *) NULL);
		if (pid == 0)
			return;
		else if (pid == -1)
			return;
		else if (pid == g_waited_pid) {
			printf("[Done] %d\n", pid);
			g_waited_pid = -1;
		}
	}
}

int CompositeCommand::run() {
	//dump();
	if (this->validCommand() != 0) {
		return -1;
	}

	if (isBackGround()) {
		signal(SIGCHLD, child_exit2);
	}

	this->preparePipe();
	for (size_t i = 0; i < commands.size(); i++) {
		runSingleCommand(i);
	}
	this->cleanPipe();

	if (isBackGround()) {
		fprintf(stdout, "[%s] %d\n",
				commands[commands.size() - 1]->name.c_str(),
				commands[commands.size() - 1]->pid);
		int status;
		waitpid(-1, &status, WNOHANG);
	} else {
		int status;
		for (size_t i = 0; i < commands.size(); i++) {
			waitpid(commands[i]->pid, &status, 0);
		}
	}

	return 0;
}

int CompositeCommand::runSingleCommand(int idx) {
	SimpleCommand& command = *commands.at(idx);
	pid_t childpid;
	if ((childpid = fork()) == -1) {
		perror("fork child failed.");
		exit(1);
	}
	if (childpid == 0) {
		//close pipes
		if (1) {
			int n = this->commands.size();
			for (int i = 0; i < n; i++) {
				if (idx - 1 != i)
					close(pipes[i][0]);
				if (idx != i || idx + 1 == n)
					close(pipes[i][1]);
			}
		}
		//handle stdin
		/* Close up standard input of the child */
		if (idx > 0) {
			close(STDIN_FILENO);
			/* Duplicate the input side of pipe to stdin */
			dup(pipes[idx - 1][0]);
		} else {
			std::string &stdinFileName = command.stdinFileName;
			if (stdinFileName.empty() && isBackGround()) {
				stdinFileName = "/dev/null";
			}
			if (!stdinFileName.empty()) {
				close(STDIN_FILENO);
				if (open(stdinFileName.c_str(), O_RDONLY) == -1) {
					fprintf(stderr, "Fail to open %s\n", stdinFileName.c_str());
					exit(1);
				}
			}
		}

		//handle stdout
		if (idx + 1 != (int) commands.size()) {
			close(STDOUT_FILENO);
			dup(pipes[idx][1]);
		} else {
			std::string &stdoutFileName = command.stdoutFileName;
			if (!stdoutFileName.empty()) {
				close(STDOUT_FILENO);
				if (open(stdoutFileName.c_str(), O_RDWR | O_CREAT | O_TRUNC,
						0666) == -1) {
					fprintf(stderr, "Fail to open %s\n",
							stdoutFileName.c_str());
					exit(1);
				}
			}
		}

		//exec
		signal(SIGINT, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
		char** args = Util::vec2argv(command.args);
		int code = execv(command.name.c_str(), args);
		Util::free_argv(args);
		if (code < 0) { /* execute the command  */
			fprintf(stderr, "*** ERROR: exec failed for %s\n",
					command.name.c_str());
			exit(1);
		}
		exit(0);
	} else { //parent
		command.pid = childpid;
		if ((size_t) (idx + 1) == commands.size()) {
			g_waited_pid = childpid;
		}
		return 0;
	}
	return (0);
}

void CompositeCommand::dump() {
	for (size_t i = 0; i < commands.size(); i++) {
		SimpleCommand& command = *commands.at(i);
		printf("%ld: %s", i, command.name.c_str());
		for (size_t j = 0; j < command.args.size(); j++) {
			printf(" %s", command.args[j].c_str());
		}
		printf(", %s %s", command.stdinFileName.c_str(),
				command.stdoutFileName.c_str());
		printf("\n");
	}
}
int CompositeCommand::validCommand() {
	for (size_t i = 0; i < commands.size(); i++) {
		SimpleCommand& command = *commands.at(i);
		if (context->isBuiltin(command.name)) {
			fprintf(stderr, "%s: Buildin command cannot run in pipe.",
					command.name.c_str());
			return -1;
		}
		if (i != 0 && !command.stdinFileName.empty()) {
			fprintf(stderr, "%s: Redirect stdin here is not allowed.",
					command.name.c_str());
			return -1;
		}
		if (i != commands.size() - 1 && !command.stdoutFileName.empty()) {
			fprintf(stderr, "%s: Redirect stdout here is not allowed.",
					command.name.c_str());
			return -1;
		}

		std::string name = command.name;
		std::string compath;
		if (name.find("/") == std::string::npos) {
			compath = context->findExternalCommandPath(name);
		} else {
			if (Util::isExecutable(name)) {
				compath = name;
			}
		}
		if (compath.empty()) {
			fprintf(stderr, "%s command not found.", name.c_str());
			return -1;
		}
		command.name = compath;
		command.args[0] = compath;
	}

	return 0;
}

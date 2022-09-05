/*
 * SimpleCommand.cpp
 *
 *  Created on: Sep 28, 2015
 *      Author: Lizhen
 */

#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "Util.h"
#include "MyShell.h"
#include "SimpleCommand.h"

SimpleCommand::SimpleCommand(std::vector<std::string>& vec, MyShell* shell) :
		Command(shell),pid(0) {
	size_t n = vec.size();
	if (vec[n - 1] == "&") {
		isBG = true;
		n = n - 1;
	}

	name = vec[0];
	this->args = std::vector<std::string>(vec.begin(), vec.begin() + n);

	for (size_t i = 0; i < n; i++) {
		if (args[i] == ">") {
			if (i != n - 2) {
				bBad = true;
				errmsg = "Pipe is not right!";
			} else {
				stdoutFileName = args[n - 1];
			}
		} else if (args[i] == "<") {
			if (i == n - 2) {
				stdinFileName = args[n - 1];
			} else if (i == n - 4 && args[i + 2] == ">") {
				stdinFileName = args[n - 3];
			} else {
				bBad = true;
				errmsg = "Pipe is not right!";
			}
		}
	}

	n = args.size();
	if (!stdinFileName.empty() && !stdoutFileName.empty()) {
		this->args = std::vector<std::string>(args.begin(),
				args.begin() + n - 4);
	} else if (!stdinFileName.empty() || !stdoutFileName.empty()) {
		this->args = std::vector<std::string>(args.begin(),
				args.begin() + n - 2);
	}
}

SimpleCommand::~SimpleCommand() {
}

int SimpleCommand::run() {
	if ((context == NULL)) {
		return -1;
	}
	if (context->isBuiltin(name)) {
		if (this->isBackGround()) {
			errmsg = "Builtin function cannot run in background";
			return -1;
		} else {
			return context->runBuiltin(this->args);
		}
	} else {
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
		} else {
			if (isBackGround()) {
				return runBackground(compath);
			} else {
				return runForeground(compath);
			}

		}
		return -1;
	}
}

int SimpleCommand::runForeground(const string& compath) {
	pid_t pid;
	int status;
	if ((pid = fork()) < 0) { /* fork a child process           */
		fprintf(stderr, "*** ERROR: forking child process failed\n");
		exit(1);
	} else if (pid == 0) { /* for the child process:         */
		signal(SIGINT, SIG_DFL);
		char** args = Util::vec2argv(this->args);
		//printf("AAa %s\n", stdoutFileName.c_str());
		if (!stdoutFileName.empty()) {
			close(STDOUT_FILENO);
			if (open(stdoutFileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666)
					== -1) {
				fprintf(stderr, "Fail to open %s\n", stdoutFileName.c_str());
				exit(1);
			}
		}
		if (!stdinFileName.empty()) {
			close(STDIN_FILENO);
			if (open(stdinFileName.c_str(), O_RDONLY) == -1) {
				fprintf(stderr, "Fail to open %s\n", stdinFileName.c_str());
				exit(1);
			}
		}

		int code = execv(compath.c_str(), args);
		Util::free_argv(args);
		if (code < 0) { /* execute the command  */
			fprintf(stderr, "*** ERROR: exec failed for %s\n", compath.c_str());			exit(1);
		}
	} else { /* for the parent:      */
		while (wait(&status) != pid)
			;
		/* wait for completion  */
		return 0;
	}
	return -1;
}

void child_exit(int signo) {
	int wstat;
	pid_t pid;

	while (true) {
		pid = wait3(&wstat, WNOHANG, (struct rusage *) NULL);
		if (pid == 0)
			return;
		else if (pid == -1)
			return;
		else
			printf("[Done] %d\n", pid);
	}
}

int SimpleCommand::runBackground(const string& compath) {
	pid_t pid;
	int status;
	signal(SIGCHLD, child_exit);
	if ((pid = fork()) < 0) { /* fork a child process           */
		fprintf(stderr, "*** ERROR: forking child process failed\n");
		exit(1);
	} else if (pid == 0) { /* for the child process:         */
		signal(SIGINT, SIG_DFL);
		signal(SIGCHLD, SIG_DFL);
		char** args = Util::vec2argv(this->args);
		//printf("AAa %s\n", stdoutFileName.c_str());
		if (!stdoutFileName.empty()) {
			close(STDOUT_FILENO);
			if (open(stdoutFileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666)
					== -1) {
				fprintf(stderr, "Fail to open %s\n", stdoutFileName.c_str());
				exit(1);
			}
		}
		if (stdinFileName.empty()) {
			stdinFileName = "/dev/null";
		}
		if (!stdinFileName.empty()) {
			close(STDIN_FILENO);
			if (open(stdinFileName.c_str(), O_RDONLY) == -1) {
				fprintf(stderr, "Fail to open %s\n", stdinFileName.c_str());
				exit(1);
			}
		}

		int code = execv(compath.c_str(), args);
		Util::free_argv(args);
		if (code < 0) { /* execute the command  */
			fprintf(stderr, "*** ERROR: exec failed for %s\n", compath.c_str());
			exit(1);
		}
	} else { /* for the parent:      */
		fprintf(stdout, "[%s] %d\n",compath.c_str(), pid);
		waitpid(-1, &status, WNOHANG);
		/* return immediately  */
		return 0;
	}
	return -1;
}

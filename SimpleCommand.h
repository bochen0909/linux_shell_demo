/*
 * SimpleCommand.h
 *
 *  Created on: Sep 28, 2015
 *      Author: Lizhen
 */

#ifndef SIMPLECOMMAND_H_
#define SIMPLECOMMAND_H_

#include<string>
#include<vector>
#include "Command.h"

/**
 * A single command is not involved in pipleline.
 */
class SimpleCommand: public Command {
public:
	friend class CompositeCommand;
	/**
	 * constructor
	 */
	SimpleCommand(std::vector<std::string>& args, MyShell* shell);
	/**
	 * destructor
	 */
	virtual ~SimpleCommand();
	/**
	 * run the command
	 */
	int run();
protected:
	/**
	 * run the command in foreground
	 */
	int runForeground(const string& compath);

	/**
	 * run the command in foreground
	 */
	int runBackground(const string& compath);

protected:
	std::string name;
	std::string stdinFileName;
	std::string stdoutFileName;
	int pid;
};

#endif /* SIMPLECOMMAND_H_ */

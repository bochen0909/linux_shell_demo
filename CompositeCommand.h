/*
 * CompositeCommand.h
 *
 *  Created on: Sep 28, 2015
 *      Author: Lizhen
 */

#ifndef COMPOSITECOMMAND_H_
#define COMPOSITECOMMAND_H_

#include<string>
#include<vector>
#include "SimpleCommand.h"

/**
 * CompositeCommand is pipeline command
 */
class CompositeCommand:public Command {
public:
	/**
	 * constructor
	 */
	CompositeCommand(std::vector<std::string>& args,MyShell* shell);
	/**
	 * destructor
	 */
	virtual ~CompositeCommand();
	/**
	 * run the command
	 */
	int run();

	/**
	 * for debug. dump details
	 */
	void dump();
protected:
	std::vector<SimpleCommand*> commands;
	int** pipes;
protected:
	/**
	 * run the idx_th command in the pipe line.
	 */
	int runSingleCommand(int idx);
	/**
	 * 0 if the command is valid pipe line command.
	 */
	int validCommand();
	/**
	 * create pipes before run sub commands.
	 */
	int preparePipe() ;
	/**
	 * close pipes that not needed for the current sub command.
	 */
	int cleanPipe() ;
};



#endif /* COMPOSITECOMMAND_H_ */

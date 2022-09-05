/*
 * Command.h
 *
 *  Created on: Sep 28, 2015
 *      Author: Lizhen
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include<string>
#include<vector>

class MyShell;
//forward declaration.

/**
 * Abstract class for single command and pipline command
 */
class Command {
public:
	/**
	 * Parse a command line and returns command pointer.
	 * arg shell: the context for the command
	 */
	static Command* parse(const std::string& line, MyShell* shell);

	/**
	 * Check if a string a special characters like | > < &
	 */
	static bool is_special(std::string& s);

	/**
	 * constructor
	 */
	Command(MyShell* shell);

	/**
	 * destructor
	 */
	virtual ~Command();

	/**
	 * Abstract method. Run the command
	 */
	virtual int run()=0;

	/**
	 * return error description for the last error.
	 */
	virtual std::string error();

	/**
	 * true if the command line is not well formed.
	 */
	virtual bool isBad();

	/**
	 * true if it is a background process.
	 */
	virtual bool isBackGround();
protected:
	std::string errmsg;
	bool isBG;
	bool bBad;
	std::vector<std::string> args;
	MyShell* context;
};

#endif /* COMMAND_H_ */

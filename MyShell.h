/*
 * MyShell.h
 *
 *  Created on: Sep 27, 2015
 *      Author: Lizhen
 */

#ifndef MYSHELL_H_
#define MYSHELL_H_


#include <readline/readline.h>
#include <readline/history.h>
#include <vector>
#include <map>
#include <string>


using namespace std;

/**
 * MyShell class
 */
class MyShell {
public:
	/**
	 * return a singleton instance for the class
	 */
	static MyShell& instance();

	/**
	 * true if a command is built in command
	 */
	static bool isBuiltin(const std::string& name);

	/**
	 * run a build in command.
	 * The first argument is the command name.
	 */
	int runBuiltin(const std::vector<std::string>& args);

public:
	/**
	 * destructor
	 */
	virtual ~MyShell();

	/**
	 * start the shell
	 */
	int run();

	/** find the full path for a non-builtin command.
	 *  It search MYPATH variable first, then search the PATH variable.
	 */
	std::string findExternalCommandPath(const std::string& comname);

	friend char * command_generator(const char *text, int state);
	friend char ** myshell_completion (const char* text, int start, int end);

protected:
	/**
	 * Parse one line of user input. If no error, run it.
	 */
	int execute_line (const string& line);

	/**
	 * Buildin command
	 */
	int com_help (std::vector<std::string> arg);

	/**
	 * Buildin command
	 */
	int com_cd (std::vector<std::string>arg);

	/**
	 * Buildin command
	 */
	int com_pwd (std::vector<std::string> ignore);

	/**
	 * Buildin command
	 */
	int com_quit (std::vector<std::string> arg);

	/**
	 * Buildin command
	 */
	int com_set (std::vector<std::string> arg);


	/**
	 * return the value of a variable.
	 * It search local variables first, then search system variables.
	 */
	std::string getvar(const std::string& varname);


protected:
	/**
	 * constructor
	 */
	MyShell(const char* progname);
protected:
	string m_progname;
	int done;
	char* line;
	std::map<std::string,std::string> variables;
private:
	static MyShell* m_instance; //singleton

};


#endif /* MYSHELL_H_ */

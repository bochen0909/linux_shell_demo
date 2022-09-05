/*
 * Util.h
 *
 *  Created on: Sep 27, 2015
 *      Author: Lizhen
 */

#ifndef MYUTIL_H_
#define MYUTIL_H_

#include <string>
#include <vector>

/**
 * Utility methods class.
 * All methods are static.
 * Actually put them in a name space is better.
 */
struct Util {
	static std::string &ltrim(std::string &s);

	static std::string &rtrim(std::string &s);

	// trim from both ends
	static std::string &trim(std::string &s);
	static inline std::string trim(char* s) {
		std::string ss = s;
		return trim(ss);
	}

	static char * dupstr(char* s);
	static std::vector<std::string> split(const std::string &input);
	static std::vector<std::string> splitByDeli(const std::string &input,
			char deli);
	static std::vector<std::string> splitcmdline(const std::string &input);
	static void printvec(const std::vector<std::string>& vec);
	static bool startsWith(const std::string& s, const std::string& sub);

	static std::string getvar(const std::string& varname);
	static bool isExcutableIn(const ::std::string& executable,
			const std::string& dirname);
	static char** vec2argv(const std::vector<std::string>& vec);
	static void free_argv(char** argv);
	static bool  isExecutable(const std::string& file);

};

#endif /* UTIL_H_ */

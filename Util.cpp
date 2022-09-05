/*
 * Util.cpp
 *
 *  Created on: Sep 27, 2015
 *      Author: Lizhen
 */

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <string.h>
#include <cstdlib>
#include<vector>
#include <sstream>
#include <iterator>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Util.h"

// trim from start
std::string & Util::ltrim(std::string &s) {
	s.erase(s.begin(),
			std::find_if(s.begin(), s.end(),
					std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
std::string & Util::rtrim(std::string &s) {
	s.erase(
			std::find_if(s.rbegin(), s.rend(),
					std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
			s.end());
	return s;
}

// trim from both ends
std::string & Util::trim(std::string &s) {
	return ltrim(rtrim(s));
}

char * Util::dupstr(char* s) {
	char *r;
	r = (char*) malloc(strlen(s) + 1);
	bzero(r, strlen(s) + 1);
	strcpy(r, s);
	return (r);
}

std::vector<std::string> Util::split(const std::string &input) {
	std::istringstream buffer(input);
	std::vector<std::string> ret((std::istream_iterator<std::string>(buffer)),
			std::istream_iterator<std::string>());
	return ret;
}

std::vector<std::string> Util::splitByDeli(const std::string &rawinput,
		char deli) {
	std::string buffer;
	for (size_t i = 0; i < rawinput.length(); i++) {
		char c = rawinput.at(i);
		if (c == deli) {
			buffer += ' ';
			buffer += c;
			buffer += ' ';
		} else {
			buffer += c;
		}
	}
	return Util::split(buffer);
}

std::vector<std::string> Util::splitcmdline(const std::string &rawinput) {

	std::string buffer;
	for (size_t i = 0; i < rawinput.length(); i++) {
		char c = rawinput.at(i);
		if (c == '|' || c == '>' || c == '<' || c == '&') {
			buffer += ' ';
			buffer += c;
			buffer += ' ';
		} else {
			buffer += c;
		}
	}
	return Util::split(buffer);
}

bool Util::startsWith(const std::string& s, const std::string& sub) {
	return s.find(sub) == 0;
}

void Util::printvec(const std::vector<std::string>& vec) {
	for (size_t i = 0; i < vec.size(); i++) {
		printf("(%s)", vec.at(i).c_str());
	}
	printf("\n");
}

std::string Util::getvar(const std::string& varname) {
	const char * val = ::getenv(varname.c_str());
	if (val == 0) {
		return "";
	} else {
		return val;
	}
}

bool Util::isExcutableIn(const ::std::string& executable,
		const std::string& dirname) {
	DIR *d;
	struct dirent *dir;
	d = opendir(dirname.c_str());
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (executable == dir->d_name) {
				std::string file = dirname + "/" + executable;
				if (Util::isExecutable(file)) {
					return true;
				}
			}
		}
		closedir(d);
	}
	return false;
}

bool Util::isExecutable(const std::string& file){
	struct stat sb;
	if(stat(file.c_str(), &sb)!=0){
		return false;
	}
	if(S_ISREG(sb.st_mode)){
		return (sb.st_mode & S_IXUSR);
	}
	return false;
}

char** Util::vec2argv(const std::vector<std::string>& vec) {
	size_t n = vec.size();
	char** argv = new char*[n + 1];
	for (size_t i = 0; i < n; i++) {
		argv[i] = Util::dupstr((char*) vec[i].c_str());
	}
	argv[n] = (char*) 0;
	return argv;
}

void Util::free_argv(char** argv) {
	char* p;
	int i = 0;
	while ((p = argv[i++])) {
		free(p);
	}
	delete argv;
}


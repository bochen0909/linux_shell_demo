#include <stdio.h>
#include "MyShell.h"

/**
 * Entry for myshell program.
 */
int main(int argc, char** argv) {
	int exit_code = MyShell::instance().run();
	return exit_code;
}

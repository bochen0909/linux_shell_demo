CC=gcc
CXX=g++
DEBUGFLAG=-ggdb	#remove this for release
CFLAGS=-Wall -ansi -pedantic $(DEBUGFLAG)
CXXFLAGS=-Wall -ansi -pedantic -std=c++11 $(DEBUGFLAG)
LDFLAGS=
EXECUTABLE=myshell myls

all: $(EXECUTABLE)
    
myshell: ShellMain.o MyShell.o  Util.o Command.o SimpleCommand.o CompositeCommand.o
	$(CXX) $(LDFLAGS) $^ -o $@ -lreadline

myls: myls.o
	$(CXX) $(LDFLAGS) $< -o $@  	  

clean:
	rm -fr ${EXECUTABLE} *.o *.x core

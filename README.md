Project: Example of Linux Shell
==================
A demo implementation of linux shell and ls command. 

Build
==================
In the project folder run _make_, for example 

	$ make
	g++ -Wall -ansi -pedantic -std=c++11 -ggdb   -c -o ShellMain.o ShellMain.cpp
	g++ -Wall -ansi -pedantic -std=c++11 -ggdb   -c -o MyShell.o MyShell.cpp
	g++ -Wall -ansi -pedantic -std=c++11 -ggdb   -c -o Util.o Util.cpp
	g++ -Wall -ansi -pedantic -std=c++11 -ggdb   -c -o Command.o Command.cpp
	g++ -Wall -ansi -pedantic -std=c++11 -ggdb   -c -o SimpleCommand.o SimpleCommand.cpp
	g++ -Wall -ansi -pedantic -std=c++11 -ggdb   -c -o CompositeCommand.o CompositeCommand.cpp
	g++  ShellMain.o MyShell.o Util.o Command.o SimpleCommand.o CompositeCommand.o 
		-o myshell -lreadline
	g++ -Wall -ansi -pedantic -std=c++11 -ggdb   -c -o myls.o myls.cpp
	g++  myls.o -o myls
	
To clean the build
	       
	$ make clean
	rm -fr myshell myls *.o *.x core

Remark
------

 _readline_ is needed to link the program. _readline_ is used to support shell functions of history, command line editing etc. These functions make the shell more easy to use. They are not project coding requirement, so _readline_ is just used. 


Command: myls 
==================
Synopsis
--------

	myls [file or folder] [file or folder] ...

Description
-----------
myls list files or/and folders. One or more file and folder paths are specified as arguments. If no argument specified, the current working folder is listed. The output is very similar to the out put of _ls -al_ command.

Examples
--------
List a folder

	$ myls /etc/at /etc/qemu
	/etc/at:
	total 8358 bytes
	drwxr-xr-x  97 root root  4096 Oct 04 14:38 ..
	drwxr-xr-x   2 root root  4096 Sep 11 15:01 .
	-rw-r-----   1 root at     166 Sep 11 15:00 at.deny
	
	/etc/qemu:
	total 8646 bytes
	-rw-r--r--   1 root root   454 Jul 08 15:25 bridge.conf
	drwxr-xr-x  97 root root  4096 Oct 04 14:38 ..
	drwxr-xr-x   2 root root  4096 Jul 08 15:25 .
	-rw-r--r--   1 root root     0 Jul 08 15:24 target-x86_64.conf

List files

	$ myls /usr/bin/gcc /bin/ls
	/usr/bin/gcc:
	-rwxr-xr-x 1 root root 14192 Jun 01 10:24 /usr/bin/gcc
	
	/bin/ls:
	-rwxr-xr-x 1 root root 129472 Mar 19 17:45 /bin/ls

Mix file and folder

	$ myls /etc/at /usr/bin/ld
	/etc/at:
	total 8358 bytes
	drwxr-xr-x  97 root root  4096 Oct 04 14:38 ..
	drwxr-xr-x   2 root root  4096 Sep 11 15:01 .
	-rw-r-----   1 root at     166 Sep 11 15:00 at.deny
	
	/usr/bin/ld:
	lrwxrwxrwx 1 root root 22 Mar 12 05:27 /usr/bin/ld -> x86_64-pc-linux-gnu-ld
 
Show error

	$ ls /bin/something
	ls: cannot access /bin/something: No such file or directory

Restrictions
------------
* Does not support no command options of _ls_, which means you cannot change the output format.
* Only shows the following file types correctly (the first character of each line). Bit such as _seteuid_ is not supported.
	
	- directory
	- regular file
	- link file
	- fifo
	- block 
	- socket
	
Shell: myshell 
==================
Synopsis
--------

	myshell

Description
-----------
myshell is a simple shell implementation. It is not a full featured shell. See **Usage** and **Restrictions**. 

Usage
--------
Show built in command

	$ help
	help            Display information about builtin commands.
	cd              Change the shell working directory.
	pwd             Print the name of the current working directory.
	exit            Exit the shell.
	set             Set or display MyShell variables



Builtin _pwd_ compared to system _pwd_

	$ pwd
	/home/user/bob
	$ /bin/pwd
	/home/user/bob

Builtin _cd_

	$ pwd
	/home/user/bob
	$ cd ..
	$ pwd
	/home/user
	$ cd /usr/bin
	$ pwd
	/usr/bin

Builtin _set_ to set _MYPATH_

	$ set
	SHELL=MyShell
	$ set MYPATH=/home/user/bob
	$ set
	MYPATH=/home/user/bob
	SHELL=MyShell

Run _myls_ on _MYPATH_

	$ myls
	myls command not found.
	$ set MYPATH=/home/user/bob:somewhere/non/exists
	$ myls | head -5
	total 51382 bytes
	drwx------    2 bob user  4096 Oct 12 15:52 Mail
	-rw-------    1 bob user    40 Oct 12 15:38 .forward
	-rw-------    1 bob user    47 Sep 26 10:35 .bashrc
	-rw-r--r--    1 bob user    31 Aug 31 16:13 .mailboxlist


Run with pips

	$ myls |grep cpp | wc -l
	7

Run with IO redirection

	$ myls > a
	$ cat < a |grep cpp | wc -l
	7
	
	$ cat < a |grep cpp> b
	$ cat b
	-rw-------  1 bob user    6572 Oct 10 17:42 MyShell.cpp
	-rw-------  1 bob user    5268 Oct 10 17:42 myls.cpp
	-rw-------  1 bob user    1690 Oct 10 17:42 Command.cpp
	-rw-------  1 bob user     140 Oct 10 17:44 ShellMain.cpp
	-rw-------  1 bob user    3244 Oct 10 17:42 Util.cpp
	-rw-------  1 bob user    5744 Oct 10 17:42 CompositeCommand.cpp
	-rw-------  1 bob user    4706 Oct 10 17:42 SimpleCommand.cpp	

Run in background

	$ myls |grep cpp | wc -l &
	[/bin/wc] 9696
	$ 7
	[Done] 9696

	$ ps -ef|grep bob > a &
	[/bin/grep] 9780
	[Done] 9780
	$ cat a
	root      9359 20938  0 13:54 ?        00:00:00 sshd: bob [priv]
	bob      9362  9359  0 13:54 ?        00:00:00 sshd: bob@pts/16
	bob      9367  9362  0 13:54 pts/16   00:00:00 -reg-tcsh
	bob      9507  9367  0 13:58 pts/16   00:00:00 bash
	bob      9530  9507  0 13:59 pts/16   00:00:00 ./myshell
	bob      9779  9530  0 14:06 pts/16   00:00:00 /bin/ps -ef
	bob      9780  9530  0 14:06 pts/16   00:00:00 /bin/grep bob

Show errors

	$ command_not_exist
	command_not_exist command not found.
	
	$ myls > a | cat
	myls: Redirect stdout here is not allowed.
	
	$ ls |> a
	|<> cannot be at beginning and must follow something
	
	$ ls & |grep cpp
	& must be at end


Restrictions
------------
* The functions supported are all in the **Usage**. No more no less. That means 
	
	- no variable substitution on command line (e.g. ls $HOME is not expected to list file in home directory)
	- does not support comment like '#'
	- appending redirection >> is not supported. The function is easy to implemented, but it takes more to change my command parsing code.
	- whatever that is not expected from your experience from other shells.

* Does not support job control. The background processes just run in the same process group as the shell itself.
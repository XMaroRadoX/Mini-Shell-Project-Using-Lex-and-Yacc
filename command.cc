#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bits/stdc++.h>
#include <ctime>
#include <string>
#include "command.h"
using namespace std;

void childTerminated()
{
	FILE *logFile = fopen("Log.txt", "a");
	time_t current_time;
	current_time = time(NULL);
	tm *timeformatted = gmtime(&current_time);
	fprintf(logFile, "Child terminated at : %d %d %d\n", timeformatted->tm_hour, timeformatted->tm_min, timeformatted->tm_sec);
	fclose(logFile);
}
SimpleCommand::SimpleCommand()
{
	// Create available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_appendFile = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	//free dynamically allocated memory for all variables
	//if the variable was ever used
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	//reset all variables
	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_appendFile = 0;
	_background = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{
	print();
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}

	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);

	int infd;
	int outfd;
	int errfd;
	int appfd;
	int fdpipe[2];

	if (pipe(fdpipe) == -1)
	{
		perror("Error Whlie Piping");
		exit(2);
	}

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		int n;
		char *command = _simpleCommands[i]->_arguments[0];
		if (!strcasecmp(command, "exit"))
		{
			printf("GOOD BYE!!\n");
			exit(1);
		}
		if (!strcasecmp(command, "cd"))
		{
			if (_simpleCommands[i]->_numberOfArguments > 1)
			{
				string path = _simpleCommands[i]->_arguments[1];
				n = path.length();
				char path_array[n + 1];
				strcpy(path_array, path.c_str());
				if (chdir(path_array))
				{
					printf("Path not found\n");
				}
				break;
			}
			else if (true)
			{
				chdir("/mnt/");
				break;
			}
		}

		if (i == 0)
		{

			if (_inputFile != 0)
			{
				infd = open(_inputFile, O_RDONLY);
				if (infd < 0)
				{
					perror("Error while creating input file\n");
					exit(2);
				}
				// Redirect output to the created utfile instead off printing to stdout
				dup2(infd, 0);
				close(infd);
			}

			if (_inputFile != 0)
			{
				dup2(infd, 0);
			}
			else
			{
				dup2(defaultin, 0);
			}
		}
		if (i != 0)
		{
			dup2(fdpipe[0], 0);
			close(fdpipe[0]);
			close(fdpipe[1]);
			if (pipe(fdpipe) == -1)
			{
				perror("Error While Piping , Error code : 0P");
				exit(2);
			}
		}
		if (i == _numberOfSimpleCommands - 1)

		{
			if (_appendFile != 0)
			{
				appfd = open(_appendFile, O_WRONLY | O_APPEND);
				if (appfd < 0)
				{
					perror("Error while creating append file\n");
					exit(2);
				}
				// Redirect output to the created utfile instead off printing to stdout
				dup2(appfd, 1);
				close(appfd);
			}
			if (_outFile != 0)
			{
				outfd = creat(_outFile, 0666);
				if (outfd < 0)
				{
					perror("Error While Directing Output\n");
					exit(2);
				}
				// Redirect output to the created utfile instead off printing to stdout
				dup2(outfd, 1);
				close(outfd);
			}
			if (_errFile != 0)
			{
				errfd = open(_appendFile, O_WRONLY | O_APPEND);
				if (errfd < 0)
				{
					perror("Error While Directing Output\n");
					exit(2);
				}
				// Redirect output to the created utfile instead off printing to stdout
				dup2(errfd, 2);
				close(errfd);
			}
			if (_appendFile != 0)
			{
				dup2(appfd, 1);
			}
			else if (_outFile != 0)
			{
				dup2(outfd, 1);
			}
			else
			{
				dup2(defaultout, 1);
			}
		}
		if (i != _numberOfSimpleCommands - 1)
		{
			dup2(fdpipe[1], 1);
		}

		// Print contents of Command data structure

		int pid = fork();
		if (pid == -1)
		{
			perror("ls: fork\n");
			exit(2);
		}
		if (pid == 0)
		{
			if (_outFile != 0)
			{
				close(outfd);
			}
			if (_inputFile != 0)
			{
				close(infd);
			}
			if (_appendFile != 0)
			{
				close(appfd);
			}
			if (_errFile != 0)
			{
				close(errfd);
			}
			childTerminated();
			close(defaultin);
			close(defaultout);
			close(defaulterr);
			// You can use execvp() instead if the arguments are stored in an array
			execvp(command, _simpleCommands[i]->_arguments);

			// exec() is not suppose to return, something went wrong
			perror("Error While execution");
			exit(2);
		}

		if (_background == 0)
		{
			waitpid(pid, 0, 0);
		}
	}
	// Restore input, output, and error
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);

	// Close file descriptors that are not needed
	if (_inputFile != 0)
	{
		close(infd);
	}
	if (_appendFile != 0)
	{
		close(appfd);
	}
	if (_outFile != 0)
	{
		close(outfd);
	}
	if (_errFile != 0)
	{
		close(errfd);
	}
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	// Wait for last process in the pipe line

	// Clear to prepare for next command
	clear();
	// Print new prompt
	prompt();
}

// Shell implementation

void Command::prompt()
{

	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("\033[33mMarwan~\%s>", cwd);
		fflush(stdout);
	}
	else
	{
		perror("getcwd() error");
	}
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

void ignoreCtrlC(int Signal)
{
	cout << "\nCtrl+C is suppresed.Type exit to terminate \n";
}
int main()
{
	signal(SIGINT, ignoreCtrlC);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
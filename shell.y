
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE AMPERSAND PIPE APPEND SMALL AMPERSANDAPPEND

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: complex_command
        ;

complex_command:
	command_and_args piped_list iomodifier_list ampersandmodifier NEWLINE {
	printf("   Yacc: Execute complex command in background\n");
		Command::_currentCommand.execute();
	
	}| command_and_args piped_list iomodifier_list NEWLINE {
	
	printf("   Yacc: Execute complex command \n");
		Command::_currentCommand.execute();
	
	
	}
	
	 | NEWLINE
	  | error NEWLINE { yyerrok; };

simple_command:	
	command_and_args iomodifier_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;
piped_list: 
	piped_list piped_command
	| 
	;
piped_command:
	PIPE command_and_args {
	};
command_and_args:
	command_word arg_list {
		Command::_currentCommand.insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       
	       Command::_currentSimpleCommand->insertArgument( $1 );
	      
	}
	;
iomodifier_list:
	iomodifier_list iomodifier
	| /* can be empty */
	;
	

iomodifier:
	iomodifier_opt
	| ampersandappendmodifier
	| iomodifier_ipt
	| appendmodifier
	;	
	
iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| /* can be empty */ 
	;
iomodifier_ipt:
	SMALL WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| /* can be empty */ 
	;
ampersandappendmodifier:
	AMPERSANDAPPEND WORD{
		printf("   Yacc: insert ampersandappend \"%s\"\n", $2);
		Command::_currentCommand._appendFile = $2;
		Command::_currentCommand._errFile = $2;
	} 
	| /* can be empty */ 
	;
appendmodifier:
	APPEND WORD {
		printf("   Yacc: insert append \"%s\"\n", $2);
		Command::_currentCommand._appendFile = $2;
	}
	| /* can be empty */ 
	;
ampersandmodifier:
	AMPERSAND {
		printf("running in background");
		Command::_currentCommand._background = 1 ;
	
	}

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif

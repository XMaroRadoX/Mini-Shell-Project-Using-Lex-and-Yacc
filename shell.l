

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}
"&>>" {
		return AMPERSANDAPPEND;
	}		
"&"	{
		return AMPERSAND;
	}
"|"	{
		return PIPE;
	}
">>"	{
		return APPEND;
	}

">" 	{
		return GREAT;
	}
"<"	{
		return SMALL;
	}
[^ \t\n&<>|][^ \t\n&<|]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

.	{
		/* Invalid character in input */
		return NOTOKEN;
	}

%%
	

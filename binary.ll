%top{
#include "binary.tab.hh"
#include "string.h"
#define YY_DECL yy::parser::symbol_type yylex()

//#define DEBUG
#ifdef DEBUG
	unsigned int d_global_counter = 0;
	#define DEBUG_PRINT(...) \
		{ \
			if(!d_global_counter++)\
				printf("Lexer:\n");\
			printf("\t");\
			printf(__VA_ARGS__);\
		}
#else
	#define DEBUG_PRINT(...) {}
#endif
}
%option noyywrap nounput batch noinput
%x dblquote
%%

\-\-[^\n]*$									{ DEBUG_PRINT("Comment: %s\n", yytext) }									

\"[^\n\"]*\"								{
												std::string s(yytext+1);
												s.pop_back();
												DEBUG_PRINT("STRING: %s\n", s.c_str())
												return yy::parser::make_STRING(s);
											}

[0-9]+(\.[0-9]*)?							{ DEBUG_PRINT("NUMBER %f\n", std::stof(yytext)) return yy::parser::make_NUMBER(std::stod(yytext));}

\,											{ DEBUG_PRINT("COMMA\n") return yy::parser::make_COMMA(); }
\:											{ DEBUG_PRINT("COLON\n") return yy::parser::make_COLON(); }
\;											{ DEBUG_PRINT("SEMI\n") return yy::parser::make_SEMI(); }
\.											{ DEBUG_PRINT("POINT\n") return yy::parser::make_POINT(); }

==											{ DEBUG_PRINT("L_EQ\n") return yy::parser::make_L_EQ(); }
\<											{ DEBUG_PRINT("L_L\n") return yy::parser::make_L_L(); }
\>											{ DEBUG_PRINT("L_G\n") return yy::parser::make_L_G(); }
\<=											{ DEBUG_PRINT("L_LEQ\n") return yy::parser::make_L_LEQ(); }
\>=											{ DEBUG_PRINT("L_GEQ\n") return yy::parser::make_L_GEQ(); }
\~=											{ DEBUG_PRINT("L_NEQ\n") return yy::parser::make_L_NEQ(); }
and											{ DEBUG_PRINT("AND\n") return yy::parser::make_AND(); }
or											{ DEBUG_PRINT("OR\n") return yy::parser::make_OR(); }

=											{ DEBUG_PRINT("EQUAL\n") return yy::parser::make_EQUAL(); }

true										{ DEBUG_PRINT("TRUE\n") return yy::parser::make_TRUE(); }
false										{ DEBUG_PRINT("FALSE\n") return yy::parser::make_FALSE(); }
nil											{ DEBUG_PRINT("NIL\n") return yy::parser::make_NIL(); }
for											{ DEBUG_PRINT("FOR\n") return yy::parser::make_FOR(); }
repeat										{ DEBUG_PRINT("REPEAT\n") return yy::parser::make_REPEAT(); }
until										{ DEBUG_PRINT("UNTIL\n") return yy::parser::make_UNTIL(); }
do											{ DEBUG_PRINT("DO\n") return yy::parser::make_DO(); }
end											{ DEBUG_PRINT("END\n") return yy::parser::make_END(); }
if 											{ DEBUG_PRINT("IF\n") return yy::parser::make_IF(); }
then										{ DEBUG_PRINT("THEN\n") return yy::parser::make_THEN(); }
else										{ DEBUG_PRINT("ELSE\n") return yy::parser::make_ELSE(); }
elseif										{ DEBUG_PRINT("ELSEIF\n") return yy::parser::make_ELSEIF(); }

function									{ DEBUG_PRINT("FUNCTION\n") return yy::parser::make_FUNCTION(); }
return										{ DEBUG_PRINT("RETURN\n") return yy::parser::make_RETURN(); }
break										{ DEBUG_PRINT("BREAK\n") return yy::parser::make_BREAK(); }

\^											{ DEBUG_PRINT("CARET\n") return yy::parser::make_CARET(); }
\+											{ DEBUG_PRINT("ADD\n") return yy::parser::make_ADD(); }
\-											{ DEBUG_PRINT("SUB\n") return yy::parser::make_SUB(); }
\*											{ DEBUG_PRINT("MUL\n") return yy::parser::make_MUL(); }
\/											{ DEBUG_PRINT("DIV\n") return yy::parser::make_DIV(); }
\%											{ DEBUG_PRINT("MOD\n") return yy::parser::make_MOD(); }
\#											{ DEBUG_PRINT("HASH\n") return yy::parser::make_HASH(); }

\(											{ DEBUG_PRINT("PAREN_OPEN\n") return yy::parser::make_PAREN_OPEN(); }
\)											{ DEBUG_PRINT("PAREN_CLOSE\n") return yy::parser::make_PAREN_CLOSE(); }
\{											{ DEBUG_PRINT("CURLY_BRACKET_OPEN\n") return yy::parser::make_CURLY_BRACKET_OPEN(); }
\}											{ DEBUG_PRINT("CURLY_BRACKET_CLOSE\n") return yy::parser::make_CURLY_BRACKET_CLOSE(); }
\[											{ DEBUG_PRINT("SQUARE_BRACKET_OPEN\n") return yy::parser::make_SQUARE_BRACKET_OPEN(); }
\]											{ DEBUG_PRINT("SQUARE_BRACKET_CLOSE\n") return yy::parser::make_SQUARE_BRACKET_CLOSE(); }

[_a-zA-Z][_a-zA-Z0-9]*						{ DEBUG_PRINT("NAME: %s\n", yytext) return yy::parser::make_NAME(yytext); }

[ \t]*										{
												//int numSpaces = std::string(yytext).size();
												//DEBUG_PRINT("SPACE: %dx\n", numSpaces)
												//return yy::parser::make_SPACE(numSpaces); 
											}

[\n]+										{ /*DEBUG_PRINT("NL\n") return yy::parser::make_NL();*/ }

.											{ DEBUG_PRINT("MISC: %s\n", yytext) }

	/* End of file */
<<EOF>>       								return yy::parser::make_FILE_END();
%%
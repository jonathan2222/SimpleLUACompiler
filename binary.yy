%skeleton "lalr1.cc" 
%defines
%define parse.error verbose
%define api.value.type variant
%define api.token.constructor

%code requires{
	#include "AST/AST.hh"
}

%code{
	#include <string>
	#include <iostream>
	#define YY_DECL yy::parser::symbol_type yylex()
	YY_DECL;

	Node* root = new Node("Stream");
}

%token <double> NUMBER
%token <std::string> NAME STRING
%token FOR DO END REPEAT UNTIL
%token FILE_END 0 "end of file"

/* Math */
%token CARET	 			"^"
%token ADD 					"+"
%token SUB 					"-"
%token MUL 					"*"
%token DIV 					"/"
%token MOD					"%"
%type <Node*> num

/* Logic */
%token IF
%token THEN
%token ELSE
%token ELSEIF
%token L_EQ					"=="
%token L_L					"<"
%token L_G					">"
%token L_LEQ				"<="
%token L_GEQ				">="
%token L_NEQ				"~="
%token AND
%token OR
%token NIL
%token TRUE
%token FALSE

/* Strings */
%token HASH					"#"

/* Brackets */
%token PAREN_OPEN 			"("
%token PAREN_CLOSE 			")"
%token CURLY_BRACKET_OPEN 	"{"
%token CURLY_BRACKET_CLOSE 	"}"
%token SQUARE_BRACKET_OPEN 	"["
%token SQUARE_BRACKET_CLOSE "]"
%token EQUAL				"="

%token COLON				":"
%token SEMI					";"
%token COMMA 				","
%token POINT 				"."

/* Reserved words */
%token FUNCTION				"function"
%token RETURN				"return"
%token BREAK				"break"

/* function and vars */
%type <Node*> var_list var prefixexp functioncall args return function funcbody optname_list name_list

%type <Node*> elseif_list elseif

/* exp */
%type <Node*> exp_list exp

/* Table */
%type <Node*> table optfield_list field_list field_step field

/* Lines */
%type <Node*> stream stat last_stat ichunk block iblock

%left OR
%left AND
%left "==" "<=" ">=" "<" ">" "~="
%left "+" "-"
%left "*" "/" "%"
%right "^"
%left UHASH USUB

%%
stream:
	block							{ $$ = root; $$->addChild($1, true); }
	;

block:
	iblock							{ $$ = $1; }
	| last_stat						{ $$ = new Block(); $$->addChild($1); }
	;

iblock:
	ichunk							{ $$ = $1; }
	| iblock last_stat				{ $$ = $1; $$->addChild($2); }
	;

ichunk:
	stat							{ $$ = new Block(); $$->addChild($1); }
	| block stat					{ $$ = $1; $$->addChild($2); }
	| block ";" stat				{ $$ = $1; $$->addChild($3); }
	;

last_stat:
	return							{ $$ = $1; }
	| "break"						{ $$ = new Node("Break"); }
	;

return:
	"return"						{ $$ = new Return(); }
	| "return" exp_list				{ $$ = new Return(); $$->addChild($2); }
	;

stat:
	var_list "=" exp_list			{ $$ = new Assignment(); $$->addChild($1); $$->addChild($3); }
	| functioncall					{ $$ = $1; }
	| function						{ $$ = $1; }
	| DO block END					{ $$ = $2; }
	| IF exp THEN block elseif_list END	{
										$$ = new If(); 
										$$->addChild($2); 
										$$->addChild($4);
										if($5 != nullptr)
											$$->addChild($5);
									}
	| FOR NAME "=" exp "," exp DO block END { 
										$$ = new For();
										$$->addChild(new Var($2));
										$$->addChild($4);
										$$->addChild($6);
										$$->addChild($8);
									}
	| FOR NAME "=" exp "," exp "," exp DO block END { 
										$$ = new Node("ForEx");
										$$->addChild(new Var($2));
										$$->addChild($4);
										$$->addChild($6);
										$$->addChild($8);
										$$->addChild($10);
									}
	| REPEAT block UNTIL exp		{
										$$ = new Repeat();
										$$->addChild($2);
										$$->addChild($4);
									}
	;

elseif_list:
	elseif							{ $$ = $1; }
	| elseif_list elseif			{ $$ = $1; if($2 != nullptr) $$->addChild($2); }
	;

elseif:
	/* empty */						{ $$ = nullptr; }
	| ELSE block					{ $$ = $2; }
	| ELSEIF exp THEN block			{ 
										$$ = new If();
										$$->addChild($2);
										$$->addChild($4);
									}
	;

args:
	"(" exp_list ")"				{ $$ = new Node("Args"); $$->addChild($2); }
	| "(" ")"						{ $$ = new Node("Args"); }
	| STRING						{ $$ = new Constant($1); }
	| table							{ $$ = $1; }
	;

exp_list:
	exp								{ $$ = new Node("Exp_list"); $$->addChild($1); }
	| exp_list "," exp				{ $$ = $1; $$->addChild($3); }
	;

var_list:
	var								{ $$ = new Node("Var_list"); $$->addChild($1); }
	| var_list "," var				{ $$ = $1; $$->addChild($3); }
	;

var:
	NAME							{ $$ = new Var($1); }
	| prefixexp "." NAME			{ $$ = $1; $$->addChild(new Var($3)); }
	| prefixexp "[" exp "]"			{ 
										$$ = new TableIndex();
										$$->addChild($1);
										$$->addChild($3);
									}
	;

prefixexp:
	var								{ $$ = $1; }
	| functioncall					{ $$ = $1; }
	;

functioncall:
	prefixexp args					{ 
										$$ = new FunctionCall();
										$$->addChild($1); 
										$$->addChild($2); 
									}
	| prefixexp ":" NAME args		{ 
										$$ = new FunctionCall();
										$$->addChild($1);
										$$->addChild(new Var($3));
										$$->addChild($4); 
									}
	;

function:
	"function" NAME funcbody				{ 
												$$ = new Function();
												$$->addChild(new Var($2));
												$$->addChild($3); }
	;

funcbody:
	"(" optname_list ")" block END	{ 
										$$ = new Node("FunctionBody");
										if($2 != nullptr)	
											$$->addChild($2); 
										$$->addChild($4);
									}
	;

optname_list:
	/* empty */						{ $$ = nullptr; }
	| name_list						{ $$ = $1; }
	;

name_list:
	NAME							{ $$ = new Node("Name_list"); $$->addChild(new Var($1)); }
	| name_list ',' NAME			{ $$ = $1; $$->addChild(new Var($3)); }
	;

exp:
	num								{ $$ = $1; }
	| STRING						{ $$ = new Constant($1); }
	| prefixexp						{ $$ = $1; }
	| table							{ $$ = $1; }
	| exp "+" exp					{ 
										$$ = new Operation("Add", "+");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp "-" exp					{
										$$ = new Operation("Sub", "-");
										$$->addChild($1); 
										$$->addChild($3);
									}
	| exp "*" exp					{
										$$ = new Operation("Mul", "*");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp "/" exp					{ 
										$$ = new Operation("Div", "/");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp "%" exp					{
										$$ = new Operation("Mod", "%");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp "^" exp					{
										$$ = new Operation("Caret", "^");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp "==" exp					{
										$$ = new Operation("L_eq", "==");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp "<=" exp					{
										$$ = new Operation("L_leq", "<=");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp ">=" exp					{
										$$ = new Operation("L_geq", ">=");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp "<" exp					{
										$$ = new Operation("L_l", "<");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp ">" exp					{
										$$ = new Operation("L_g", ">");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp "~=" exp					{
										$$ = new Operation("L_neq", "~=");
										$$->addChild($1); 
										$$->addChild($3); 
									}
	| exp AND exp					{ 
										/* This is not used in the test cases! */
										$$ = new Operation("L_and", "&&"); 
										$$->addChild($1);
										$$->addChild($3); 
									}
	| exp OR exp					{ 
										/* This is not used in the test cases! */
										$$ = new Operation("L_or", "||"); 
										$$->addChild($1);
										$$->addChild($3); 
									}
	| "#" exp %prec UHASH			{
										$$ = new Hash();
										$$->addChild($2);
									}
	| "-" exp %prec USUB			{
										$$ = new USubN("USub");
										$$->addChild($2);
									}
	| "(" exp ")"					{
										/* This should be under prefixexp, but that will cause reduce/reduce. */ 
										$$ = new Paren(); 
										$$->addChild($2); 
									}
	;

table:
	"{" optfield_list "}"			{
										$$ = new Table();
										if($2 != nullptr)
										{
											Node*& node = $2;
											for(Node*& child : node->children)
												$$->addChild(child);
										}
									}
	;

optfield_list:
	/* Empty */						{ $$ = nullptr; }
	| field_list					{ $$ = $1; }
	;

field_list:
	field							{ $$ = new Node("Field_list"); $$->addChild($1); }
	| field_list field_step field   { $$ = $1; $$->addChild($3); } 
	;

field_step:
	","								{ $$ = nullptr; }
	| ";"							{ $$ = nullptr; }
	;

field:
	"[" exp "]" "=" exp				{ 
										$$ = new Node("TBE_exp_exp");
										$$->addChild($2);
										$$->addChild($5);
									}
	| NAME "=" exp					{ 
										$$ = new Node("TBE_name_exp");
										$$->addChild(new Node("Name", Data($1, $1)));
										$$->addChild($3);
									}
	| exp							{
										$$ = new Node("TBE_exp");
										$$->addChild($1);
									}
	;

num:
	NUMBER							{ $$ = new Constant($1); }
	| NIL							{ $$ = new Constant(); /* Nil Object*/ }
	| TRUE							{ $$ = new Constant(true); }
	| FALSE							{ $$ = new Constant(false); }
	;


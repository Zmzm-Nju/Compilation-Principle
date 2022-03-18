%option yylineno
%{
#include <string.h>
#include "syntax.tab.h"
struct Token
{
	int col;
	int is_terminal;
	char token_type[100];
	char token_text[100];
	struct Token *firstChild;
	struct Token *nextSibling;
};
int Program_Is_Correct = 1;
void SetTokenValue(char* type, char* text);
%}
letter_  [A-Za-z_]
digit 	 [0-9]
INT 	 (0|[1-9]{digit}*)
OCT 	 (0[0-7]+)
OCTERROR (0[0-7]*[8-9]+[0-9]*)
HEX		 ((0x|0X)[0-9a-fA-F]+)
HEXERROR ((0x|0X)[0-9a-fA-F]*[g-zG-Z]+[0-9a-zA-Z]*)
FLOAT	 (({digit}*\.{digit}+)|({digit}+\.{digit}*))([Ee]?[+-]?{digit}+)?
ID	     {letter_}({letter_}|{digit})*	
RELOP 	 >|<|>=|<=|==|!=
%%
" "	    {;}
"\t"	{;}
"\n"	{;}
"\r"	{;}
{OCTERROR} {
	Program_Is_Correct = 0;
	printf("Error type A at Line %d: Illegal octal number\'%s\'.\n",yylineno, yytext);
	SetTokenValue("INT", "0");
    return INT; 
}
{HEXERROR} {
	Program_Is_Correct = 0;
	printf("Error type A at Line %d: Illegal hexadecimal number\'%s\'.\n",yylineno, yytext);
	SetTokenValue("INT", "0");
    return INT;
}
{INT}	{SetTokenValue("INT",yytext);return INT;}
{OCT}	{SetTokenValue("INT",yytext);return INT;}
{HEX}	{SetTokenValue("INT",yytext);return INT;}
{FLOAT}	{SetTokenValue("FLOAT",yytext);return FLOAT;}
";"	    {SetTokenValue("SEMI",yytext);return SEMI;}
","	    {SetTokenValue("COMMA",yytext);return COMMA;}
{RELOP}	{SetTokenValue("RELOP",yytext);return RELOP;}
"="	    {SetTokenValue("ASSIGNOP",yytext);return ASSIGNOP;}
"+"	    {SetTokenValue("PLUS",yytext);return PLUS;}
"-"	    {SetTokenValue("MINUS",yytext);return MINUS;}
"*"	    {SetTokenValue("STAR",yytext);return STAR;}
"/"	    {SetTokenValue("DIV",yytext);return DIV;}
"&&"	{SetTokenValue("AND",yytext);return AND;}
"||"	{SetTokenValue("OR",yytext);return OR;}
"."	    {SetTokenValue("DOT",yytext);return DOT;}
"!"	    {SetTokenValue("NOT",yytext);return NOT;}
"("	    {SetTokenValue("LP",yytext);return LP;}
")"	    {SetTokenValue("RP",yytext);return RP;}
"["	    {SetTokenValue("LB",yytext);return LB;}
"]"	    {SetTokenValue("RB",yytext);return RB;}
"{"	    {SetTokenValue("LC",yytext);return LC;}
"}"	    {SetTokenValue("RC",yytext);return RC;}
"struct"	{SetTokenValue("STRUCT",yytext);return STRUCT;}
"return"	{SetTokenValue("RETURN",yytext);return RETURN;}
"if"	{SetTokenValue("IF",yytext);return IF;}
"else"	{SetTokenValue("ELSE",yytext);return ELSE;}
"while"	{SetTokenValue("WHILE",yytext);return WHILE;}
"int"	{SetTokenValue("TYPE",yytext);return TYPE;}
"float"	{SetTokenValue("TYPE",yytext);return TYPE;}
{ID}	{SetTokenValue("ID",yytext);return ID;}
.	    {Program_Is_Correct = 0; printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);}
"//" 	{char c = input(); while (c != '\n') c = input();}
%%
void SetTokenValue(char* type, char* text)
{
	yylval.token = (struct Token *)malloc(sizeof(struct Token));
	yylval.token->col = yylineno;
	yylval.token->is_terminal = 1;
	strcpy(yylval.token->token_type, type);
	strcpy(yylval.token->token_text, text);
	yylval.token->firstChild = NULL;
	yylval.token->nextSibling = NULL;
}

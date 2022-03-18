#ifndef Token_H
#define Token_H
struct Token
{
	int col;
	int is_terminal;
	char token_type[100];
	char token_text[100];
	struct Token *firstChild;
	struct Token *nextSibling;
};
#endif
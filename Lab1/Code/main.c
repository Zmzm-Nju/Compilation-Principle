#include <stdio.h>
#include "syntax.tab.h"
extern int Program_Is_Correct;
extern struct Token* Root;
void PrintTree(struct Token* root, int depth);
int main(int argc, char** argv)
{
	if(argc <= 1)
	{
		yyparse();
		return 1;
	}
	FILE* fp = fopen(argv[1], "r");
	if (!fp)
	{
		perror(argv[1]);
		return 1;
	}
	yyrestart(fp);
	yyparse();
	if(Program_Is_Correct)
		PrintTree(Root, 0);
	return 0;
}
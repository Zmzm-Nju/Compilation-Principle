#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "semantic.h"
#include "syntax.tab.h"
#include "intermediate.h"
extern int Program_Is_Correct;
extern int Translate_Is_Correct;
extern struct Token* Root;
void PrintTree(struct Token* root, int depth);
int main(int argc, char** argv)
{
	if(argc <= 1)
	{
		yyparse();
		return 1;
	}
	FILE* fp1 = fopen(argv[1], "r");
	if (!fp1)
	{
		perror(argv[1]);
		return 1;
	}
	FILE* fp2 = fopen(argv[2], "wt+");
	if (!fp2)
	{
		perror(argv[2]);
		return 1;
	}
	yyrestart(fp1);
	yyparse();
	if(Program_Is_Correct)
	{
		//PrintTree(Root, 0);
		init_table();
		TraverseTree(Root);
		TraverseTree_InterCode(Root);
		if(Translate_Is_Correct)
		{
			//Generate_Intercode_File(fp2);
			Generate_Objectcode_File(fp2);
		}
	}
	return 0;
}
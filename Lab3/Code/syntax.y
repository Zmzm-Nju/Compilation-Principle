%{
    #include <stdio.h>
    #include <stdarg.h>
    #include "lex.yy.c"
    int  otoi(char* s);
	int  htoi(char* s);
    void PrintTree(struct Token* root, int depth);
	struct Token* CreatNode(char *type, int num, ...);
	struct Token* Root = NULL;
	extern int Program_Is_Correct;
%}
%error-verbose
/* declared types */
%union {
	struct Token* token;
}
/* declared tokens */
%token <token> INT
%token <token> FLOAT
%token <token> SEMI
%token <token> COMMA 
%token <token> RELOP 
%token <token> ASSIGNOP
%token <token> PLUS MINUS STAR DIV
%token <token> AND 
%token <token> OR 
%token <token> DOT 
%token <token> NOT 
%token <token> LP 
%token <token> RP 
%token <token> LB 
%token <token> RB 
%token <token> LC 
%token <token> RC
%token <token> STRUCT 
%token <token> RETURN 
%token <token> IF 
%token <token> ELSE 
%token <token> WHILE 
%token <token> TYPE
%token <token> ID
/* declared non-terminals */
%type <token> Program ExtDefList ExtDef ExtDecList
%type <token> Specifier StructSpecifier OptTag Tag
%type <token> VarDec FunDec VarList ParamDec
%type <token> CompSt StmtList Stmt 
%type <token> DefList Def DecList Dec
%type <token> Exp Args
/* deal with ambiguity */
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT
%left LB RB
%left LP RP
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%%
//7.1.2 High-level Definitions
Program: ExtDefList					{$$ = CreatNode("Program",1,$1); Root = $$;}
	;
ExtDefList: ExtDef ExtDefList		{$$ = CreatNode("ExtDefList",2,$1,$2);}				
	| 								{$$ = NULL;}
	;
ExtDef : Specifier ExtDecList SEMI	{$$ = CreatNode("ExtDef",3,$1,$2,$3);}
	| Specifier SEMI				{$$ = CreatNode("ExtDef",2,$1,$2);}
	| Specifier FunDec CompSt		{$$ = CreatNode("ExtDef",3,$1,$2,$3);}
	| error SEMI					{Program_Is_Correct = 0;}
	;
ExtDecList : VarDec					{$$ = CreatNode("ExtDecList",1,$1);}
	| VarDec COMMA ExtDecList		{$$ = CreatNode("ExtDecList",3,$1,$2,$3);}
	;
//7.1.3 Specifiers
Specifier : TYPE					{$$ = CreatNode("Specifier",1,$1);}
	| StructSpecifier				{$$ = CreatNode("Specifier",1,$1);}
	;
StructSpecifier : STRUCT OptTag LC DefList RC	{$$ = CreatNode("StructSpecifier",5,$1,$2,$3,$4,$5);}
	| STRUCT Tag								{$$ = CreatNode("StructSpecifier",2,$1,$2);}
//	| error RC									{Program_Is_Correct = 0;}
	;
OptTag : ID				{$$ = CreatNode("OptTag",1,$1);}
	| 					{$$ = NULL;}
	;
Tag : ID				{$$ = CreatNode("Tag",1,$1);}
	;
//7.1.4 Declarators
VarDec : ID				{$$ = CreatNode("VarDec",1,$1);}
	| VarDec LB INT RB	{$$ = CreatNode("VarDec",4,$1,$2,$3,$4);}
	| error RB			{Program_Is_Correct=0;}
	;
FunDec : ID LP VarList RP	{$$ = CreatNode("FunDec",4,$1,$2,$3,$4);}
	| ID LP RP				{$$ = CreatNode("FunDec",3,$1,$2,$3);}
	| error RP				{Program_Is_Correct=0;}
	;
VarList : ParamDec COMMA VarList	{$$ = CreatNode("VarList",3,$1,$2,$3);}
	| ParamDec						{$$ = CreatNode("VarList",1,$1);}
	;
ParamDec : Specifier VarDec			{$$ = CreatNode("ParamDec",2,$1,$2);}
	;
//7.1.5 Statements
CompSt : LC DefList StmtList RC		{$$ = CreatNode("CompSt",4,$1,$2,$3,$4);}
	| error RC						{Program_Is_Correct=0;}
	;
StmtList : Stmt StmtList		{$$ = CreatNode("StmtList",2,$1,$2);}
	|							{$$ = NULL;}
	;
Stmt : Exp SEMI					{$$ = CreatNode("Stmt",2,$1,$2);}
	| error SEMI				{Program_Is_Correct=0;}
	| CompSt					{$$ = CreatNode("Stmt",1,$1);}
	| RETURN Exp SEMI			{$$ = CreatNode("Stmt",3,$1,$2,$3);}	
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE	{$$ = CreatNode("Stmt",5,$1,$2,$3,$4,$5);}
	| IF LP Exp RP Stmt ELSE Stmt				{$$ = CreatNode("Stmt",7,$1,$2,$3,$4,$5,$6,$7);}
	| WHILE LP Exp RP Stmt						{$$ = CreatNode("Stmt",5,$1,$2,$3,$4,$5);}
	;
//7.1.6 Local Definitions
DefList : Def DefList			{$$ = CreatNode("DefList",2,$1,$2);}
	|							{$$ = NULL;}
	; 	
Def : Specifier DecList SEMI	{$$ = CreatNode("Def",3,$1,$2,$3);}
//	| error SEMI				{Program_Is_Correct=0;}
	;
DecList : Dec					{$$ = CreatNode("DecList",1,$1);}
	| Dec COMMA DecList			{$$ = CreatNode("DecList",3,$1,$2,$3);}
	;
Dec : VarDec					{$$ = CreatNode("Dec",1,$1);}
	| VarDec ASSIGNOP Exp		{$$ = CreatNode("Dec",3,$1,$2,$3);}
	;
//7.1.7 Expressions
Exp : Exp ASSIGNOP Exp			{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| Exp AND Exp				{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| Exp OR Exp				{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| Exp RELOP Exp				{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| Exp PLUS Exp				{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| Exp MINUS Exp				{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| Exp STAR Exp				{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| Exp DIV Exp				{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| LP Exp RP					{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| MINUS Exp					{$$ = CreatNode("Exp",2,$1,$2);}
	| NOT Exp					{$$ = CreatNode("Exp",2,$1,$2);}
	| ID LP Args RP				{$$ = CreatNode("Exp",4,$1,$2,$3,$4);}
	| ID LP RP					{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| Exp LB Exp RB				{$$ = CreatNode("Exp",4,$1,$2,$3,$4);}
	| Exp DOT ID				{$$ = CreatNode("Exp",3,$1,$2,$3);}
	| ID						{$$ = CreatNode("Exp",1,$1);}
	| INT						{$$ = CreatNode("Exp",1,$1);}
	| FLOAT						{$$ = CreatNode("Exp",1,$1);}
	;
Args : Exp COMMA Args			{$$ = CreatNode("Args",3,$1,$2,$3);}
	| Exp						{$$ = CreatNode("Args",1,$1);}
	;
%%
struct Token* CreatNode(char *type, int num,...)
{
    //printf("%s\n",type);
	struct Token* cur = (struct Token*)malloc(sizeof(struct Token));
	struct Token* temp = NULL;

	va_list valist;
	va_start(valist, num);
	temp = va_arg(valist, struct Token*);

	cur->col = temp->col;
	cur->is_terminal = 0;
	strcpy(cur->token_type, type);
	strcpy(cur->token_text, "");
	cur->firstChild = temp;
    cur->nextSibling = NULL;

	for(int i = 1; i < num; i++)
	{
		temp->nextSibling = va_arg(valist, struct Token*);
		if(temp->nextSibling)
			temp = temp->nextSibling;
	}
	return cur;
}

void PrintTree(struct Token* root, int depth)
{
	if(root == NULL) return;
	for(int i = 0; i < depth; i++)
		printf("  ");
	printf("%s",root->token_type);
	if(!root->is_terminal)
		printf(" (%d)",root->col);
	else if(strcmp(root->token_type,"ID")==0 || strcmp(root->token_type,"TYPE")==0)
		printf(": %s",root->token_text);
	else if(strcmp(root->token_type,"INT")==0)
	{
		int num = 0;
		if(strcmp(root->token_text,"0") == 0) ;
		else if(root->token_text[0] =='0' && (root->token_text[1]=='x'||root->token_text[1]=='X')) num = htoi(root->token_text);
		else if(root->token_text[0] =='0') num = otoi(root->token_text);
		else num = atoi(root->token_text);
		printf(": %d", num);
	}
	else if(strcmp(root->token_type,"FLOAT")==0)
		printf(": %lf",atof(root->token_text));
	printf("\n");
	PrintTree(root->firstChild, depth + 1);
	PrintTree(root->nextSibling, depth);
}

void yyerror(char* msg)
{
    Program_Is_Correct = 0;
	fprintf(stderr, "Error type B at line %d: %s\n",yylineno, msg);
}

int otoi(char* s)  
{  
    int n = 0;  
    for (int i = 0; s[i]; i++)  
		n = 8 * n + s[i] - '0';  
    return n;  
}

int htoi(char* s)  
{  
    int n = 0;  
    for (int i = 2; s[i]; i++)  
    {  
        if (s[i] >= 'a' && s[i] <= 'z')  
            n = 16 * n + (10 + s[i] - 'a');  
        else if(s[i] >='A' && s[i] <= 'Z')
            n = 16 * n + (10 + s[i] - 'A');
		else
			n = 16 * n + s[i] - '0';  
    }  
    return n;  
}
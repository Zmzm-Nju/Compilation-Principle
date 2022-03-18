#ifndef _INTERMEDIATE_H_
#define _INTERMEDIATE_H_
#define MAX_LINE 100000
#include "token.h"
#include <stdio.h>
typedef struct Operand_* Operand;
typedef struct Arg_List_* Arg_List;
struct Operand_
{
    enum
    {
        OP_VARIABLE,
        OP_CONSTANT,
        OP_ADDRESS,
        OP_LABEL,
        OP_FUNCTION,
		OP_RELOP
    } kind;
    struct {
        int var_no;
        int value;
	    char name[100];
		int is_addr;
    } u;
};
struct InterCode
{
    enum
    {
        IR_LABEL,IR_FUNCTION,IR_ASSIGN,
        IR_ADD,IR_SUB,IR_MUL,IR_DIV,
        IR_GET_ADDR,IR_INTO_ADDR_RIGHT,IR_INTO_ADDR_LEFT,
        IR_GOTO,IR_IF_GOTO,IR_RETURN,
        IR_DEC,IR_ARG,IR_CALL,IR_PARAM,IR_READ,IR_WRITE
    } kind;
    union {
        struct
        {
            Operand op;
        } one;
        struct
        {
            Operand right, left;
        } two;
        struct
        {
            Operand result, op1, op2;
        } three;
        struct
        {
            Operand x, relop, y, z;
        } if_goto;
		struct
		{
			Operand op;
			int size;
		}dec;	
    } u;
};
struct Arg_List_
{
	Operand op;
	Arg_List next;
};
Operand new_temp();
Operand new_label();
Arg_List Translate_Args(struct Token* node);
void Print_Operand(FILE* fp, Operand op);
void Generate_Intercode_File(FILE* fp);
void Translate_Exp(struct Token* node, Operand dst);
void Translate_VarDec(struct Token* node, Operand dst);
void Translate_ExtDef(struct Token* node);
void Translate_ExtDefList(struct Token* node);
void Translate_FunDec(struct Token* node);
void Translate_CompSt(struct Token* node);
void Translate_Dec(struct Token* node);
void Translate_DecList(struct Token* node);
void Translate_Def(struct Token* node);
void Translate_DefList(struct Token* node);
void Translate_Stmt(struct Token* node);
void Translate_StmtList(struct Token* node);
void Translate_Cond(struct Token* node, Operand label_true, Operand label_false);
void create_one_intercode(Operand op, int kind);
void create_two_intercode(Operand op1, Operand op2, int kind);
void create_three_intercode(Operand result, Operand op1,Operand op2, int kind);
void TraverseTree_InterCode(struct Token* node);
#endif
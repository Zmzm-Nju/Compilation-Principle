//#define debug
#include "semantic.h"
#include "intermediate.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int cur_index = 0;
int label_num = 1;
int temp_var_num = 1;
struct InterCode codes[MAX_LINE];
extern Item Symbol_Table;
int Translate_Is_Correct = 1;
void Print_Operand(FILE* fp, Operand op)
{
	switch(op->kind)
	{
		case OP_VARIABLE:
			fprintf(fp, "%s",op->u.name);break;
		case OP_CONSTANT:
			fprintf(fp, "#%d",op->u.value);break;
		case OP_ADDRESS:
			fprintf(fp, "%s",op->u.name);break;
		case OP_LABEL:
			fprintf(fp, "%s",op->u.name);break;
		case OP_FUNCTION:
		case OP_RELOP:
			fprintf(fp, "%s",op->u.name);break;
	}
}
void Generate_Intercode_File(FILE* fp)
{
	int x = 0;
    while(x < cur_index)
    {
    	switch(codes[x].kind)
    	{
    		case IR_LABEL:
    			fprintf(fp, "LABEL ");
    			Print_Operand(fp, codes[x].u.one.op);
    			fprintf(fp, " :");
    			break;
    		case IR_FUNCTION:
    		    fprintf(fp, "FUNCTION ");
    			Print_Operand(fp, codes[x].u.one.op);
    			fprintf(fp, " :");
    			break;
    		case IR_ASSIGN:
    		    Print_Operand(fp, codes[x].u.two.left);
		        fprintf(fp, " := ");
				Print_Operand(fp, codes[x].u.two.right);
    			break;
    		case IR_ADD:
    			Print_Operand(fp, codes[x].u.three.result);
		        fprintf(fp, " := ");
				Print_Operand(fp, codes[x].u.three.op1);
				fprintf(fp, " + ");
				Print_Operand(fp, codes[x].u.three.op2);
				break;
    		case IR_SUB:
    			Print_Operand(fp, codes[x].u.three.result);
		        fprintf(fp, " := ");
				Print_Operand(fp, codes[x].u.three.op1);
				fprintf(fp, " - ");
				Print_Operand(fp, codes[x].u.three.op2);
				break;
    		case IR_MUL:
    			Print_Operand(fp, codes[x].u.three.result);
		        fprintf(fp, " := ");
				Print_Operand(fp, codes[x].u.three.op1);
				fprintf(fp, " * ");
				Print_Operand(fp, codes[x].u.three.op2);
				break;
    		case IR_DIV:
    			Print_Operand(fp, codes[x].u.three.result);
		        fprintf(fp, " := ");
				Print_Operand(fp, codes[x].u.three.op1);
				fprintf(fp, " / ");
				Print_Operand(fp, codes[x].u.three.op2);
				break;
			case IR_GET_ADDR:
    		    Print_Operand(fp, codes[x].u.two.left);
		        fprintf(fp, " := &");
				Print_Operand(fp, codes[x].u.two.right);
    			break;				
			case IR_INTO_ADDR_RIGHT:
			   	Print_Operand(fp, codes[x].u.two.left);
		        fprintf(fp, " := *");
				Print_Operand(fp, codes[x].u.two.right);
    			break;			
			case IR_INTO_ADDR_LEFT:
				fprintf(fp, "*");
    		    Print_Operand(fp, codes[x].u.two.left);
		        fprintf(fp, " := ");
				Print_Operand(fp, codes[x].u.two.right);
    			break;
    		case IR_GOTO:
    			fprintf(fp, "GOTO ");
    			Print_Operand(fp, codes[x].u.one.op);
    			break;
    		case IR_IF_GOTO:
    			fprintf(fp, "IF ");
    			Print_Operand(fp, codes[x].u.if_goto.x);
    			fprintf(fp, " ");
     			Print_Operand(fp, codes[x].u.if_goto.relop);
     			fprintf(fp, " ");
     			Print_Operand(fp, codes[x].u.if_goto.y);
     			fprintf(fp, " GOTO ");
    			Print_Operand(fp, codes[x].u.if_goto.z);
    			break;  
    		case IR_RETURN:
    			fprintf(fp, "RETURN ");
    			Print_Operand(fp, codes[x].u.one.op);
    			break;
    		case IR_DEC:
    			fprintf(fp, "DEC ");
    			Print_Operand(fp, codes[x].u.dec.op);
    			fprintf(fp, " ");
    			fprintf(fp, "%d",codes[x].u.dec.size);
    			break;
    		case IR_ARG:
    			fprintf(fp, "ARG ");
    			Print_Operand(fp, codes[x].u.one.op);
    			break;
    		case IR_CALL:
    			Print_Operand(fp, codes[x].u.two.left);
    			fprintf(fp, " := CALL ");
    			Print_Operand(fp, codes[x].u.two.right);
    			break;
    		case IR_PARAM:
    			fprintf(fp, "PARAM ");
    			Print_Operand(fp, codes[x].u.one.op);
    			break;
    		case IR_READ:
    			fprintf(fp, "READ ");
    			Print_Operand(fp, codes[x].u.one.op);
    			break;    			
    		case IR_WRITE:
    			fprintf(fp, "WRITE ");
    			Print_Operand(fp, codes[x].u.one.op);
    			break;    			
    			      			 			
    	}
    	fprintf(fp, "\n");
        x++;
    }
}
Operand new_label()
{
	char str[100];
	sprintf(str, "label%d", label_num);
	Operand op = (Operand)malloc(sizeof(struct Operand_));
	op->kind = OP_LABEL;
	strcpy(op->u.name, str);
	label_num++;
	return op;
}
Operand new_temp()
{
	Operand op = (Operand)malloc(sizeof(struct Operand_));
	op->kind = OP_VARIABLE;
	char str[100];
	sprintf(str, "t%d", temp_var_num);
	strcpy(op->u.name, str);
	op->u.is_addr = 0;
	temp_var_num++;
	return op;
}
int get_size(Type type)
{
	if(type == NULL)
		return 0;
	else if(type->kind == BASIC)
		return 4;
	else if(type->kind == ARRAY)
		return type->u.array.size * get_size(type->u.array.elem);
	else if(type->kind == STRUCTURE)
	{
		int size = 0;
		FieldList temp = type->u.structure_.structure;
		while(temp != NULL)
		{
			size = size + get_size(temp->type);
			temp = temp->next;
		}
		return size;
	}
	return 0;
}
void Translate_Exp(struct Token* node, Operand dst)
{
    /*	Exp -> 
		//基本表达式
		  Exp ASSIGNOP Exp
		| ID
		| INT	
		| Exp PLUS Exp
		| Exp MINUS Exp
		| Exp STAR Exp
		| Exp DIV Exp	
		| MINUS Exp
		//条件表达式
		| Exp AND Exp
		| Exp OR Exp
		| Exp RELOP Exp
		| NOT Exp
		//函数调用
		| ID LP Args RP
		| ID LP RP
		//数组
		| Exp LB Exp RB
		| LP Exp RP			
	*/
#ifdef debug
	printf("Exp\n");
#endif
	if(Translate_Is_Correct==0)
		return;
	//Exp -> LP Exp RP
	if(strcmp(node->firstChild->token_type,"LP") == 0)
		Translate_Exp(node->firstChild->nextSibling, dst);
    if(strcmp(node->firstChild->token_type,"INT")==0)
    {
		temp_var_num--;
		dst->kind = OP_CONSTANT;
		dst->u.value = atoi(node->firstChild->token_text);
    }
	//Exp -> ID
    else if(strcmp(node->firstChild->token_type,"ID")==0 && node->firstChild->nextSibling == NULL)
    {
		Item item = find_items_in_global_table(node->firstChild->token_text);
		temp_var_num--;
		dst->kind = OP_VARIABLE;
		strcpy(dst->u.name, node->firstChild->token_text);	
    }
	//Exp -> Exp ASSIGNOP Exp
	else if(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "ASSIGNOP")==0)
	{
		Operand t1 = new_temp();
		Translate_Exp(node->firstChild, t1);
		Operand t2 = new_temp();
		Translate_Exp(node->firstChild->nextSibling->nextSibling, t2);
		create_two_intercode(t1, t2, IR_ASSIGN);
	}
	//Exp -> Exp PLUS Exp
	else if(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "PLUS")==0)
	{
		Operand t1 = new_temp();
		Translate_Exp(node->firstChild,t1);
		Operand t2 = new_temp();
		Translate_Exp(node->firstChild->nextSibling->nextSibling,t2);
		create_three_intercode(dst, t1, t2, IR_ADD);
	}
	//Exp -> Exp MINUS Exp
	else if(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "MINUS")==0)
	{
		Operand t1 = new_temp();
		Translate_Exp(node->firstChild,t1);
		Operand t2 = new_temp();
		Translate_Exp(node->firstChild->nextSibling->nextSibling,t2);
		create_three_intercode(dst, t1, t2, IR_SUB);
	}
	//Exp -> Exp STAR Exp
	else if(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "STAR")==0)
	{
		Operand t1 = new_temp();
		Translate_Exp(node->firstChild,t1);
		Operand t2 = new_temp();
		Translate_Exp(node->firstChild->nextSibling->nextSibling,t2);
		create_three_intercode(dst, t1, t2, IR_MUL);
	}
	//Exp -> Exp DIV Exp
	else if(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "DIV")==0)
	{
		Operand t1 = new_temp();
		Translate_Exp(node->firstChild,t1);
		Operand t2 = new_temp();
		Translate_Exp(node->firstChild->nextSibling->nextSibling,t2);
		create_three_intercode(dst, t1, t2, IR_DIV);
	}
	//Exp -> MINUS Exp 
	else if(strcmp(node->firstChild->token_type, "MINUS")==0)
	{
		Operand t1 = new_temp();
		Translate_Exp(node->firstChild->nextSibling,t1);
		Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = OP_CONSTANT;
		t2->u.value = 0;
		create_three_intercode(dst, t2, t1, IR_SUB);
	}
	else if(strcmp(node->firstChild->token_type, "NOT")==0 || 
			(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "RELOP") == 0)||
			(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "AND") == 0)||
			(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "OR") == 0))
	{	
		Operand label1 = new_label();
		Operand label2 = new_label();
		Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		t1->kind = OP_CONSTANT;
		t1->u.value = 0;
		Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = OP_CONSTANT;
		t2->u.value = 1;
		create_two_intercode(dst, t1, IR_ASSIGN);
		Translate_Cond(node, label1, label2);
		create_one_intercode(label1, IR_LABEL);	
		create_two_intercode(dst, t2, IR_ASSIGN);
		create_one_intercode(label2, IR_LABEL);	
	}
	//Exp -> ID LP (Args) RP
	else if(strcmp(node->firstChild->token_type, "ID") == 0 && node->firstChild->nextSibling != NULL)
	{
		Operand op = (Operand)malloc(sizeof(struct Operand_));
		op->kind = OP_FUNCTION;
		strcpy(op->u.name, node->firstChild->token_text);
		if(strcmp(node->firstChild->nextSibling->nextSibling->token_type, "RP")==0)
		{
			if(strcmp(node->firstChild->token_text, "read")==0)
				create_one_intercode(dst, IR_READ);
			else
			{
				if(dst != NULL)
					create_two_intercode(dst, op, IR_CALL);
				else
				{
					Operand t1 = new_temp();
					create_two_intercode(t1, op, IR_CALL);
				}
			}
		}
		else
		{
			Arg_List Args = NULL;
			Args = Translate_Args(node->firstChild->nextSibling->nextSibling);
			if(strcmp(node->firstChild->token_text, "write")==0)
				create_one_intercode(Args->op, IR_WRITE);
			else
			{
				Arg_List temp = Args;
				while(temp != NULL)
				{
					Item item = find_items_in_global_table(temp->op->u.name);
					create_one_intercode(temp->op, IR_ARG);
					temp = temp->next;
				}
				if(dst != NULL)
					create_two_intercode(dst, op, IR_CALL);
				else
				{
					Operand t1 = new_temp();
					create_two_intercode(t1, op, IR_CALL);
				}
			}
		}
	}
	//Exp -> Exp1 LB Exp2 RB
	else if(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "LB")==0)
	{
		Operand t1 = new_temp();
		Translate_Exp(node->firstChild->nextSibling->nextSibling,t1);
		Operand id = new_temp();
		Translate_Exp(node->firstChild, id);

		Item item = find_items_in_global_table(id->u.name);
		//the size of the array element
		Operand constant = (Operand)malloc(sizeof(struct Operand_));
		constant->kind = OP_CONSTANT;
		constant->u.value = get_size(item->item_type->u.array.elem);
		//t2 = t1 * 4;
		Operand t2 = new_temp();
		create_three_intercode(t2, t1, constant,IR_MUL);
		//t3 = &ID;
		Operand t3 = new_temp();
		create_two_intercode(t3, id, IR_GET_ADDR);
		//dst = t3 + t2;
		create_three_intercode(dst, t3, t2, IR_ADD);
		//此时dst是当前数组元素的地址
		dst->kind = OP_ADDRESS;
		dst->u.is_addr = 0;
	}
	//Exp -> Exp1 DOT ID2 
	else if(strcmp(node->firstChild->token_type, "Exp")==0 && strcmp(node->firstChild->nextSibling->token_type, "DOT")==0)
	{
		//不会出现类型为结构体的变量
		Translate_Is_Correct = 0;
		printf("Cannot translate: Code contains variables or parameters of structure type.\n");
	}
}
Arg_List Translate_Args(struct Token* node)
{	
#ifdef debug
	printf("Translate_Args\n");
#endif
	if (Translate_Is_Correct == 0)
		return NULL;
	/*
		Args ->  Exp
				|Exp COMMA Args
	*/
	Arg_List head = (Arg_List)malloc(sizeof(struct Arg_List_));
	head->op = new_temp();
	head->next = NULL;
	Translate_Exp(node->firstChild, head->op);	
	if (head->op->kind == OP_VARIABLE)
	{
		Item item = find_items_in_global_table(head->op->u.name);
		if (item != NULL && (item->item_type->kind == ARRAY||item->item_type->kind == STRUCTURE))
		{
			Translate_Is_Correct = 0;
			printf("Cannot Translate: Code contains parameters of array type or structure type.\n");
			return;
		}
	}
	if (node->firstChild->nextSibling == NULL){}	
	else
	{
		Arg_List temp =  Translate_Args(node->firstChild->nextSibling->nextSibling);
		temp -> next = head;
		head = temp;
	}
	return head;
}

void Translate_VarDec(struct Token* node, Operand dst)
{
#ifdef debug
	printf("VarDec\n");
#endif
	if(Translate_Is_Correct == 0)
		return;
	/*VarDec → ID
			| VarDec LB INT RB
	*/
	if(strcmp(node->firstChild->token_type, "ID")==0)
	{
		Item item = find_items_in_global_table(node->firstChild->token_text);
		//用变量本名代替临时变量名
		if(item->item_type->kind == BASIC)
		{
			if(dst != NULL)
			{
				temp_var_num--;
				dst->kind = OP_VARIABLE;
				strcpy(dst->u.name, item->item_name);
			}
		}
		else if(item->item_type->kind == ARRAY)
		{
			if(item->item_type->u.array.elem->kind == ARRAY)
			{
				Translate_Is_Correct = 0;
				printf("Cannot translate: Code contains variables or parameters of multi-dimensional array type.\n");
				return;
			}
			else
			{
				Operand op = (Operand)malloc(sizeof(struct Operand_));
				op->kind = OP_VARIABLE;
				strcpy(op->u.name, item->item_name);
				codes[cur_index].kind = IR_DEC;
				codes[cur_index].u.dec.op = op;
				codes[cur_index].u.dec.size = get_size(item->item_type);
				cur_index++;
			}
		}
		else if(item->item_type->kind == STRUCTURE)
		{
			Translate_Is_Correct = 0;
			printf("Cannot translate: Code contains variables or parameters of structure type.\n");
			return;
		}
	}
	else
		Translate_VarDec(node->firstChild, dst);
}
void Translate_Dec(struct Token* node)
{
	/*Dec → VarDec
		| VarDec ASSIGNOP Exp
	*/
#ifdef debug
	printf("Dec\n");
#endif
	if(Translate_Is_Correct == 0)
		return;
	if(node->firstChild->nextSibling != NULL)
	{
		Operand t1 = new_temp();		
		Translate_VarDec(node->firstChild, t1);
		Operand t2 = new_temp();
		Translate_Exp(node->firstChild->nextSibling->nextSibling, t2);
		create_two_intercode(t1, t2, IR_ASSIGN);
	}
	else
		Translate_VarDec(node->firstChild, NULL);
}
void Translate_DecList(struct Token* node)
{
	/*DecList → Dec
			  | Dec COMMA DecList
	*/
#ifdef debug
	printf("DecList\n");
#endif
	if(Translate_Is_Correct == 0)
		return;
	Translate_Dec(node->firstChild);
	if(node->firstChild->nextSibling != NULL)
		Translate_DecList(node->firstChild->nextSibling->nextSibling);
}
void Translate_Def(struct Token* node)
{
	//Def → Specifier DecList SEMI
#ifdef debug
	printf("Def\n");
#endif
	if(Translate_Is_Correct==0)
		return;
	Translate_DecList(node->firstChild->nextSibling);
}
void Translate_DefList(struct Token* node)
{
	/*DefList → Def DefList
				|
	*/
#ifdef debug
	printf("DefList\n");
#endif
	if(Translate_Is_Correct==0)
		return;
	if(node == NULL)
		return;
	if(strcmp(node->firstChild->token_type,"Def")==0)
	{
		Translate_Def(node->firstChild);
		Translate_DefList(node->firstChild->nextSibling);
	}
}
void Translate_FunDec(struct Token* node)
{
#ifdef debug
	printf("FunDec\n");
#endif
	if(Translate_Is_Correct==0)
		return;
	Operand op = (Operand)malloc(sizeof(struct Operand_));
	op->kind = OP_FUNCTION;
	strcpy(op->u.name, node->firstChild->token_text);
	create_one_intercode(op, IR_FUNCTION);
	
	Item item = find_items_in_global_table(op->u.name);
	if(item->item_type->u.function.paramNum != 0)
	{
		FieldList temp = item->item_type->u.function.params;
		while(temp != NULL)
		{
			op = (Operand)malloc(sizeof(struct Operand_));
			op->kind = OP_VARIABLE;
			strcpy(op->u.name ,temp->name);
			create_one_intercode(op, IR_PARAM);
			temp = temp->next;
		}
	}
}
void Translate_CompSt(struct Token* node)
{
#ifdef debug	
	printf("Compst\n");
#endif
	//CompSt → LC DefList StmtList RC
	if(Translate_Is_Correct == 0)
		return;
	struct Token* CompSt = node;
	struct Token* StmtList,*DefList;
	if(strcmp(CompSt->firstChild->nextSibling->token_type,"StmtList") == 0)
		StmtList = CompSt->firstChild->nextSibling;
	else
	{
		DefList = CompSt->firstChild->nextSibling;
		StmtList = CompSt->firstChild->nextSibling->nextSibling;
		Translate_DefList(DefList);
	}
	Translate_StmtList(StmtList);
}
//According to the assumption, there is no definition of global variables and structures
void Translate_ExtDef(struct Token* node)
{
#ifdef debug
	printf("ExtDef\n");
#endif
	//ExtDef → Specifier FunDec CompSt
	if(Translate_Is_Correct==0)
		return;
	if(strcmp(node->firstChild->nextSibling->token_type,"FunDec")==0)
	{
		struct Token* FunDec = node->firstChild->nextSibling;
		Translate_FunDec(FunDec);
		struct Token* CompSt = FunDec->nextSibling;
		Translate_CompSt(CompSt);
	}
}
void Translate_ExtDefList(struct Token* node)
{
#ifdef debug
	printf("ExtDefList\n");
#endif
    if(Translate_Is_Correct == 0)
		return;
	if(node == NULL)
		return;
	if(strcmp(node->firstChild->token_type,"ExtDef")==0)
	{
		Translate_ExtDef(node->firstChild);
		Translate_ExtDefList(node->firstChild->nextSibling);
	}
}
void TraverseTree_InterCode(struct Token* node)
{
    if(Translate_Is_Correct==0)
		return;
	if(node == NULL)
		return;
	if(strcmp(node->token_type,"ExtDefList")==0)
		Translate_ExtDefList(node);
	else
    {
        TraverseTree_InterCode(node->firstChild);
	    TraverseTree_InterCode(node->nextSibling);
    }
}
void Translate_Stmt(struct Token* node)
{
#ifdef debug
	printf("Stmt\n");
#endif
	if (Translate_Is_Correct == 0)
		return;
	//Exp SEMI
	if (strcmp(node->firstChild->token_type, "Exp") == 0)
		Translate_Exp(node->firstChild, NULL);
	//CompSt
	else if (strcmp(node->firstChild->token_type, "CompSt") == 0)
		Translate_CompSt(node->firstChild);
	//RETURN Exp SEMI
	else if (strcmp(node->firstChild->token_type, "RETURN") == 0)
	{
		Operand t1 = new_temp();
		Translate_Exp(node->firstChild->nextSibling, t1);
		create_one_intercode(t1, IR_RETURN);
	}
	//IF LP Exp RP Stmt (ELSE Stmt)
	else if (strcmp(node->firstChild->token_type, "IF") == 0)
	{
		Operand label1 = new_label();
		Operand label2 = new_label();
		struct Token* Exp = node->firstChild->nextSibling->nextSibling;
		struct Token* Stmt1 = Exp->nextSibling->nextSibling;
		Translate_Cond(Exp, label1, label2);
		create_one_intercode(label1, IR_LABEL);
		Translate_Stmt(Stmt1);
		if (Stmt1->nextSibling == NULL)
			create_one_intercode(label2, IR_LABEL);
		else
		{
			Operand label3 = new_label();
			struct Token* Stmt2 = Stmt1->nextSibling->nextSibling;
			create_one_intercode(label3, IR_GOTO);
			create_one_intercode(label2, IR_LABEL);
			Translate_Stmt(Stmt2);
			create_one_intercode(label3, IR_LABEL);
		}
	}
	//WHILE LP Exp RP Stmt
	else if (strcmp(node->firstChild->token_type, "WHILE") == 0)
	{
		Operand label1 = new_label();
		Operand label2 = new_label();
		Operand label3 = new_label();
		create_one_intercode(label1, IR_LABEL);
		Translate_Cond(node->firstChild->nextSibling->nextSibling, label2, label3);
		create_one_intercode(label2, IR_LABEL);
		Translate_Stmt(node->firstChild->nextSibling->nextSibling->nextSibling->nextSibling);
		create_one_intercode(label1, IR_GOTO);
		create_one_intercode(label3, IR_LABEL);
	}
}
void Translate_StmtList(struct Token* node)
{
	/*
	StmtList → Stmt StmtList
				|
	*/
#ifdef debug
	printf("StmtList\n");
#endif
	if (Translate_Is_Correct == 0)
		return;
	if (node == NULL)
		return;
	if (strcmp(node->firstChild->token_type, "Stmt") == 0)
	{
		Translate_Stmt(node->firstChild);
		Translate_StmtList(node->firstChild->nextSibling);
	}
}
void Translate_Cond(struct Token* node, Operand label_true, Operand label_false)
{
#ifdef debug
	printf("Cond\n");
#endif
	if (Translate_Is_Correct == 0)
		return;
	if (strcmp(node->firstChild->token_type, "NOT") == 0)
		Translate_Cond(node->firstChild, label_false, label_true);
	else if (strcmp(node->firstChild->nextSibling->token_type, "RELOP") == 0)
	{
		Operand t1 = new_temp();
		Operand t2 = new_temp();
		Translate_Exp(node->firstChild, t1);
		Translate_Exp(node->firstChild->nextSibling->nextSibling, t2);
		Operand relop1 = (Operand)malloc(sizeof(struct Operand_));
		relop1->kind = OP_RELOP;
		strcpy(relop1->u.name, node->firstChild->nextSibling->token_text);
		if (t1->kind == OP_ADDRESS)
		{
			Operand t3 = new_temp();
			create_two_intercode(t3, t1, IR_INTO_ADDR_RIGHT);
			t1 = t3;
		}
		if (t2->kind == OP_ADDRESS)
		{
			Operand t3 = new_temp();
			create_two_intercode(t3, t2, IR_INTO_ADDR_RIGHT);
			t2 = t3;
		}
		codes[cur_index].kind = IR_IF_GOTO;
		codes[cur_index].u.if_goto.x = t1;
		codes[cur_index].u.if_goto.relop = relop1;
		codes[cur_index].u.if_goto.y = t2;
		codes[cur_index].u.if_goto.z = label_true;
		cur_index++;
		create_one_intercode(label_false, IR_GOTO);
	}
	else if (strcmp(node->firstChild->nextSibling->token_type, "AND") == 0)
	{
		Operand label1 = new_label();
		Translate_Cond(node->firstChild, label1, label_false);
		create_one_intercode(label1, IR_LABEL);
		Translate_Cond(node->firstChild->nextSibling->nextSibling, label_true, label_false);
	}
	else if (strcmp(node->firstChild->nextSibling->token_type, "OR") == 0)
	{
		Operand label1 = new_label();
		Translate_Cond(node->firstChild, label_true, label1);
		create_one_intercode(label1, IR_LABEL);
		Translate_Cond(node->firstChild->nextSibling->nextSibling, label_true, label_false);
	}
	else
	{
		Operand t1 = new_temp();
		Translate_Exp(node, t1);
		Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = OP_CONSTANT;
		t2->u.value = 0;
		Operand relop1 = (Operand)malloc(sizeof(struct Operand_));
		relop1->kind = OP_RELOP;
		strcpy(relop1->u.name, "!=");
		if (t1->kind == OP_ADDRESS)
		{
			Operand t3 = new_temp();
			create_two_intercode(t3, t1, IR_INTO_ADDR_RIGHT);
			t1 = t3;
		}
		if (t2->kind == OP_ADDRESS)
		{
			Operand t3 = new_temp();
			create_two_intercode(t3, t2, IR_INTO_ADDR_RIGHT);
			t2 = t3;
		}
		codes[cur_index].kind = IR_IF_GOTO;
		codes[cur_index].u.if_goto.x = t1;
		codes[cur_index].u.if_goto.relop = relop1;
		codes[cur_index].u.if_goto.y = t2;
		codes[cur_index].u.if_goto.z = label_true;
		cur_index++;
		create_one_intercode(label_false, IR_GOTO);
	}
}
//IR_FUNCTION, IR_PARAM
void create_one_intercode(Operand op, int kind)
{
	//printf("One\n");
	if(op->kind == OP_ADDRESS && op->u.is_addr==0)
	{
		Operand t1 = new_temp();
		create_two_intercode(t1, op, IR_INTO_ADDR_RIGHT);
		op = t1;
	}
	codes[cur_index].kind = kind;
	codes[cur_index].u.one.op = op;
	cur_index++;
	//printf("%d\n",cur_index);
}
//IR_ASSIGN, IR_GET_ADDR,IR_INTO_ADDR_RIGHT,IR_INTO_ADDR_LEFT, IR_CALL
void create_two_intercode(Operand op1,Operand op2, int kind)
{
	//printf("Two\n");
	if(kind == IR_ASSIGN && (op1->kind == OP_ADDRESS || op2->kind == OP_ADDRESS))
	{
		if(op1->kind == OP_ADDRESS && op2->kind != OP_ADDRESS)
			create_two_intercode(op1, op2, IR_INTO_ADDR_LEFT);
		else if(op2->kind == OP_ADDRESS && op1->kind != OP_ADDRESS)
			create_two_intercode(op1, op2, IR_INTO_ADDR_RIGHT);
		else
		{
			Operand t1 = new_temp();
			create_two_intercode(t1, op2, IR_INTO_ADDR_RIGHT);
			create_two_intercode(op1, t1, IR_INTO_ADDR_LEFT);						
		}
	}
	else if(kind == IR_GET_ADDR && op2->kind == OP_ADDRESS)
	{
		codes[cur_index].kind = IR_ASSIGN;
		codes[cur_index].u.two.left = op1;
		codes[cur_index].u.two.right = op2;
		cur_index++;
	}
	else
	{
		codes[cur_index].kind = kind;
		codes[cur_index].u.two.left = op1;
		codes[cur_index].u.two.right = op2;
		cur_index++;
	}

}
//IR_ADD,IR_SUB,IR_MUL,IR_DIV,
void create_three_intercode(Operand result, Operand op1,Operand op2, int kind)
{
	//printf("Three\n");
	if(op1->kind == OP_ADDRESS && op1->u.is_addr==0)
	{
		Operand t1 = new_temp();
		create_two_intercode(t1, op1, IR_INTO_ADDR_RIGHT);
		op1 = t1;
	}
	if(op2->kind == OP_ADDRESS && op1->u.is_addr==0)
	{
		Operand t1 = new_temp();
		create_two_intercode(t1, op2, IR_INTO_ADDR_RIGHT);
		op2 = t1;
	}
	codes[cur_index].kind = kind;
	codes[cur_index].u.three.result = result;
	codes[cur_index].u.three.op1 = op1;
	codes[cur_index].u.three.op2 = op2;
	cur_index++;
}
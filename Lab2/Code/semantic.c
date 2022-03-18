#include <string.h>
#include "semantic.h"
Item Symbol_Table = NULL;
Item Struct_Symbol_Table = NULL;//单独结构体内部的符号表,判断域是否重复
int  num_of_anonymous_structs = 0;
char anonymous_struct_name[100] = "Zmzm";
void ExtDef(struct Token* node)
{
    /*
		ExtDef -> Specifier ExtDecList SEMI
	  	ExtDef -> Specifier SEMI
	  	ExtDef -> Specifier FunDec CompSt
	*/
	struct Token* child_1 = node->firstChild; //Specifier
	Type specifier_type = Specifier(child_1);
	struct Token* child_2 = child_1->nextSibling;//ExtDecList or FunDec
	if(strcmp(child_2->token_type,"ExtDecList")==0)//全局变量定义
	{
		/*
		ExtDecList →  VarDec
					| VarDec COMMA ExtDecList
		VarDec →  ID
				| VarDec LB INT RB
		*/
		struct Token* ExtDec_List = child_2;
		Item item = NULL;
		//有逗号有变量则循环填表
		while(ExtDec_List->firstChild->nextSibling != NULL)
		{
			item = VarDec(ExtDec_List->firstChild, specifier_type);
			if(find_items_in_global_table(item->item_name))
				printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDec_List->firstChild->col, item->item_name);
		    else
		    {
		    	item->next_item = Symbol_Table;
				Symbol_Table = item;
			}
		    ExtDec_List = ExtDec_List->firstChild->nextSibling->nextSibling;
		}
		item = VarDec(ExtDec_List->firstChild, specifier_type);
		if(find_items_in_global_table(item->item_name))
			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", ExtDec_List->firstChild->col, item->item_name);
	    else
	    {
	    	item->next_item = Symbol_Table;
			Symbol_Table = item;
		}
	}	
	else if(strcmp(child_2->token_type,"FunDec")==0)//函数定义
	{
		/*
			FunDec -> ID LP VarList RP
			FunDec -> ID LP RP
		*/
		//处理函数头
		Type type = (Type)malloc(sizeof(struct Type_));
		Item item = (Item)malloc(sizeof(struct Item_));
		item->item_type = type;
		item->next_item = NULL;
		strcpy(item->item_name ,child_2->firstChild->token_text);
		
		type->kind = FUNCTION;
		type->u.function.funcType = specifier_type;
		type->u.function.paramNum = 0;
		type->u.function.params = NULL;
		if(strcmp(child_2->firstChild->nextSibling->nextSibling->token_type, "VarList")==0)
		{
			struct Token *Var_List = child_2->firstChild->nextSibling->nextSibling;
			Item item_1 = NULL;
			//有多个参数则循环填表
			int parse_num = 1;
			FieldList head = NULL ,tail = NULL;
			/*
			VarList → ParamDec COMMA VarList
					| ParamDec
			ParamDec → Specifier VarDec
			*/
			while(Var_List->firstChild->nextSibling != NULL)
			{
				parse_num++;
				struct Token *Param_Dec = Var_List->firstChild;
				Type specifier_type = Specifier(Param_Dec->firstChild);
				item_1 = VarDec(Param_Dec->firstChild->nextSibling, specifier_type);
				if(find_items_in_global_table(item_1->item_name))
					printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Param_Dec->firstChild->nextSibling->col, item->item_name);
				else 
				{
					FieldList temp = (FieldList)malloc(sizeof(struct FieldList_));
					strcpy(temp->name, item_1->item_name);
					temp->type = item_1->item_type;
					temp->next = NULL;
					if(tail == NULL)
						head = tail = temp;	
					else
					{
						tail->next = temp;
						tail = temp;
					}
					item_1->next_item = Symbol_Table;
					Symbol_Table = item_1;
				} 
				Var_List = Var_List->firstChild->nextSibling->nextSibling;
			}
			struct Token *Param_Dec = Var_List->firstChild;
			Type specifier_type = Specifier(Param_Dec->firstChild);
			item_1 = VarDec(Param_Dec->firstChild->nextSibling, specifier_type);
			if(find_items_in_global_table(item_1->item_name))
				printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", Param_Dec->firstChild->nextSibling->col, item->item_name);
			else 
			{
				FieldList temp = (FieldList)malloc(sizeof(struct FieldList_));
				strcpy(temp->name, item_1->item_name);
				temp->type = item_1->item_type;
				temp->next = NULL;	
				if(tail == NULL)
					head = tail = temp;	
				else
				{
					tail->next = temp;
					tail = temp;
				}
				item_1->next_item = Symbol_Table;
				Symbol_Table = item_1;
			} 
			type->u.function.params = head;
			type->u.function.paramNum = parse_num;
		}
		if(find_items_in_global_table(item->item_name))
			printf("Error type 4 at Line %d: Redefined function \"%s\".\n", child_2->col, item->item_name);
        else 
        {
			item->next_item = Symbol_Table;
			Symbol_Table = item;
		} 
        //处理函数体
        struct Token* child_3 = child_2->nextSibling;          //CompSt
        CompSt(child_3, item->item_type->u.function.funcType); //传入返回值的类型
		
	}
	//结构体定义在处理Specifier时已经解决，不需要处理
}
Type Specifier(struct Token* node)
{
    Type specifier_type = (Type)malloc(sizeof(struct Type_));
    //Specifier -> Type
	if(strcmp(node->firstChild->token_type, "TYPE")==0)
    {
        specifier_type->kind = BASIC;
        if(strcmp(node->firstChild->token_text,"int")==0)
            specifier_type->u.basic = 0; //int
        else specifier_type->u.basic = 1;//float
        return specifier_type;
    }
    else
    {
    	/*	
			Specifier → StructSpecifier
    		StructSpecifier -> STRUCT OptTag LC DefList RC
    		StructSpecifier -> STRUCT Tag
            OptTag -> ID | kong
    		Tag -> ID
    	*/
    	specifier_type->kind = STRUCTURE;
    	struct Token* struct_specifier = node->firstChild;
    	if(strcmp(struct_specifier->firstChild->nextSibling->token_type,"Tag")==0)
    	{
			//StructSpecifier -> STRUCT Tag
    		Item item = find_items_in_global_table(struct_specifier->firstChild->nextSibling->firstChild->token_text);//ID的text
    		if(item == NULL || item->item_type->kind != STRUCTURE)
    		{
    			printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", node->col, struct_specifier->firstChild->nextSibling->firstChild->token_text);
				return NULL;
    		}
    		else return item->item_type;
    	}
    	else
    	{
			//StructSpecifier -> STRUCT OptTag LC DefList RC
    		struct Token* def_list = NULL;
    		Item item = (Item)malloc(sizeof(struct Item_));
			item->next_item = NULL;
			item->item_type = specifier_type;
			item->item_name[0] = '\0';
    		specifier_type->u.structure_.structure = NULL;
    		if(strcmp(struct_specifier->firstChild->nextSibling->token_type, "OptTag")==0)
    		{
    			strcpy(item->item_name, struct_specifier->firstChild->nextSibling->firstChild->token_text);
				strcpy(specifier_type->u.structure_.name, struct_specifier->firstChild->nextSibling->firstChild->token_text);
    			if(find_items_in_global_table(item->item_name) != NULL)
    			{
    				printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", struct_specifier->firstChild->nextSibling->firstChild->col ,\
					                                                              struct_specifier->firstChild->nextSibling->firstChild->token_text);
    				return NULL;
    			}
    			def_list = struct_specifier->firstChild->nextSibling->nextSibling->nextSibling;
    		}
    		else
    		{
				char s[100];
				int_to_str(num_of_anonymous_structs++, s);
				strcpy(item->item_name,anonymous_struct_name);
				strcat(item->item_name,s);
				strcpy(item->item_type->u.structure_.name, item->item_name);
				def_list = struct_specifier->firstChild->nextSibling->nextSibling;
    		}
			//an empty struct
			if(strcmp(def_list->token_type,"DefList") != 0)
				specifier_type->u.structure_.structure = NULL;
			else
			{
				//Delete the old table
				while(Struct_Symbol_Table)
				{
					Item temp = Struct_Symbol_Table;
					Struct_Symbol_Table = Struct_Symbol_Table->next_item;
					free(temp);
				}
				//start constructing a linked list for the struct
				FieldList head = NULL;
				FieldList tail = NULL;
				while(def_list != NULL)
				{
					FieldList ret = Def_in_structure(def_list->firstChild);
					if(tail == NULL)
						head = ret, tail = ret;
					else
					{
						while(tail->next) tail = tail->next;
						tail->next = ret;
						tail = tail->next;
					}
					def_list = def_list->firstChild->nextSibling;
				}
				specifier_type->u.structure_.structure = head;
			}
			//Insert the item into the symbol table
			item->next_item = Symbol_Table;
			Symbol_Table = item;
			return specifier_type;
    	}
    }

}
Item VarDec(struct Token* node, Type type)
{
	/*
	VarDec → ID
			| VarDec LB INT RB
	*/
	Item item = (Item)malloc(sizeof(struct Item_));
	item->next_item = NULL;
	item->item_type = (Type)malloc(sizeof(struct Type_));
	item->item_name[0] = '\0';
	//Get the Name
	struct Token* child = node->firstChild;
	while(child->firstChild != NULL)
		child = child->firstChild;
	strcpy(item->item_name, child->token_text);
	child = node->firstChild;
	//Basic
	if(strcmp(child->token_type, "ID")==0)
	{
		item->item_type = type;
		return item;
	}
	//Array
	Type temp[100] = {NULL};
	temp[0] = type;
	int i = 0;
	while(child->firstChild != NULL)
	{
		Type var = (Type)malloc(sizeof(struct Type_));
		var->kind = ARRAY;
		var->u.array.size = atoi(child->nextSibling->nextSibling->token_text);
		var->u.array.elem = temp[i], i++;
		temp[i] = var;
		child = child->firstChild;
	}
	item->item_type = temp[i];
	return item;		
}

Item Dec(struct Token* node, Type type)
{
	/*
		Dec → VarDec
			| VarDec ASSIGNOP Exp
	*/
	struct Token* child_1 = node->firstChild;//VarDec
	Item item = VarDec(child_1, type);
	if(child_1->nextSibling != NULL)
	{
		struct Node* child_3 = child_1->nextSibling->nextSibling;//Exp
		Type exp_type = Exp(child_3);
		//判断Exp和VarDec类型相等
		if(check_type(item->item_type, exp_type)==0)
			printf("Error type 5 at Line %d: Type mismatched for assignment.\n", node->col);
	}
	return item;
}
Item Dec_in_structure(struct Token* node, Type type)
{
	/*
	Dec → VarDec
		| VarDec ASSIGNOP Exp
	*/
	struct Token* child = node->firstChild;//VarDec
	Item item = VarDec(child, type);
	if(child->nextSibling != NULL)
		//结构体变量不允许初始化
		printf("Error type 15 at Line %d: Illegal Initialization.\n", node->col);
	return item;
}
void Def(struct Token* node)
{
	/*
		Def -> Specifier DecList SEMI
	*/
	Type specifier_type = Specifier(node->firstChild);
	struct Token* Dec_List = node->firstChild->nextSibling;
	/*
		DecList -> Dec
		DecList -> Dec COMMA DecList
	*/
	Item item = NULL;
	//Has a COMMA, 循环填表
	while(Dec_List->firstChild->nextSibling != NULL)
	{
		item = Dec(Dec_List->firstChild,specifier_type);
		if(find_items_in_global_table(item->item_name))
			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",node->col, item->item_name);
		else
		{
			item->next_item = Symbol_Table;
			Symbol_Table = item;
		}    
		Dec_List = Dec_List->firstChild->nextSibling->nextSibling;
	}
	item = Dec(Dec_List->firstChild,specifier_type);
	if(find_items_in_global_table(item->item_name))
		printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",node->col, item->item_name);
	else
	{
		item->next_item = Symbol_Table;
		Symbol_Table = item;
	}
}
FieldList Def_in_structure(struct Token* node)
{
	/*
		Def -> Specifier DecList SEMI
	*/
	Type specifier_type = Specifier(node->firstChild);
	struct Token* Dec_List = node->firstChild->nextSibling;
	/*
		DecList -> Dec
		DecList -> Dec COMMA DecList
	*/
	Item item = NULL;
	FieldList head = NULL,tail = NULL;
	while(Dec_List->firstChild->nextSibling != NULL)//有逗号
	{
		item = Dec_in_structure(Dec_List->firstChild,specifier_type);
		if(find_items_in_struct_table(item))
			printf("Error type 15 at Line %d: Redefined field \"%s\".\n",node->col, item->item_name);
		else
		{
			item->next_item = Struct_Symbol_Table;
			Struct_Symbol_Table = item;
		}
		FieldList temp = (FieldList)malloc(sizeof(struct FieldList_));
		strcpy(temp->name, item->item_name);
		temp->type = item->item_type;
		temp->next = NULL;
		if(tail == NULL)
			head = tail = temp;
		else
		{
			tail->next = temp;
			tail = tail->next;
		}
		Dec_List = Dec_List->firstChild->nextSibling->nextSibling;
	}
	//deal with the last Dec
	item = Dec_in_structure(Dec_List->firstChild,specifier_type);
	if(find_items_in_struct_table(item))
		printf("Error type 15 at Line %d: Redefined field \"%s\".\n",node->col, item->item_name);
	else
	{
		item->next_item = Struct_Symbol_Table;
		Struct_Symbol_Table = item;
	}
	FieldList temp = (FieldList)malloc(sizeof(struct FieldList_));
	strcpy(temp->name, item->item_name);
	temp->type = item->item_type;
	temp->next = NULL;
	if(tail == NULL)
		head = tail = temp;
	else
	{
		tail->next = temp;
		tail = tail->next;
	}
	return head;
}

Type Exp(struct Token* node)
{
	/*	Exp -> Exp ASSIGNOP Exp		
			| Exp AND Exp			
			| Exp OR Exp			
			| Exp RELOP Exp			
			| Exp PLUS Exp			
			| Exp MINUS Exp			
			| Exp STAR Exp			
			| Exp DIV Exp			
			| LP Exp RP				
			| MINUS Exp				
			| NOT Exp					
			| ID LP Args RP			
			| ID LP RP					
			| Exp LB Exp RB			
			| Exp DOT ID
			| ID					
			| INT						
			| FLOAT					
	*/
	Type int_type = (Type)malloc(sizeof(struct Type_));
	int_type->kind = BASIC;
	int_type->u.basic = 0;
	Type float_type = (Type)malloc(sizeof(struct Type_));
	float_type->kind = BASIC;
	float_type->u.basic = 1;
	if(strcmp(node->firstChild->token_type, "Exp")==0)
	{
		Type type_1 = Exp(node->firstChild);
		//Exp ASSIGNOP Exp
		if(strcmp(node->firstChild->nextSibling->token_type, "ASSIGNOP")==0)
		{
			Type type_3 = Exp(node->firstChild->nextSibling->nextSibling);
			//左值三种情况：ID、Exp LB Exp RB、Exp DOT ID
			int i = 0;
			struct Token* temp = node->firstChild->firstChild;
			while(temp)
			{
				i++;
				temp = temp->nextSibling;
			}
			if(i == 1  && strcmp(node->firstChild->firstChild->token_type,"ID") != 0)// 1个子节点
			{
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", node->col);
				return NULL;	
			}
			else if(i == 3)//3个子节点
			{
				if(!(strcmp(node->firstChild->firstChild->token_type,"Exp")==0 && 
					strcmp(node->firstChild->firstChild->nextSibling->token_type,"DOT")==0 && 
					strcmp(node->firstChild->firstChild->nextSibling->nextSibling->token_type,"ID")==0))
				{
					printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", node->col);
					return NULL;
				}
			}
			else if(i == 4)//4个子节点
			{
				if(!(strcmp(node->firstChild->firstChild->token_type,"Exp")==0 && 
					strcmp(node->firstChild->firstChild->nextSibling->token_type,"LB")==0 && 
					strcmp(node->firstChild->firstChild->nextSibling->nextSibling->token_type,"Exp")==0 && 
					strcmp(node->firstChild->firstChild->nextSibling->nextSibling->nextSibling->token_type,"RB")==0))
				{
					printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", node->col);
					return NULL;				
				}
			}
			if(!check_type(type_1, type_3))
			{
				printf("Error type 5 at Line %d: Type mismatched for assignment.\n", node->col);
				return NULL;
			}
			else
				return type_1;
		}
		/*
		Exp AND Exp			
		Exp OR Exp			
		Exp RELOP Exp
		仅有int型变量才能进行逻辑运算或者作为if和while语句的条件；
		*/
		else if(strcmp(node->firstChild->nextSibling->token_type, "AND")==0 ||
			strcmp(node->firstChild->nextSibling->token_type, "OR")==0 ||
			strcmp(node->firstChild->nextSibling->token_type, "RELOP")==0)
		{
			Type type_3 = Exp(node->firstChild->nextSibling->nextSibling);
			if((!check_type(int_type, type_1))||(!check_type(int_type, type_3))||(!check_type(type_1, type_3)))
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", node->col);
				return NULL;
			}
			else return int_type;
		}
		/*
		Exp PLUS Exp			
		Exp MINUS Exp			
		Exp STAR Exp			
		Exp DIV Exp
		仅有int型和float型变量才能参与算术运算。
		整型（int）变量不能与浮点型（float）变量相互赋值或者相互运算.
		*/
		else if(strcmp(node->firstChild->nextSibling->token_type, "PLUS")==0 ||
			strcmp(node->firstChild->nextSibling->token_type, "MINUS")==0 ||
			strcmp(node->firstChild->nextSibling->token_type, "STAR")==0 ||
			strcmp(node->firstChild->nextSibling->token_type, "DIV")==0)
		{
			Type type_3 = Exp(node->firstChild->nextSibling->nextSibling);
			if(check_type(type_1, type_3) == 0 ||
				(!check_type(int_type, type_1))&&(!check_type(float_type, type_1))||
				(!check_type(int_type, type_3))&&(!check_type(float_type, type_3)))
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", node->col);
				return NULL;
			}
			else return type_1;
		}
		//Exp LB Exp RB	
		else if(strcmp(node->firstChild->nextSibling->token_type, "LB")==0)
		{
			
			struct Token* child_1 = node->firstChild;
			struct Token* child_3 = child_1->nextSibling->nextSibling;
			Type child_1_type = Exp(child_1);
			Type child_3_type = Exp(child_3);
			if(child_1_type == NULL)
				return NULL;
			if(child_1_type->kind != ARRAY)
			{
			    printf("Error type 10 at Line %d: Illegal use of \"[]\", the expression is not an array.\n", node->col);
				return NULL;
			}
			if(child_3_type->kind != BASIC || child_3_type->u.basic == 1)
			{
				printf("Error type 12 at Line %d: Array index is not an integer.\n",node->col);
				return child_1_type->u.array.elem;
			}
			return child_1_type->u.array.elem;
		}
		//Exp DOT ID
		else if(strcmp(node->firstChild->nextSibling->token_type, "DOT")==0)
		{
			Type structure_type = type_1;
			if(structure_type == NULL)
				return NULL;
			if(structure_type->kind != STRUCTURE)
			{
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", node->col);
				return NULL;
			}
			struct Token* id_token = node->firstChild->nextSibling->nextSibling;
			char *id_token_type = id_token->token_text;
			FieldList temp = structure_type->u.structure_.structure;
		    while(temp != NULL)
		    {
		        if(strcmp(temp->name, id_token_type)==0)
		            return temp->type;
		        temp = temp->next;
		    }
			printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",node->col,id_token_type);
        	return NULL;
		}
	}
	else
	{
		/*
		LP Exp RP				
		MINUS Exp				
		NOT Exp
		*/
		if(strcmp(node->firstChild->token_type, "LP")==0 || strcmp(node->firstChild->token_type, "MINUS")==0 ||strcmp(node->firstChild->token_type, "NOT")==0)
			return Exp(node->firstChild->nextSibling);
		//ID
		if(strcmp(node->firstChild->token_type, "ID")==0 && node->firstChild->nextSibling == NULL)
		{
			Item item = find_items_in_global_table(node->firstChild->token_text);
			if(item == NULL)
			{
			    printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",node->col,node->firstChild->token_text);
				return NULL;
			}
			else
				return item->item_type;
		}
		if(strcmp(node->firstChild->token_type, "INT")==0)
			return int_type;
		if(strcmp(node->firstChild->token_type, "FLOAT")==0)
			return float_type;
		/*
					
		ID LP RP
		*/
		if(strcmp(node->firstChild->token_type, "ID")==0 && node->firstChild->nextSibling != NULL)
		{
			//函数调用
			struct Token* func_token = node->firstChild;
			Item item = find_items_in_global_table(func_token->token_text);
			if(item == NULL)
			{
			    printf("Error type 2 at Line %d: Undefined function \"%s\".\n",func_token->col, func_token->token_text);
				return NULL;
			}
			if(item->item_type->kind != FUNCTION)
			{
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n",func_token->col, func_token->token_text);
				return NULL;
			}
			//获取参数列表
			FieldList right_params_type = item->item_type->u.function.params;
			int right_num_of_params = item->item_type->u.function.paramNum;
			if(strcmp(func_token->nextSibling->nextSibling->token_type, "Args") == 0) //调用时提供了参数
			{
				/*
				Exp  → ID LP Args RP
				Args → Exp COMMA Args
					| Exp
				*/
				struct Token* args_token = func_token->nextSibling->nextSibling;
				int param_num = 1;
				while(args_token->firstChild->nextSibling != NULL)
				{
					param_num++;
					Type args_type = Exp(args_token->firstChild);
					if(right_params_type != NULL && !check_type(args_type, right_params_type->type))
					{
						printf("Error type 9 at Line %d: Type mismatched for parameters when calling.\n", node->col);	
						return NULL;
					}		
					args_token = args_token->firstChild->nextSibling->nextSibling;
					if(right_params_type != NULL)
						right_params_type = right_params_type->next;
				}
				Type args_type = Exp(args_token->firstChild);
				if(right_params_type != NULL && !check_type(args_type, right_params_type->type))
				{
					printf("Error type 9 at Line %d: Type mismatched for parameters when calling.\n", node->col);	
						return NULL;
				}	
				if(param_num != right_num_of_params)
				{
					printf("Error type 9 at Line %d: The number of parameters is wrong. %d parameter(s) are needed but %d parameter(s) are provided\n", node->col, right_num_of_params, param_num);
					return NULL;
				}		
			}
			else //调用时没有提供参数
			{
				//Exp  → ID LP Args RP
				if(right_num_of_params != 0)
					printf("Error type 9 at Line %d: The number of parameters is wrong. No parameters are provided when calling, but actually need %d parameter(s).\n",node->col, right_num_of_params);
				return NULL;
			}
			return item->item_type->u.function.funcType;
		}
	}	
	return NULL;
}
void Stmt(struct Token* node, Type return_type)
{
	/*
		Stmt → Exp SEMI
			| CompSt
			| RETURN Exp SEMI
			| IF LP Exp RP Stmt
			| IF LP Exp RP Stmt ELSE Stmt
			| WHILE LP Exp RP Stmt
	*/

	if(strcmp(node->firstChild->token_type,"Exp")==0)
		Exp(node->firstChild);
	else if(strcmp(node->firstChild->token_type,"CompSt")==0)
		CompSt(node->firstChild, return_type);
	else if(strcmp(node->firstChild->token_type,"RETURN")==0)
	{
		Type ret = Exp(node->firstChild->nextSibling);
		if(!check_type(return_type, ret))
			printf("Error type 8 at Line %d: Type mismatched for return.\n", node->col);
	}
	else if(strcmp(node->firstChild->token_type,"IF")==0)
	{
		struct Token* exp_node = node->firstChild->nextSibling->nextSibling;
		struct Token* stmt_node = exp_node->nextSibling->nextSibling;
		Exp(exp_node);
		Stmt(stmt_node, return_type);	
		if(stmt_node->nextSibling != NULL)
			Stmt(stmt_node->nextSibling->nextSibling, return_type);
	}
	else if(strcmp(node->firstChild->token_type,"WHILE")==0)
	{
		struct Token* exp_node = node->firstChild->nextSibling->nextSibling;
		struct Token* stmt_node = exp_node->nextSibling->nextSibling;
		Exp(exp_node);
		Stmt(stmt_node, return_type);
	}
}
void CompSt(struct Token* node, Type return_type)
{
	/*
		CompSt -> LC DefList StmtList RC
	*/
	struct Token* child_1 = node->firstChild;//LC
	struct Token* child_2 = child_1->nextSibling;//DefList
	struct Token* child_3 = child_2->nextSibling;//StmtList
	if(strcmp(child_2->token_type,"DefList")==0) //DefList不为空
	{
		if(strcmp(child_3->token_type,"StmtList")!=0) //StmtList为空
			child_3 = NULL;
	}
	else
	{
		child_3 = child_2;
		child_2 = NULL;
		if(strcmp(child_3->token_type,"StmtList")!=0) //StmtList为空
			child_3 = NULL;
	}

	/*
		DefList -> Def DefList
		DefList -> ε
	*/	
	while(child_2 != NULL)
	{
		Def(child_2->firstChild);
		child_2 = child_2->firstChild->nextSibling;
	}
	/*
		StmtList -> Stmt StmtList
		StmtList -> ε
	*/
	while(child_3 != NULL)
	{
		Stmt(child_3->firstChild, return_type);
		child_3 = child_3->firstChild->nextSibling;
	}
}

int check_type(Type type1, Type type2)
{
	if(type1 == NULL && type2 == NULL)
		return 1;
	else if(type1 == NULL || type2 == NULL)
		return 0;
	if(type1->kind != type2->kind)
		return 0;
	else
	{
		if(type1->kind == BASIC)
			if(type1->u.basic != type2->u.basic)
				return 0;
			else
				return 1;
		if(type1->kind == ARRAY)
		{
			if(check_type(type1->u.array.elem,type2->u.array.elem)==0)
				return 0;
			else return 1;
		}
		if(type1->kind == STRUCTURE)//名等价
		{
			if(strcmp(type1->u.structure_.name, type2->u.structure_.name)==0)
				return 1;
			else return 0;
		}
	}
}
int find_items_in_struct_table(Item item)
{
	Item cur = Struct_Symbol_Table;
	while(cur)
	{
		if(strcmp(cur->item_name,item->item_name)==0)
			return 1;
		cur = cur->next_item;
	}
	return 0;
}
Item find_items_in_global_table(char* s)
{
    Item cur = Symbol_Table;
    while(cur != NULL)
    {
        if(strcmp(cur->item_name, s)==0)
            break;
        cur = cur->next_item;
    }
    return cur;
}
void int_to_str(int n, char* s)
{
	if(n = 0)
	{
		s[0] = '0',s[1] = '\0';
		return;
	}
	int i = 0, temp = n;
	for(; temp != 0; i++)
		temp /= 10; 
	s[i] = '\0';
	for(i = i - 1; i >= 0; i--)
	{
		temp = n % 10;
		n /= 10;
		s[i] = '0' + temp;
	}
	return;
}
void TraverseTree(struct Token* root)
{
    if(root == NULL)
		return;
	if(strcmp(root->token_type,"ExtDef")==0)
		ExtDef(root);
	TraverseTree(root->firstChild);
	TraverseTree(root->nextSibling);
}
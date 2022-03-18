#ifndef Semantic_H
#define Semantic_H

#include "token.h"
typedef struct Item_* Item;
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
struct Type_
{
	enum { BASIC, ARRAY, STRUCTURE, FUNCTION} kind;
	union
	{
		int basic; //基本类型
		struct { Type elem; int size; } array; //数组类型信息包括元素类型与数组大小构成
		struct {
			char name[100];
            FieldList structure;
        } structure_; //结构体类型信息是一个链表
		struct {
			FieldList params;//函数的参数
			Type funcType;   //返回值类型
			int paramNum;    //参数数量
		}function;
	} u;
};
struct FieldList_
{
	char name[100]; //域的名字
	Type type;  //域的类型
	FieldList next; //下一个域
};
struct Item_
{
    char item_name[100];
    Type item_type;
    Item next_item;
};
/*Handle the definition of top-level functions, structures, and global variables.
(处理最高层函数、结构体、全局变量的定义)*/
void ExtDef(struct Token* node);
/*Parse the Specifiers and return its type (basic or struct).
(解析类型描述符, 返回其类型(basic or struct))*/
Type Specifier(struct Token* node); 
/*Parse the definition of a single variable (basic or array) and return its corresponding item in the symbol table.
(解析对单个变量的定义(basic or array), 返回其在符号表中对应的项)*/
Item VarDec(struct Token* node, Type type);

/*每个Def就是一行变量定义，它包括一个类型描述符Specifier以及一个DecList，例如int a, b, c;
由于DecList中的每个Dec又可以变成 VarDec|VarDec ASSIGNOP Exp，这允许我们对局部变量在定义时进行初始化，例如int a = 5;*/

/*Analyze the definition of a single variable in the function body, and return its corresponding item in the global symbol table.
(解析函数体内单个变量的定义，返回其在全局符号表中对应的项)*/
Item Dec(struct Token* node, Type type);
/*Analyze the definition of a single variable in the structure. Assignment is not allowed here, and return its corresponding item in the structure symbol table.
(解析结构体内单个变量的定义,此处不允许赋值,返回其在结构体符号表中对应的项)*/
Item Dec_in_structure(struct Token* node, Type type);

/*Analyze the definition of a row of variables in the function body and add it to the global symbol table.
(解析函数体内一行变量的定义，并且将其加入全局符号表)*/
void Def(struct Token* node);
/*Analyze the definition of a row of variables in the structure, and return a linked list composed of FieldList, including all variables defined in this row.
(解析结构体内一行变量的定义, 返回一个由FieldList组成的链表, 包括了在这一行中定义的所有变量)*/
FieldList Def_in_structure(struct Token* node);

/*Processing expression, return the type of expression.
(处理表达式，返回表达式的类型)*/
Type Exp(struct Token* node);
/*Process a single statement in the function body.
(处理函数体内的单个语句)*/
void Stmt(struct Token* node, Type return_type);
/*Handle the function body, return _type is used to check the return value type.
(处理函数体，return _type 用于检查返回值类型)*/
void CompSt(struct Token* node, Type return_type);

void init_table();
/*Check if the two types are the same, return 1 if they are the same, otherwise return 0.
(检查两个类型是否相同，相同返回1，否则返回0)*/
int check_type(Type type1, Type type2);
/*Look up the symbol table of the structure to determine whether there is a redefinition of the domain.
(查找结构体的符号表，用于判断是否有域的重定义)*/
int find_items_in_struct_table(Item item);
/*Look up the global symbol table, return the table entry if found, otherwise return NULL.
(查找全局符号表，如果找到返回表项，否则返回NULL)*/
Item find_items_in_global_table(char* s); 
/*Convert the number into a string, used to generate the name of the anonymous structure.
(把数字转成字符串，用于生成匿名结构体的名字)*/
void int_to_str(int n, char* s);
/*Traverse the syntax analysis tree and perform semantic analysis.
(遍历语法分析树，进行语义分析)*/
void TraverseTree(struct Token* root);

#endif
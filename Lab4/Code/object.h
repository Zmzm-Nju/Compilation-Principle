#ifndef _OBJECT_H_
#define _OBJECT_H_
#include <stdio.h>
#include <string.h>
#include "semantic.h"
#include "intermediate.h"
typedef struct VaribleDescriptor_* VaribleDescriptor;
typedef struct RegisterDescriptor_* RegisterDescriptor;
struct RegisterDescriptor_
{
	int free;//寄存器是否空闲
	char name[20];
};
struct VaribleDescriptor_
{
	int reg_num;//变量所在的寄存器编号
	Operand op;
	VaribleDescriptor next;
};
//为变量分配寄存器
int allocate(Operand op);
//如果该变量已经在寄存器中，返回其编号；否则为其分配新的寄存器,并返回编号
int ensure(FILE* fp, Operand op);
//调用者在调用函数之前,保存a0-a3,t0-t10
void push_regs_saved_by_caller(FILE* fp);
//调用者在调用函数之后,恢复a0-a3,t0-t10
void pop_regs_saved_by_caller(FILE* fp);
//被调用者在进入函数时,保存s0-s7
void push_regs_saved_by_callee(FILE* fp);
//被调用者在return之前,恢复s0-s7
void pop_regs_saved_by_callee(FILE* fp);
//初始化目标代码文件
void Init_Objectcode(FILE* fp);
void Generate_Objectcode_File(FILE* fp);
void del_varible_descriptor(VaribleDescriptor p);
void add_varible_descriptor(int reg_num, Operand op);

#endif
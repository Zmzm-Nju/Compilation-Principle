#include "object.h"
extern int cur_index;
extern struct InterCode codes[MAX_LINE];
//记录最近使用的寄存器，在寄存器满时不将其淘汰
int recently_used_reg = -1;
//当前函数的寄存器描述符表
struct RegisterDescriptor_ Reg_List[32];
//当前函数的变量描述符表
VaribleDescriptor cur_varible_list = NULL;
int allocate(Operand op)
{
	for(int i = 8; i < 26; i++)
	{
		if(Reg_List[i].free)
		{
			Reg_List[i].free=0;
			add_varible_descriptor(i, op);
			return i;
		}
	}
	VaribleDescriptor p = cur_varible_list;
    //删除常量
	while(p != NULL)
	{
		if(p->op->kind == OP_CONSTANT && p->reg_num != recently_used_reg)
		{
			int num = p->reg_num;
			del_varible_descriptor(p);
			add_varible_descriptor(num, op);
			return num;
		}
		p = p->next;
	}
	p = cur_varible_list;
    //删除临时变量
	while(p != NULL)
	{
		if(p->op->kind != OP_CONSTANT)
		{
			if(p->op->u.name[0]=='t' && p->reg_num != recently_used_reg)
			{
				int num = p->reg_num;
			    del_varible_descriptor(p);
			    add_varible_descriptor(num, op);
			    return num;
			}
		}
		p=p->next;
	}
	//删除非最近使用
	while(p != NULL)
	{
		if(p->reg_num != recently_used_reg)
		{
				int num = p->reg_num;
			    del_varible_descriptor(p);
			    add_varible_descriptor(num, op);
			    return num;
		}
		p=p->next;
	}
}
int ensure(FILE* fp, Operand op)
{
	if(op->kind != OP_CONSTANT)
	{
		VaribleDescriptor p = cur_varible_list;
		while(p != NULL)
		{
			if(p->op->kind != OP_CONSTANT && strcmp(p->op->u.name, op->u.name)==0)
            {
                recently_used_reg = p->reg_num;
				return p->reg_num;
            }
			p = p->next;
		}
		int reg_num = allocate(op);
        recently_used_reg = reg_num;
		return reg_num;
	}
    //constant
	else
	{
		int reg_num = allocate(op);
		fprintf(fp, "  li %s, %d\n", Reg_List[reg_num].name, op->u.value);
        recently_used_reg = reg_num;
		return reg_num;
	}
}
void del_varible_descriptor(VaribleDescriptor p)
{
    VaribleDescriptor head = cur_varible_list;
    if(head == p)
        cur_varible_list = cur_varible_list->next;
    else{
        while(head != NULL && head->next != p)
            head = head->next;
        VaribleDescriptor temp = head->next;
        head->next = temp->next;
        free(temp);
    }
}
void add_varible_descriptor(int reg_num, Operand op)
{
    VaribleDescriptor p = (VaribleDescriptor)malloc(sizeof(struct VaribleDescriptor_));
    p->reg_num = reg_num;
    p->op = op;
    p->next = NULL;
    if(cur_varible_list == NULL)
        cur_varible_list = p;
    else{
        p->next = cur_varible_list;
        cur_varible_list = p;
    }
}
void push_regs_saved_by_caller(FILE* fp)
{
	fprintf(fp, "  addi $sp, $sp, -56\n");
	for(int i = 4; i <= 15; i++)
		fprintf(fp, "  sw %s, %d($sp)\n", Reg_List[i].name, (i-4)*4);
	fprintf(fp, "  sw %s, %d($sp)\n", Reg_List[24].name, 48);
    fprintf(fp, "  sw %s, %d($sp)\n", Reg_List[25].name, 52);
}
void pop_regs_saved_by_caller(FILE* fp)
{
	for(int i = 4; i <= 15; i++)
		fprintf(fp, "  lw %s, %d($sp)\n", Reg_List[i].name, (i-4)*4);
	fprintf(fp, "  lw %s, %d($sp)\n", Reg_List[24].name, 48);
    fprintf(fp, "  lw %s, %d($sp)\n", Reg_List[25].name, 52);
	fprintf(fp, "  addi $sp, $sp, 56\n");
}
void push_regs_saved_by_callee(FILE* fp)
{
	fprintf(fp, "  addi $sp, $sp, -32\n");
	for(int i = 16; i <= 23; i++)
		fprintf(fp, "  sw %s, %d($sp)\n", Reg_List[i].name, (i-16)*4);
}
void pop_regs_saved_by_callee(FILE* fp)
{
	for(int i = 16; i <= 23; i++)
		fprintf(fp, "  lw %s, %d($sp)\n", Reg_List[i].name, (i-16)*4);
	fprintf(fp, "  addi $sp, $sp, 32\n");
}
void Init_Objectcode(FILE* fp)
{
	for(int i = 0; i < 32; i++)
		Reg_List[i].free = 1;
	strcpy(Reg_List[0].name, "$zero");
    strcpy(Reg_List[1].name, "$at");
    strcpy(Reg_List[2].name, "$v0");
    strcpy(Reg_List[3].name, "$v1");
    for(int i = 4; i <= 7; i++)
		sprintf(Reg_List[i].name, "$a%d",i - 4);
    //$t0至$t9可任意使用,属于调用者保存的寄存器,在函数调用之前要先溢出到内存中。
	//$s0至$s7可任意使用,是被调用者保存的寄存器,函数内修改$s0至$s7,需在函数开头将原有数据压入栈,并在函数末尾恢复这些数据。
    for(int i = 8; i <= 15; i++)
		sprintf(Reg_List[i].name, "$t%d",i - 8);
    for(int i = 16; i <= 23; i++)
		sprintf(Reg_List[i].name, "$s%d",i - 16);
    strcpy(Reg_List[24].name, "$t8");
    strcpy(Reg_List[25].name, "$t9");
    strcpy(Reg_List[26].name, "$k0");
    strcpy(Reg_List[27].name, "$k1");
    strcpy(Reg_List[28].name, "$gp");
    strcpy(Reg_List[29].name, "$sp");
    strcpy(Reg_List[30].name, "$fp");
    strcpy(Reg_List[31].name, "$ra");		

	fprintf(fp, ".data\n");
	fprintf(fp, "_prompt: .asciiz \"Enter an integer:\"\n");
	fprintf(fp, "_ret: .asciiz \"\\n\"\n");
	fprintf(fp, ".globl main\n");
	int x = 0;
    while(x < cur_index)
    {
    	if(codes[x].kind == IR_DEC)
    		fprintf(fp, "%s: .word %d\n",codes[x].u.dec.op->u.name, codes[x].u.dec.size);
    	x++;
    }
	fprintf(fp, ".text\n");
	fprintf(fp, "read:\n");
	fprintf(fp, "  li $v0, 4\n");
	fprintf(fp, "  la $a0, _prompt\n");
	fprintf(fp, "  syscall\n");
	fprintf(fp, "  li $v0, 5\n");
	fprintf(fp, "  syscall\n");
	fprintf(fp, "  jr $ra\n\n");
	fprintf(fp, "write:\n");
	fprintf(fp, "  li $v0, 1\n");
	fprintf(fp, "  syscall\n");
	fprintf(fp, "  li $v0, 4\n");
	fprintf(fp, "  la $a0, _ret\n");
	fprintf(fp, "  syscall\n");
	fprintf(fp, "  move $v0, $0\n");
	fprintf(fp, "  jr $ra\n");
}
void Generate_Objectcode_File(FILE* fp)
{
    Init_Objectcode(fp);
    int x = 0;
    while(x < cur_index)
    {
        if(codes[x].kind == IR_LABEL)
    	    fprintf(fp, "%s:\n",codes[x].u.one.op->u.name);
        else if(codes[x].kind == IR_FUNCTION)
	    {
	   	    fprintf(fp, "\n%s:\n",codes[x].u.one.op->u.name);
            push_regs_saved_by_callee(fp); //保存被调用者保存寄存器
	   	    for(int i = 8; i < 26; i++)
			    Reg_List[i].free = 1;
		    cur_varible_list = NULL;
			Item item = find_items_in_global_table(codes[x].u.one.op->u.name);
			int t = 0;
			x++;
			while(x < cur_index && codes[x].kind == IR_PARAM)
			{ 			
				if(t < 4)
                    //保存到a0-a3
					add_varible_descriptor(4+t, codes[x].u.one.op);
				else
				{
					int reg_num = ensure(fp, codes[x].u.one.op);
					fprintf(fp, "  lw %s, %d($fp)\n", Reg_List[reg_num].name, (t-4)*4);
					add_varible_descriptor(reg_num, codes[x].u.one.op);
				}
				t++;
				x++;
			}
			x--;
	    }
        else if(codes[x].kind == IR_ASSIGN)
        {
    	    Operand left_op = codes[x].u.two.left;
    	    Operand right_op = codes[x].u.two.right;
            //右边是常数，只需要为左侧分配寄存器
    	    if(right_op->kind == OP_CONSTANT)
    	    {
    		    int left_num = ensure(fp, left_op);
    		    fprintf(fp, "  li %s, %d\n", Reg_List[left_num].name, right_op->u.value);
    	    } 	
    	    else
    	    {
    		    int left_reg = ensure(fp, left_op);
    		    int right_reg = ensure(fp, right_op);
    		    fprintf(fp, "  move %s, %s\n", Reg_List[left_reg].name, Reg_List[right_reg].name);   		
    	    }
        }
        else if(codes[x].kind == IR_ADD)
        {
            Operand result = codes[x].u.three.result;
    	    Operand op1 = codes[x].u.three.op1;
    	    Operand op2 = codes[x].u.three.op2;
		    if(op1->kind == OP_CONSTANT && op2->kind == OP_CONSTANT)
		    {
			    int res_reg = ensure(fp, result);
			    fprintf(fp, "  li %s, %d\n", Reg_List[res_reg].name, op1->u.value + op2->u.value);
		    }   
    	    else if(op1->kind != OP_CONSTANT && op2->kind == OP_CONSTANT)
    	    {
 			    int op1_reg = ensure(fp, op1);
			    int res_reg = ensure(fp, result);
			    fprintf(fp, "  addi %s, %s, %d\n",Reg_List[res_reg].name, Reg_List[op1_reg].name, op2->u.value);   		
    	    }
    	    else
    	    {
			    int op1_reg = ensure(fp, op1);
			    int op2_reg = ensure(fp, op2);
			    int res_reg = ensure(fp, result);
			    fprintf(fp, "  add %s, %s, %s\n",Reg_List[res_reg].name,Reg_List[op1_reg].name,Reg_List[op2_reg].name);
    	    }
        }
        else if(codes[x].kind == IR_SUB)
        {
            Operand result = codes[x].u.three.result;
    	    Operand op1 = codes[x].u.three.op1;
    	    Operand op2 = codes[x].u.three.op2;   
		    if(op1->kind == OP_CONSTANT && op2->kind == OP_CONSTANT)
		    {
			    int res_reg = ensure(fp, result);
			    fprintf(fp, "  li %s, %d\n", Reg_List[res_reg].name, op1->u.value - op2->u.value);
		    }   
    	    else if(op1->kind != OP_CONSTANT && op2->kind == OP_CONSTANT)
    	    {
 			    int op1_reg = ensure(fp, op1);
			    int res_reg = ensure(fp, result);
			    fprintf(fp, "  addi %s, %s, %d\n",Reg_List[res_reg].name, Reg_List[op1_reg].name, -op2->u.value);   		
    	    }
    	    else
    	    {
			    int op1_reg = ensure(fp, op1);
			    int op2_reg = ensure(fp, op2);
			    int res_reg = ensure(fp, result);
			    fprintf(fp, "  sub %s, %s, %s\n",Reg_List[res_reg].name, Reg_List[op1_reg].name, Reg_List[op2_reg].name);
    	    }	
        }
        else if(codes[x].kind == IR_MUL)
	    {
		    Operand result = codes[x].u.three.result;
		    Operand op1 = codes[x].u.three.op1;
		    Operand op2 = codes[x].u.three.op2; 
		    if(op1->kind == OP_CONSTANT && op2->kind == OP_CONSTANT)
		    {
			    int res_reg = ensure(fp, result);
			    fprintf(fp, "  li %s, %d\n", Reg_List[res_reg].name, op1->u.value * op2->u.value);
		    }  
		    else
		    {
			    int op1_reg = ensure(fp, op1);
			    int op2_reg = ensure(fp, op2);
			    int res_reg = ensure(fp, result);
			    fprintf(fp, "  mul %s, %s, %s\n",Reg_List[res_reg].name, Reg_List[op1_reg].name, Reg_List[op2_reg].name);
		    }	
	    }
	    else if(codes[x].kind == IR_DIV)
	    {
    	    Operand result = codes[x].u.three.result;
    	    Operand op1 = codes[x].u.three.op1;
    	    Operand op2 = codes[x].u.three.op2;   
		    int op1_reg = ensure(fp, op1);
		    int op2_reg = ensure(fp, op2);
		    int res_reg = ensure(fp, result);
		    fprintf(fp, "  div %s, %s\n", Reg_List[op1_reg].name, Reg_List[op2_reg].name);	
		    fprintf(fp, "  mflo %s\n", Reg_List[res_reg].name);	
	    }
        else if(codes[x].kind == IR_GET_ADDR)
        {
    	    int left_reg = ensure(fp, codes[x].u.two.left);
    	    int right_reg = ensure(fp, codes[x].u.two.right);
    	    fprintf(fp, "  la %s, %s\n", Reg_List[right_reg].name, codes[x].u.one.op->u.name);
    	    fprintf(fp, "  move %s, %s\n", Reg_List[left_reg].name, Reg_List[right_reg].name);
        }
	    else if(codes[x].kind == IR_INTO_ADDR_RIGHT)
	    {
	        Operand left = codes[x].u.two.left;
    	    Operand right = codes[x].u.two.right;
		    int left_reg = ensure(fp, left);
    	    int right_reg = ensure(fp, right);
    	    fprintf(fp, "  lw %s, 0(%s)\n", Reg_List[left_reg].name, Reg_List[right_reg].name);  		
	    }
	    else if(codes[x].kind == IR_INTO_ADDR_LEFT)
	    {
	        Operand left = codes[x].u.two.left;
    	    Operand right = codes[x].u.two.right;
		    int left_reg = ensure(fp, left);
    	    int right_reg = ensure(fp, right);
    	    fprintf(fp, "  sw %s, 0(%s)\n", Reg_List[right_reg].name, Reg_List[left_reg].name);  	
	    }
        else if(codes[x].kind == IR_GOTO)
		    fprintf(fp, "  j %s\n",codes[x].u.one.op->u.name);
	    else if(codes[x].kind == IR_IF_GOTO)
	    {
		    Operand op_x = codes[x].u.if_goto.x;
		    Operand op_relop = codes[x].u.if_goto.relop;
		    Operand op_y = codes[x].u.if_goto.y;
		    Operand op_z = codes[x].u.if_goto.z;
		    int x_reg = ensure(fp, op_x);
		    int y_reg = ensure(fp, op_y);
		    if(strcmp(op_relop->u.name,"==")==0)
			    fprintf(fp, "  beq %s, %s, %s\n",Reg_List[x_reg].name, Reg_List[y_reg].name, op_z->u.name);
		    else if(strcmp(op_relop->u.name,"!=")==0)
			    fprintf(fp, "  bne %s, %s, %s\n",Reg_List[x_reg].name, Reg_List[y_reg].name, op_z->u.name);
		    else if(strcmp(op_relop->u.name,">")==0)
			    fprintf(fp, "  bgt %s, %s, %s\n",Reg_List[x_reg].name, Reg_List[y_reg].name, op_z->u.name);
		    else if(strcmp(op_relop->u.name,"<")==0)
			    fprintf(fp, "  blt %s, %s, %s\n",Reg_List[x_reg].name, Reg_List[y_reg].name, op_z->u.name);			
		    else if(strcmp(op_relop->u.name,">=")==0)
			    fprintf(fp, "  bge %s, %s, %s\n",Reg_List[x_reg].name, Reg_List[y_reg].name, op_z->u.name);
		    else if(strcmp(op_relop->u.name,"<=")==0)
			    fprintf(fp, "  ble %s, %s, %s\n",Reg_List[x_reg].name, Reg_List[y_reg].name, op_z->u.name);				
	    }
        else if(codes[x].kind == IR_RETURN)
	    {
		    if(codes[x].u.one.op->kind == OP_CONSTANT && codes[x].u.one.op->u.value == 0)
			    fprintf(fp, "  move $v0, $0\n");
		    else
		    {
			    int reg_num = ensure(fp, codes[x].u.one.op);
			    fprintf(fp, "  move $v0, %s\n", Reg_List[reg_num].name);
		    }
            pop_regs_saved_by_callee(fp);
		    fprintf(fp, "  jr $ra\n");    
        }
        else if(codes[x].kind == IR_CALL)
	    {
		    Item item = find_items_in_global_table(codes[x].u.two.right->u.name);
		    int left_reg = ensure(fp, codes[x].u.two.left);
			int param_num = item->item_type->u.function.paramNum;
			push_regs_saved_by_caller(fp);	
		    //IR_ARG
		    int arg = x - 1, i = 0;
			//参数数量 <= 4
			if(param_num <= 4)
			{
				while(arg >= 0 && codes[arg].kind == IR_ARG)
				{
					int reg_num = ensure(fp, codes[arg].u.one.op);//给实参分配寄存器
					fprintf(fp, "  move %s, %s\n", Reg_List[i + 4].name, Reg_List[reg_num].name);
					i++, arg--;
				}
			}
			else
			{
				for(int i = 0; i < 4; i++)
				{
					int reg_num = ensure(fp, codes[arg].u.one.op);//给实参分配寄存器
					fprintf(fp, "  move %s, %s\n", Reg_List[i + 4].name, Reg_List[reg_num].name);
					i++, arg--;
				}
				fprintf(fp, "  addi $sp, $sp, -%d\n", (param_num-4)*4);
				while(arg >= 0 && codes[arg].kind == IR_ARG)
				{
					int reg_num = ensure(fp, codes[arg].u.one.op);
					fprintf(fp, "  sw %s, %d($sp)\n", Reg_List[reg_num].name,(i-4)*4);
					i++, arg--;
				}
			}
			//set fp
			fprintf(fp, "  move $fp, $sp\n");
			//save return address
			fprintf(fp, "  addi $sp, $sp, -4\n");
		    fprintf(fp, "  sw $ra, 0($sp)\n");
			//call	
		    fprintf(fp, "  jal %s\n", codes[x].u.two.right->u.name);
			//restore return address	
		    fprintf(fp, "  lw $ra, 0($sp)\n");
		    fprintf(fp, "  addi $sp, $sp, 4\n");
			if(param_num > 4)
			    fprintf(fp, "  addi $sp, $sp, %d\n", 4 * (param_num - 4));
		    pop_regs_saved_by_caller(fp);
			fprintf(fp, "  move %s, $v0\n", Reg_List[left_reg].name);  
	    }
		else if(codes[x].kind == IR_READ)
		{
			fprintf(fp, "  addi $sp, $sp, -4\n");
			fprintf(fp, "  sw $ra, 0($sp)\n");
			fprintf(fp, "  jal read\n");
			fprintf(fp, "  lw $ra, 0($sp)\n");
			fprintf(fp, "  addi $sp, $sp, 4\n");
			int reg_num = ensure(fp, codes[x].u.one.op);
			fprintf(fp, "  move %s, $v0\n", Reg_List[reg_num].name);
		}
		else if(codes[x].kind == IR_WRITE)
		{
			int reg_num = ensure(fp, codes[x].u.one.op);
			fprintf(fp, "  addi $sp, $sp, -8\n");
			fprintf(fp, "  sw $a0, 0($sp)\n");
			fprintf(fp, "  sw $ra, 4($sp)\n");
			fprintf(fp, "  move $a0, %s\n", Reg_List[reg_num].name);			
			fprintf(fp, "  jal write\n");
			fprintf(fp, "  lw $a0, 0($sp)\n");
			fprintf(fp, "  lw $ra, 4($sp)\n");			
			fprintf(fp, "  addi $sp, $sp, 8\n");		
		}
        x++;
    }
}
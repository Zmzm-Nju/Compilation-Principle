flex lexical.l
bison -d syntax.y
gcc main.c syntax.tab.c semantic.c intermediate.c object.c -lfl -ly -w -o parser
./parser ../Test/test$1.cmm temp.s

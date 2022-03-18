flex lexical.l
bison -d syntax.y
gcc main.c syntax.tab.c semantic.c -lfl -ly -w -o parser
echo "The result of $1:"
./parser ../Test/$1

flex lexical.l
bison -d syntax.y
gcc main.c syntax.tab.c semantic.c -lfl -ly -w -o parser
for i in `seq 1 17`; do
    echo "The result of Test$i:"
    ./parser ../Test/test$i.cmm
    echo ""
done
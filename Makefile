
all:
	flex --outfile=scanner.c scanner.l
	bison --yacc --defines --output=parser.c parser.y
	gcc -o executable scanner.c parser.c quads.c symtable.c target_code.c
	gcc -o AVM avm.c -lm
clean:
	rm scanner.c parser.c parser.h quads.txt instructions.txt binary.abc  executable AVM


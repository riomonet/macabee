# -*- Makefile -*-

# makefile embeds dependcy tree
# recipe

# target: dependencies
# 	action						

macabee: mongoose.o sqlite3.o main.o
	gcc mongoose.o sqlite3.o main.o

mongoose.o: mongoose.c
	gcc -c mongoose.c

sqlite3.o: sqlite3.c
	gcc -c sqlite3.c

main.o: main.c
	gcc -c main.c





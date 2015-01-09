gcc -std=c99 -c arc.c
gcc -std=c99 -c main.c
gcc arc.o main.o -o main
./main

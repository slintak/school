CC	= gcc
FLAGS	= -std=c99 -pedantic -Wall -Wextra -g

decode: proj1
	./$< -d 1 < out.txt.cs

code: proj1
	./$< -c 1 < in.txt.cs

proj1: proj1.c
	$(CC) $(FLAGS) -o $@ $<

todo:
	grep -i -n TODO *.c
	grep -i -n FIXME *.c

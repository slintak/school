CC	= gcc
FLAGS	= -std=c99 -pedantic -Wall -Wextra -lm -g

proj2: proj2.c
	$(CC) $(FLAGS) -o $@ $<

todo:
	grep -i -n TODO *.c
	grep -i -n FIXME *.c

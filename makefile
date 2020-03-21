OBJS = main.o childstruct.o clients.o pollstruct.o utils.o
SOURCE = main.c childstruct.c clients.c pollstruct.c utils.c
HEADER = childstruct.h clients.h pollstruct.h utils.h
OUT = main
CC = gcc
FLAGS = -o3 -lm

exec: $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o main

$(OUT): $(OBJS)
	$(CC) $(OBJS) -c $@

clean:
	rm -f $(OBJS) $(OUT)

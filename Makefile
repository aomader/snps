CC      = gcc
OBJECTS = src/snps-solver.o src/snps.o
CFLAGS  = -Wall -pedantic -std=c99 -O3
LIBS    = -lm
NAME    = snps

all: $(OBJECTS)
	$(CC) -o $(NAME) $(LIBS) $(OBJECTS)  `pkg-config --libs glib-2.0` -O3

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) `pkg-config --cflags glib-2.0` $< 

clean:
	rm src/*.o $(NAME)

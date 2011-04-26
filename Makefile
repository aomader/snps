CC      = gcc
OBJECTS = src/snps-solver.o src/snps.o
CFLAGS  = -Wall -pedantic -std=c99
NAME    = snps

all: $(OBJECTS)
	$(CC) -o $(NAME) $(OBJECTS)  `pkg-config --libs glib-2.0`

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) `pkg-config --cflags glib-2.0` $< 

clean:
	rm src/*.o $(NAME)

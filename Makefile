objects = parse.o encode.o setup_regex.o smallfunc.o
all_objects = as $(objects)
flags=-g -Wall -Wextra -Werror -pedantic
std=-std=c99
#optimize=-O3 -msse3 -msse4.1 -msse4.2 -mfpmath=both -ffast-math
optimize=
libs=-lm

target: $(all_objects)

as: as.c $(objects)
	gcc $(optimize) $(flags) $(std) -o as as.c $(objects) $(libs)

parse.o: parse.c parse.h
	gcc $(optimize) $(flags) $(std) -c parse.c

encode.o: encode.c encode.h
	gcc $(optimize) $(flags) $(std) -c encode.c

setup_regex.o: setup_regex.c setup_regex.h
	gcc $(optimize) $(flags) $(std) -c setup_regex.c

smallfunc.o: smallfunc.c smallfunc.h
	gcc $(optimize) $(flags) $(std) -c smallfunc.c

clean:
	rm -rf instr_sets/*~ *~ *.o $(all_objects)


objects = parse.o encode.o
all_objects = as $(objects)
flags=-g -Wall -Wextra -Werror -pedantic
std=-ansi
optimize=-O3

target: $(all_objects)

as: as.c $(objects)
	gcc $(optimize) $(flags) $(std) -o as as.c $(objects)

parse.o: parse.c parse.h
	gcc $(optimize) $(flags) $(std) -c parse.c

encode.o: encode.c encode.h
	gcc $(optimize) $(flags) $(std) -c encode.c

clean:
	rm -rf instr_sets/*~ *~ *.o $(all_objects)


EXE=redrocks

#  Main target
all: $(EXE)

CFLG=-O3 -Wall
LIBS=-lSDL -lSDL_mixer -lGLU -lGL -lm
CLEAN=rm -f $(EXE) *.o *.a

#  Dependencies
redrocks.o: redrocks.c CSCIx229.h
fatal.o: fatal.c CSCIx229.h
loadtexbmp.o: loadtexbmp.c CSCIx229.h
printsdl.o: printsdl.c CSCIx229.h
project.o: project.c CSCIx229.h
errcheck.o: errcheck.c CSCIx229.h
object.o: object.c CSCIx229.h

#  Create archive
CSCIx229.a:fatal.o loadtexbmp.o printsdl.o project.o errcheck.o object.o
	ar -rcs $@ $^

#  Compile rules
.c.o:
	gcc -c $(CFLG) $<
.cpp.o:
	g++ -c $(CFLG) $<

#  Link
redrocks:redrocks.o CSCIx229.a
	gcc -O3 -o $@ $^ $(LIBS)

#  Clean
clean:
	$(CLEAN)


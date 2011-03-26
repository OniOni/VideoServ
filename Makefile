CC=gcc
EDL=gcc
RM=rm
BUILDDIR=build/
SRCDIR=./
CCFLAGS=
LDFLAGS=
RMFLAGS=-f
EXE=serv.a
LIBS=
EFFACE=clean
OBJ=$(BUILDDIR)tcp_pull.o $(BUILDDIR)udp_pull.o $(BUILDDIR)utils.o $(BUILDDIR)central.o

$(EXE) : $(OBJ) $(BUILDDIR)main.o
	$(EDL) $(LDFLAGS) -o $(EXE) $(OBJ) $(LIBS) $(BUILDDIR)main.o

$(BUILDDIR)%.o : $(SRCDIR)%.c $(SRCDIR)%.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILDDIR)main.o : $(SRCDIR)main.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(EFFACE) :
	$(RM) $(RMFLAGS) $(BUILDDIR)*.o $(EXE) core

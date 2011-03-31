CC=gcc
EDL=gcc
RM=rm
BUILDDIR=build/
SRCDIR=src/
CCFLAGS=`pkg-config --cflags --libs glib-2.0`
LDFLAGS=`pkg-config --cflags --libs glib-2.0`
RMFLAGS=-f
EXE=serv.a
LIBS=
EFFACE=clean
OBJ=$(BUILDDIR)tcp_pull.o $(BUILDDIR)udp_pull.o $(BUILDDIR)utils.o $(BUILDDIR)central.o $(BUILDDIR)net_utils.o $(BUILDDIR)tcp_push.o $(BUILDDIR)tcp_utils.o $(BUILDDIR)udp_utils.o

$(EXE) : $(OBJ) $(BUILDDIR)main.o
	$(EDL) $(LDFLAGS) -o $(EXE) $(OBJ) $(LIBS) $(BUILDDIR)main.o

$(BUILDDIR)%.o : $(SRCDIR)%.c $(SRCDIR)%.h
	$(CC) $(CCFLAGS) -c $< -o $@

$(BUILDDIR)main.o : $(SRCDIR)main.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(EFFACE) :
	$(RM) $(RMFLAGS) $(BUILDDIR)*.o $(EXE) core

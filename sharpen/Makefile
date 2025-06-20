MF=	Makefile

#COMPILER
CC=	oshcc
# On ARCHER, cc is a wrapper for whichever C compiler - GNU (gcc), Intel (icc), or Cray (craycc) - has been chosen by loading the appropriate PrgEnv module prior to compilation.

CFLAGS=	-ggdb3 -O0 -DC_OPENSHMEM_PRACTICAL
LDFLAGS='-lm'

EXE=	sharpen

SRC= \
	sharpen.c \
	dosharpen.c \
	filter.c \
	cio.c \
	utilities.c

INC = \
	sharpen.h \
	utilities.h

#
# No need to edit below this line
#

.SUFFIXES:
.SUFFIXES: .c .o

OBJ=	$(SRC:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(EXE)

$(EXE):	$(OBJ)
	$(CC) $(FFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(OBJ):	$(MF) $(INC)

clean:
	rm -f $(OBJ) $(EXE) core

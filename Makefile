GPP			=	g++
CFLAGS	=	-O2 -Wall


all: main.cpp
			$(GPP) $(CFLAGS) main.cpp -o main.out

clean:
			rm *.out
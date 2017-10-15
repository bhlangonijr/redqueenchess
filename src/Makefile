CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -O3 -Wall 
LDFLAGS=-g -O3 
LDLIBS=-lpthread 

SRCS=board.cpp evaluator.cpp magicmoves.cpp mersenne.cpp searchagent.cpp simplepvsearch.cpp stringutil.cpp uci.cpp ucioption.cpp main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: redqueen

redqueen: $(OBJS) 
	$(CXX) $(LDFLAGS) -o redqueen $(OBJS) $(LDLIBS) 

clean: 
	$(RM) $(OBJS)

distclean: clean 
	$(RM) redqueen
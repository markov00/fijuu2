# you need to 'make clean' after changing any of these

# uncomment this to turn on frame limiting 
#
#FRAMELIMIT=60

# set this to 0 to turn off debug input
#
DEBUG_INPUT=0




CXXFLAGS=$(shell pkg-config --cflags OGRE liblo ) 
LDFLAGS=$(shell pkg-config --libs OGRE liblo) -lSDL -lm

ifneq ($(strip $(FRAMELIMIT)),)
CXXFLAGS+=-DFRAMELIMIT=$(FRAMELIMIT)
endif


CXXFLAGS+=-DDEBUG_INPUT=$(DEBUG_INPUT)
		    

SRCS=fijuuu.cpp 

all: fijuuu 

.PHONY: depend clean
Makefile.dep: $(SRCS)
	$(CXX) -M $(CXXFLAGS) $(SRCS) > Makefile.dep

clean:
	rm *.o Makefile.dep

include Makefile.dep

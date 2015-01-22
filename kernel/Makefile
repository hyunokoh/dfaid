CFLAGS = -g -std=c++11
INCLUDE=../include
LIBS = -L../kernel -ldataflow
CPP = g++
all : libdataflow.so

libdataflow.so : Fraction.cc graph.cc
	$(CPP) $(CFLAGS) -shared -o $@ $^ -I$(INCLUDE) 

clean : 
	rm *.so 


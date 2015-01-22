CFLAGS = -g -std=c++11
INCLUDE=../include
LIBS = -L../kernel -ldataflow
CPP = g++
all : prodcons kmeans

prodcons : prodcons.cc
	$(CPP) $(CFLAGS) -o $@ $^ -I$(INCLUDE) $(LIBS)

prodcons_par : prodcons_par.cc
	$(CPP) $(CFLAGS) -o $@ $^ -I$(INCLUDE) $(LIBS)

kmeans : kmeans.cc
	$(CPP) $(CFLAGS) -o $@ $^ -I$(INCLUDE) $(LIBS)

clean : 
	rm prodcons
	rm prodcons_par
	rm kmeans


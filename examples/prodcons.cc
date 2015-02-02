#include <vector>
#include <ratio>
#include <iostream>
#include <thread>
#include <string.h>

#include "graph.h"

using namespace std;
using namespace dataflow;

class Prod : public Actor
{
public:
	Prod() { addOutport(&output); }

	void setup() { output.setRate(2); Actor::setup(); }
	void init() { state = 0; Actor::init(); }
	void go(int i=0) { output[2*i] = state; output[2*i+1] = state+1; state += 2; }

	outport<int> output;
private:
	int state;
};

class Cons : public Actor
{
public:
	Cons() { addInport(&input); }

	void go(int i=0) { cout << input[i] << " "; }

	inport<int> input;
};
class Prodcons : public Graph
{
public:
	Prod prod;
	Cons cons;

	void create();
	void setup();
	void go();

	void goSequential();
	void goParallel();
};

void Prodcons::go()
{
	goParallel();
}

void Prodcons::goSequential()
{ 
	int i;
	for(i=0; i<2; i++) {
		prod.go(i);
	}
	prod.updateIndices(2);
	for(i=0; i<4; i++) {
		cons.go(i);
	}
	cons.updateIndices(4);
}

void Prodcons::goParallel()
{
	int i;
	thread prod0(&Prod::go,&prod,0);
	thread prod1(&Prod::go,&prod,1);

	prod0.join();
	prod1.join();
	prod.updateIndices(2);

	thread cons0(&Cons::go,&cons,0);
	thread cons1(&Cons::go,&cons,1);
	thread cons2(&Cons::go,&cons,2);
	thread cons3(&Cons::go,&cons,3);

	cons0.join();
	cons1.join();
	cons2.join();
	cons3.join();
	cons.updateIndices(4);
}

	
void Prodcons::create()
{
	addActor(&prod);
	addActor(&cons);

	connect(&prod, &prod.output, &cons, &cons.input);
}	

void Prodcons::setup()
{
	prod.output.setMinBufferSize(4);

	Graph::setup();
}

int main(int argc, const char* argv[])
{
	Prodcons prodcons;
	prodcons.create();

	prodcons.setup();
	prodcons.init();
	int i;

	if(argc>=2 && strcmp(argv[1],"-par")==0) {
		for(i=0; i<10; i++) {
			prodcons.goParallel();
		}
	} else {
		for(i=0; i<10; i++) {
			prodcons.goSequential();
		}
	}

	prodcons.wrapup();
}

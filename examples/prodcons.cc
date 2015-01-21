#include <vector>
#include <ratio>
#include <iostream>

#include "graph.h"

using namespace std;
using namespace dataflow;

class Prod : public Actor
{
public:
	Prod() { addOutport(&output); }

	void setup() { output.setRate(2); Actor::setup(); }
	void init() { state = 0; Actor::init(); }
	void go() { output[0] = state; output[1] = state+1; state += 2; Actor::go(); }

	outport<int> output;
private:
	int state;
};

class Cons : public Actor
{
public:
	Cons() { addInport(&input); }

	void go() { cout << input[0] << " "; Actor::go(); }

	inport<int> input;
};
class Prodcons : public Graph
{
public:
	void create();

	void go() { pActorList[0]->go(); pActorList[1]->go(); pActorList[1]->go(); }
};
	
void Prodcons::create()
{
	Prod* prod = new Prod;
	Cons* cons = new Cons;

	addActor(prod);
	addActor(cons);

	connect(prod, &prod->output, cons, &cons->input);
}	

int main()
{
	Prodcons prodcons;
	prodcons.create();

	prodcons.setup();
	prodcons.init();
	int i;
	for(i=0; i<10; i++) {
		prodcons.go();
	}

	prodcons.wrapup();
}

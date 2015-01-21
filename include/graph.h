#ifndef __DFGRAPH__
#define __DFGRAPH__

#include <vector>
#include <ratio>
#include <iostream>
#include "Fraction.h"

using namespace std;

namespace dataflow {

class Edge;

class Port
{
public:
	Port() : pRate(1), pEdge(0), pIndexBound(1), pIndex(0) {}

	Edge* pEdge;

	Fraction pRate;
	Fraction pIndex;
	Fraction pIndexBound; 
public:
	virtual void setup() {}
	virtual void init(){ pIndex = 0; }
	virtual void phase_init() { pIndex = 0; }
	virtual void wrapup(){} 
	
	void advanceIndex();

	void setEdge(Edge* theEdge) { pEdge = theEdge; }

	int getBufferSize();

	void setRate(Fraction theRate) { pRate = theRate; }
	void setRate(int theRate) { pRate = theRate; }

	void setIndexBound(Fraction theBound) { pIndexBound = theBound; }
	Fraction getIndexBound() { return pIndexBound; }
};

template<typename Type> class inport : public Port
{
public:
	inport<Type>() : mBuffer(0) {}

	/*virtual*/ void setup();
	Type& operator[](int index) { return mBuffer[(int)pIndex+index]; }

	void setBuffer(Type *theBuffer) { mBuffer = theBuffer; }
private:
	Type* mBuffer;
};

template<typename Type>class outport : public Port
{
public:
	outport<Type>() : mBuffer(0) {}

	/*virtual*/ void setup();
	/*virtual*/ void wrapup() {}
	Type& operator[](int index) { return mBuffer[index+(int)pIndex]; }

	Type* getBuffer() { return mBuffer; }

private:
	Type* mBuffer;
};

class Actor
{
public:
	void addInport(Port* input) { mInportList.push_back(input); }
	void addOutport(Port* output) { mOutportList.push_back(output); }

	virtual void setup(); 
	virtual void init();
	virtual void phase_init(){}
	virtual void go();
	virtual void wrapup() {}

	void updateIndices();
private:
	vector<Port*> mInportList;
	vector<Port*> mOutportList;
};

class Edge
{
public:
	void connect(Actor* source, Port* sourcePort, Actor* sink, Port* sinkPort);

	virtual void setup(){ mSourcePort->setup(); mSinkPort->setup(); }
	virtual void init(){ mSourcePort->init(); mSinkPort->init(); }
	virtual void phase_init(){ mSourcePort->phase_init(); mSinkPort->phase_init(); }
	virtual void wrapup(){ mSourcePort->wrapup(); mSinkPort->wrapup(); }
	
	Port* getSourcePort() { return mSourcePort; }
	Port* getSinkPort() { return mSinkPort; }

private:
	Actor* mSourceActor;
	Actor* mSinkActor;

	Port* mSourcePort;
	Port* mSinkPort;
};

class Graph
{
protected:
	vector<Actor*> pActorList;
	vector<Edge*> pEdgeList;

public:
	void addActor(Actor* node) { pActorList.push_back(node); }
	void connect(Actor* a1, Port* p1, Actor* a2, Port* p2);

	virtual void create() = 0;
	virtual void setup();
	virtual void init();
	virtual void phase_init();
	virtual void go();
	virtual void wrapup();
};

template<typename Type>
void outport<Type>::setup()
{
	Port::setup();

        int sourceBufferSize = getBufferSize();
        int sinkBufferSize = pEdge->getSinkPort()->getBufferSize();

	int bufferSize = (sourceBufferSize>sinkBufferSize)?sourceBufferSize:sinkBufferSize;
	mBuffer = new Type[bufferSize];

	pIndexBound = bufferSize;
}

template<typename Type>
void inport<Type>::setup()
{
	Port::setup();

        mBuffer = dynamic_cast<outport <Type>* > (pEdge->getSourcePort())->getBuffer();
	pIndexBound = pEdge->getSourcePort()->getIndexBound();
}

}

#endif

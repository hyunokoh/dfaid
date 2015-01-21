#include <vector>
#include <ratio>
#include <iostream>

#include "graph.h"

using namespace std;

namespace dataflow {

int Port::getBufferSize() 
{
	int bufferSize = (int)pRate;
	if(pRate.numerator() % pRate.denominator()>0) 
		bufferSize++;

	return bufferSize;
}

void Port::advanceIndex()
{
	pIndex += pRate;
	if(pIndex>=pIndexBound) pIndex = 0;
}

void Edge::connect(Actor* source, Port* sourcePort, Actor* sink, Port* sinkPort) 
{
	mSourceActor = source; mSinkActor = sink; mSourcePort = sourcePort; mSinkPort = sinkPort; 
	mSourcePort->setEdge(this);
	mSinkPort->setEdge(this);
}

void Actor::setup()
{
}

void Actor::init()
{
}

void Actor::go()
{
	updateIndices();
}

void Actor::updateIndices()
{
	vector<Port*>::iterator port;

	for(port=mOutportList.begin(); port!=mOutportList.end();++port) {
		(*port)->advanceIndex();
	}

	for(port=mInportList.begin(); port!=mInportList.end();++port) {
		(*port)->advanceIndex();
	}
}

void Graph::connect(Actor* a1, Port* p1, Actor* a2, Port* p2) {
	Edge* edge = new Edge;
	edge->connect(a1,p1,a2,p2);
	pEdgeList.push_back(edge);
}

void Graph::setup()
{
	vector<Actor*>::iterator actor;
	for(actor = pActorList.begin(); actor != pActorList.end(); ++actor) {	
		(*actor)->setup();
	}		

	vector<Edge*>::iterator edge;
	for(edge = pEdgeList.begin(); edge != pEdgeList.end(); ++edge) {	
		(*edge)->setup();
	}		
}

void Graph::init()
{
	vector<Actor*>::iterator actor;
	for(actor = pActorList.begin(); actor != pActorList.end(); ++actor) {	
		(*actor)->init();
	}		

	vector<Edge*>::iterator edge;
	for(edge = pEdgeList.begin(); edge != pEdgeList.end(); ++edge) {	
		(*edge)->init();
	}		
}

void Graph::phase_init()
{
	vector<Actor*>::iterator actor;
	for(actor = pActorList.begin(); actor != pActorList.end(); ++actor) {	
		(*actor)->phase_init();
	}		

	vector<Edge*>::iterator edge;
	for(edge = pEdgeList.begin(); edge != pEdgeList.end(); ++edge) {	
		(*edge)->phase_init();
	}
}

void Graph::go()
{
	vector<Actor*>::iterator actor;
	for(actor = pActorList.begin(); actor != pActorList.end(); ++actor) {	
		(*actor)->go();
	}		
}

void Graph::wrapup()
{
	vector<Actor*>::iterator actor;
	for(actor = pActorList.begin(); actor != pActorList.end(); ++actor) {	
		(*actor)->wrapup();
	}		

	vector<Edge*>::iterator edge;
	for(edge = pEdgeList.begin(); edge != pEdgeList.end(); ++edge) {	
		(*edge)->wrapup();
	}
}

}

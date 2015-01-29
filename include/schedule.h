#include "graph.h"

namespace dataflow {

class Schedule 
{
public:
	void setGraph(Graph* theGraph) { pGraph = theGraph; }

};

class DynamicSchedule : public Schedule 
{
public:
	
	void setup();
	void init();
	void phase_init();
	void go();
	void wrapup();

};

// topological sorting
void DynamicSchedule::setup()
{
}

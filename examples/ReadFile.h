
#include <string>
#include <stdio.h>

#include "graph.h"
#include "page.h"
// file read simulation actor

using namespace std;
using namespace dataflow;

class ReadFile : public Actor 
{
public:
	ReadFile() { addOutport(&output); }

	outport<page_type> output;

	void init();
	void phase_init();
	void go();

	bool isNoInput() { return mNoInputFlag; }

	FILE *ifp;
	string fileName;
private:
	bool mNoInputFlag; 
};

void ReadFile::init()
{
	ifp=fopen(fileName.c_str(),"r");
	Actor::init();
}

void ReadFile::phase_init()
{
	fseek(ifp,0,0); 
	mNoInputFlag = false;

	Actor::phase_init();
}

void ReadFile::go()
{
	// read a page from a file
	if(fread(output[0].data, sizeof(page_type),1,ifp)<=0) {
		mNoInputFlag = true;
	} 

	Actor::go();
}

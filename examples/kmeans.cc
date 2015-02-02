#include "graph.h"

#include <math.h>

using namespace dataflow;

//#include "page.h"

#define MAX_POINTS 32 
#define MAX_CENTROID 10
#define MAX_DIMENSION 2 
#define PAGE_SIZE (MAX_POINTS*sizeof(int)*MAX_DIMENSION)

typedef struct {
        char data[PAGE_SIZE];
} page_type;

typedef struct {
        int data[PAGE_SIZE/sizeof(int)];
} index_type;

typedef struct {
        int data[PAGE_SIZE/sizeof(int)];
} centroidIndex_type;

typedef struct {
        float data[MAX_CENTROID][MAX_DIMENSION];
} centroid_type;

typedef union {
        char cdata[4];
        int idata;
} item_type;

#include "ReadFile.h"

class Fork : public Actor
{
public:
	Fork() { addInport(&input); addOutport(&output1); addOutport(&output2); }

	inport<page_type> input;
	outport<page_type> output1;
	outport<page_type> output2;

	void setup();
	void go(int=0);
};

void Fork::setup()
{
	output1.embed(&input);
	output2.embed(&input);
}

void Fork::go(int i)
{
}

class ClosestCentroid : public Actor
{
public:
	ClosestCentroid() { addInport(&page); addInport(&centroid); addOutport(&index); }

	inport<page_type> page;
	inport<centroid_type> centroid;
	outport<centroidIndex_type> index;

	void setup();
	void init();
	void go(int=0);

	int numCentroids;
	int dimension;
	int numPoints;
};

void ClosestCentroid::setup()
{
	numPoints = PAGE_SIZE/dimension/sizeof(int);
	cout << "numPoints : " << numPoints << endl;
	Actor::setup();
}

void ClosestCentroid::init()
{
        // initialize centroid
        int i,j;
        for(i=0; i<numCentroids; i++) {
		cout << "centroid( ";
                for(j=0; j<dimension; j++) {
                        centroid[0].data[i][j] = i*10;
			cout << centroid[0].data[i][j] << " ";
                }
		cout << ")" << endl;
        }
	cout << "-----------------------" <<endl;

	Actor::init();
}

void ClosestCentroid::go(int inst)
{
	int pi;
	for(pi=0; pi<numPoints;pi++) {
		// compute distance
		float minDistance = 0;
		int minIndex = -1;
		int i,j,k;
		for(i=0;i<numCentroids;i++) {
			float distance = 0;
			for(j=0; j<dimension; j++) {
				float centroid_temp = centroid[0].data[i][j];

				item_type item;
				item.idata = page[inst].data[pi*dimension+j]%100;
				item.idata = (item.idata<0)?-item.idata:item.idata;
				distance += (item.idata-centroid_temp) * (item.idata-centroid_temp);
			}

			if(minIndex==-1 || minDistance>distance) {
			    minDistance = distance;
			    minIndex = i;
			}
		}

		//minDistance = sqrt(minDistance);

		//printf("minIndex : %d, minDistance %f\n", minIndex, minDistance);
		index[inst].data[pi] = minIndex;
	}
}

class UpdateCentroid : public Actor
{
public:
	UpdateCentroid() { addInport(&index); addInport(&page); addOutport(&centroid); }

	inport<centroidIndex_type> index;
	inport<page_type> page;
	outport<centroid_type> centroid;

	void setup();
	void init();
	void phase_init();
	void go(int=0);
	void wrapup();

	int numCentroids;
	int dimension;
	int numPoints;

	bool isConverged() { return mConvergedFlag; }
private:
	bool mConvergedFlag;

private:
        centroid_type cent;
        int* sizeofCentroid;
};

void UpdateCentroid::setup()
{
	numPoints = PAGE_SIZE/dimension/sizeof(int);
	cout << "numPoints : " << numPoints << endl;

        sizeofCentroid = new int[numCentroids];
	Actor::setup();
}

void UpdateCentroid::init()
{
	mConvergedFlag = false;
        int i,j;
        for(i=0; i<numCentroids; i++) {
                sizeofCentroid[i] = 0;
                for(j=0; j<dimension;j++)
                        cent.data[i][j] = 0;
        }

	Actor::init();
}

void UpdateCentroid::phase_init()
{
        int differentFlag = false;
        int i,j,p;
        for(i=0;i<numCentroids;i++) {
		cout << "centroid( ";
                for(j=0; j<dimension;j++) {
                        if(sizeofCentroid[i]!=0) {
                                float newcent = cent.data[i][j]/sizeofCentroid[i];
                                if(newcent != centroid[0].data[i][j])
                                        differentFlag = true;
                                centroid[0].data[i][j] = newcent;
				cout << centroid[0].data[i][j] << " ";
                	}
                        cent.data[i][j] = 0;
                }
		cout << ")" << endl;
                sizeofCentroid[i] = 0;
        }
	cout << "-----------------------" <<endl;

        if(!differentFlag) {
		mConvergedFlag = true;	
        }

	Actor::phase_init();
}

void UpdateCentroid::go(int i)
{
	int p,j,k;
        for(p=0; p<numPoints; p++) {
                int id = index[i].data[p];
                sizeofCentroid[id]++;
                for(j=0; j<dimension; j++) {
                        int item;
                        item = page[i].data[p*dimension+j]%100;
                        cent.data[id][j] += item;
                }
        }
}

void UpdateCentroid::wrapup()
{
	delete sizeofCentroid;

	Actor::wrapup();
}

class Kmeans : public Graph
{
public:
	ReadFile read;
	Fork fork;
	ClosestCentroid cc;
	UpdateCentroid uc;
		
        void create();
	void setup();

        void go();

	void goOnce(int);

	bool isNoInput() { return read.isNoInput(); }
	bool isConverged() { return uc.isConverged(); }

	void setParallelism(int thePar) { mParallelism = thePar; }
private:
	int mParallelism;
};

void Kmeans::goOnce(int i)
{
//	fork.go(i);
	cc.go(i);
//	uc.go(i);
}

void Kmeans::go()
{

	read.go();
	cc.go();
	uc.go();
	read.updateIndices();
	cc.updateIndices();
	uc.updateIndices();
/*
	// current ReadFile has an internal state
	vector<thread> threadList;

	int i;
	int numPar = 0;
	for(i=0; i<mParallelism; i++) {
		if(read.isNoInput()) break; 
		read.go(i); 
		numPar++;
	}

	for(i=0; i<numPar; i++) {
		threadList.push_back(thread(&Kmeans::goOnce, this, i));
	//	cc.go(i);
	}
	
	vector<thread>::iterator th;
	for(th=threadList.begin(); th!=threadList.end(); ++th) {
		(*th).join();
	}

	for(i=0; i<numPar; i++) {
		uc.go(i);
	}

	read.updateIndices(numPar);
	cc.updateIndices(numPar);
	uc.updateIndices(numPar);
*/
}

void Kmeans::create()
{
        addActor(&read);
	addActor(&fork);
        addActor(&cc);
        addActor(&uc);

        connect(&read, &read.output, &fork, &fork.input);
        connect(&fork, &fork.output1, &cc, &cc.page);
        connect(&fork, &fork.output2, &uc, &uc.page);
        connect(&cc, &cc.index, &uc, &uc.index);
        connect(&uc, &uc.centroid, &cc, &cc.centroid);
}      

void Kmeans::setup()
{
	read.fileName = "prodcons.cc";

	cc.numCentroids = 10;
        cc.dimension = 2;

	uc.numCentroids = 10;
        uc.dimension = 2;

	read.output.setMinBufferSize(mParallelism);
	cc.index.setMinBufferSize(mParallelism);

	Graph::setup();
}

int main()
{
        Kmeans kmeans;
	kmeans.setParallelism(1);

        kmeans.create();

        kmeans.setup();
	kmeans.init();

        int i;
        for(i=0; i<1000; i++) {
                kmeans.go();

		if(kmeans.isNoInput()) kmeans.phase_init();
		if(kmeans.isConverged()) break;
        }

	kmeans.wrapup();
}

#include "graph.h"

#include <math.h>

using namespace dataflow;

#include "page.h"
#define MAX_CENTROID 10
#define MAX_DIMENSION 2 

typedef struct {
        int data[PAGE_SIZE];
} index_type;

typedef struct {
        int data[MAX_CENTROID];
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

	void go();
};

void Fork::go()
{
	output1[0] = input[0];
	output2[0] = input[0];

	Actor::go();
}

class ClosestCentroid : public Actor
{
public:
	ClosestCentroid() { addInport(&page); addInport(&centroid); addOutport(&index); }

	inport<page_type> page;
	inport<centroid_type> centroid;
	outport<centroidIndex_type> index;

	void init();
	void go();

	int numCentroids;
	int dimension;
	int numPoints;
};

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

void ClosestCentroid::go()
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
				item.idata = page[0].data[pi*dimension+j]%100;
				item.idata = (item.idata<0)?-item.idata:item.idata;
				distance += (item.idata-centroid_temp) * (item.idata-centroid_temp);
			}

			if(minIndex==-1 || minDistance>distance) {
			    minDistance = distance;
			    minIndex = i;
			}
		}

		minDistance = sqrt(minDistance);

		//printf("minIndex : %d, minDistance %f\n", minIndex, minDistance);
		index[0].data[pi] = minIndex;
	}

	Actor::go();
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
	void go();
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
                }
		cout << ")" << endl;
        }
	cout << "-----------------------" <<endl;

        if(!differentFlag) {
		mConvergedFlag = true;	
        }

	Actor::phase_init();
}

void UpdateCentroid::go()
{
	int p,j,k;
        for(p=0; p<numPoints; p++) {
                int id = index[0].data[p];
                sizeofCentroid[id]++;
                for(j=0; j<dimension; j++) {
                        int item;
                        item = page[0].data[p*dimension+j]%100;
                        cent.data[id][j] += item;
                }
        }

	Actor::go();
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

        void go() { read.go(); fork.go(); cc.go(); uc.go(); }

	bool isNoInput() { return read.isNoInput(); }
	bool isConverged() { return uc.isConverged(); }
};

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
	read.fileName = "kmeans.cc";

        cc.numPoints = 32;
	cc.numCentroids = 10;
        cc.dimension = 2;

        uc.numPoints = 32;
	uc.numCentroids = 10;
        uc.dimension = 2;

	Graph::setup();
}

int main()
{
        Kmeans kmeans;
        kmeans.create();

        kmeans.setup();
	kmeans.init();
	//kmeans.phase_init();
        int i;
        for(i=0; i<1000; i++) {
                kmeans.go();

		if(kmeans.isNoInput()) kmeans.phase_init();
		if(kmeans.isConverged()) break;
        }

	kmeans.wrapup();
}

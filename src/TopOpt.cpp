#include "TopOpt.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <float.h>
#include <math.h>
#include <string>
using namespace std;

void printData(tfloat *data, int size);
void printVector(vector<int> &v);

tfloat passiveRadius = 0;

TopOpt::TopOpt(int nelx, int nely, tfloat volfrac, tfloat penal, tfloat rmin, tfloat maxChange)
:nelx(nelx), nely(nely), 
volfrac(volfrac), penal(penal), rmin(rmin),
KE(8,8),K(2*(nelx+1)*(nely+1),2*(nely+2)+2),
mIter(0),
mMaxChange(maxChange),
mChange(1.0),
xold(nely, nelx)
{
    x = new Matrix(nely, nelx);
    xNew = new Matrix(nely, nelx);

    dc = new Matrix(nely, nelx);
    dcNew = new Matrix(nely, nelx);
    F = new tfloat[2*(nelx+1)*(nely+1)];
    U = new tfloat[2*(nelx+1)*(nely+1)];
    initKE();
    initFreeDofs();
    
    x->fill(volfrac);
    calculatePassive();
	float x, y, z;
	step(x,y,z);

    // start iteration
//     while (change > maxChange){
//         loop ++;
//         xold.set(*x);
// 
//         FEAnalysis();
//         
//         tfloat compliance = objectiveFunctionAndSensitivityAnalyses();
//         
//         // Filtering of sensitivities
//         meshIndependencyFilter();
//         
//         // Design update by the optimality criteria method
//         optimalityCriteriaBasedOptimization();
//         
//         // print results
//         change = x->maxDiff(xold);
//         tfloat volume = x->sum()/(nelx*nely);
//         printf("%% It.: %i Obj.: %f Vol.: %f ch.: %f \n", loop, compliance, volume, change);
//     }
   // exportResultMatlab(x, loop);
	//exportResultTxt(x, loop);

}

TopOpt::~TopOpt()
{
	delete x;
	delete xNew;
	delete U;
	delete dc;
	delete dcNew;
	delete F;
}

bool TopOpt::step(float& compliance, float& volume, float& change)
{

	if (mChange < mMaxChange){
		compliance = mCurCompliance;
		volume = mCurVolume;
		change = mChange;
		return false;
	}
		
		mIter++;
		xold.set(*x);

		FEAnalysis();

		mCurCompliance = objectiveFunctionAndSensitivityAnalyses();

		// Filtering of sensitivities
		meshIndependencyFilter();

		// Design update by the optimality criteria method
		optimalityCriteriaBasedOptimization();

		// print results
		mChange = x->maxDiff(xold);
		mCurVolume = x->sum() / (nelx*nely);
		
		compliance = mCurCompliance;
		volume = mCurVolume;
		change = mChange;
}

void TopOpt::exportResultMatlab(Matrix *x, int iteration){
    std::cout<<std::endl;
    printf("x = [");
    x->print();
    printf("];\n");
    printf("colormap(gray); imagesc(-x); axis equal; axis tight; axis off;pause(1e-6);");
//    printf("imwrite(1-x,'topopt_%i.png');", iteration);
//    printf("imwrite(1-x,'topopt_volfrac_%f_%i.png');", volfrac,iteration);
    printf("imwrite(1-x,'topopt_passive_%f_%i.png');", passiveRadius,iteration);
#ifdef RAMP    
    //printf("imwrite(1-x,'topopt_RAMP_%f_%i.png');", penal,iteration);
#else
    //printf("imwrite(1-x,'topopt_SIMP_%f_%i.png');", penal,iteration);
    
#endif
    std::cout<<std::endl<<std::flush;
}

void TopOpt::exportResultTxt(Matrix *x, int iteration)
{
	ofstream myfile;
	myfile.open("example.txt");
	myfile << "iteration: " << std::to_string(iteration) << std::endl;

	for (int i = 0; i < x->getRows(); i++){
		for (int j = 0; j < x->getColumns(); j++){
			myfile << std::to_string(x->get(i, j)) << ",";
		}
		myfile << std::endl;
	}
	myfile.close();
}

void TopOpt::calculatePassive(){
    tfloat centerY = nely/2.;
    tfloat centerX = nelx/5.;
    tfloat radius = nely/3.;
    radius = passiveRadius;
    std::cout<<"Radius "<<radius<<std::endl;
    for (int ely = 1;ely<=nely;ely++){
        for (int elx = 1;elx<=nelx;elx++){
            bool isInsidePassiveDisk = sqrt((ely-centerY)*(ely-centerY)+(elx-centerX)*(elx-centerX)) < radius;
            if (isInsidePassiveDisk){
                int index = x->getIndex(ely-1, elx-1);
                passiveNoMaterial.push_back(index);
                x->set(ely-1, elx-1, 0.001);
            }
        }
    }
}


void TopOpt::FEAnalysis(){
    int edof[8]; // elements degrees of freedom
    K.fill(0);
    memset(F, 0, sizeof(tfloat)*K.getRows());
    memset(U, 0, sizeof(tfloat)*K.getRows());
    
    // Assamble matrix K based on x
    for (int ely = 1; ely <= nely; ely++) {
        for (int elx = 1; elx <= nelx; elx++) {
            int n1 = (nely + 1) * (elx - 1) + ely;
            int n2 = (nely + 1) * elx + ely;
            edof[0] = 2*n1-1 -1;
            edof[1] = 2*n1   -1;
            edof[2] = 2*n2-1 -1;
            edof[3] = 2*n2   -1;
            edof[4] = 2*n2+1 -1;
            edof[5] = 2*n2+2 -1;
            edof[6] = 2*n1+1 -1;
            edof[7] = 2*n1+2 -1;
            tfloat xPenal = interpolate(elx, ely);
            for (int x = 0; x < 8; x++) {
                for (int y = 0;y < 8; y++) {
                    int ny = edof[y];
                    int nx = edof[x];
                    if (!K.isBanded() || ny <= nx) // only set lower left matrix if banded
                        K.add(ny, nx, xPenal*KE.get(y, x));
                }
            }
        }
    }
    // add loads to vector F
    defineLoads();
    
    // solve KU=F <=> U = F/K
    bool solved = K.solve(F, U, fixeddofs);
    assert(solved);
    for (int i=0;i<fixeddofs.size();i++){
        U[fixeddofs[i]] = 0;
    }
}

tfloat TopOpt::interpolate(int elx, int ely) {
#ifdef RAMP
    tfloat xVal = x->get(ely-1, elx-1);
    tfloat xPenal = xVal/(1+penal*(1-xVal));
#else // SIMP
    tfloat xPenal = pow(x->get(ely-1, elx-1), penal);
#endif
    return xPenal;
}

tfloat TopOpt::interpolateDiff(int elx, int ely) {
#ifdef RAMP
    // MABLE 
    // fn := proc (x) options operator, arrow; x/(1+p*(1-x)) end proc
    // diff(fn(x), x)
    tfloat xVal = x->get(ely-1, elx-1);
    tfloat tmp =1+penal*(1-xVal); 
    return (1+penal)/(tmp*tmp);
    
#else // SIMP
    return penal*pow(x->get(ely-1,elx-1),(penal-1));
#endif
}

tfloat TopOpt::objectiveFunctionAndSensitivityAnalyses(){
    tfloat Ue[8]; // Displacement for an element
    tfloat compliance = 0.0;
    // Objective function and sensitivity analyses
    for (int ely = 1;ely <= nely;ely ++){
        for (int elx = 1;elx <= nelx;elx ++){
            int n1 = (nely+1)*(elx-1)+ely;
            int n2 = (nely+1)*elx+ely;
            Ue[0] = U[2*n1-1-1];
            Ue[1] = U[2*n1  -1];
            Ue[2] = U[2*n2-1-1];
            Ue[3] = U[2*n2  -1];
            Ue[4] = U[2*n2+1-1];
            Ue[5] = U[2*n2+2-1];
            Ue[6] = U[2*n1+1-1];
            Ue[7] = U[2*n1+2-1];
            tfloat transposeUeKEUe = KE.vTransposeMultMMultV(Ue);
            compliance += interpolate(elx,ely)*transposeUeKEUe;
            dc->set(ely-1, elx-1, -interpolateDiff(elx, ely)*transposeUeKEUe);
        }
    }
    return compliance;
}


void TopOpt::meshIndependencyFilter(){
    dcNew->fill(0.0);
    int roundedRMin = (int)(rmin);
    for (int i=1;i<=nelx;i++){
        for (int j = 1;j<=nely;j++){
            tfloat sum = 0;
            for (int k = max(i-roundedRMin,1);k<=min(i+roundedRMin, nelx);k++){
                for (int l = max(j-roundedRMin,1);l<=min(j+roundedRMin,nely);l++){
                    tfloat fac = rmin-sqrt((i-k)*(i-k)+(j-l)*(j-l));
                    sum = sum + max(0.0,fac);
                    tfloat newDcValue = dcNew->get(j-1,i-1) + max(0.0,fac)*x->get(l-1,k-1)*dc->get(l-1,k-1);
                    dcNew->set(j-1,i-1, newDcValue);
                }
            }
            dcNew->set(j-1,i-1, dcNew->get(j-1,i-1) / (x->get(j-1,i-1)*sum));
        }
    }
    swap(dc,dcNew); // dcNew is now 'assigned' to dc
}

void TopOpt::defineLoads(){
    F[1] = -1; // add load in upper left corner
}

void TopOpt::initFreeDofs(){
    // define supports (half MBB-beam)
    for (int i=0;i<2*(nely+1);i+=2){ // add supports in left 
        fixeddofs.push_back(i);
    }
    fixeddofs.push_back(2*(nely+1)*(nelx+1)-1);
    vector<int> alldofs;
    
    for (int i=0;i<2*(nely+1)*(nelx+1);i++){
        alldofs.push_back(i);
    }
    
    freedofs = setDiff(alldofs, fixeddofs);
}

void TopOpt::optimalityCriteriaBasedOptimization() {
    tfloat l1 = 0;
    tfloat l2 = 100000;
    tfloat move = 0.2;
    while (l2-l1 > 1e-4) {
        tfloat lmid = 0.5*(l1+l2);
        tfloat invLmid = 1.0/lmid;
        for (int i=0; i < x->getDataCount(); i++) {
            tfloat xVal = (*x)[i];
            (*xNew)[i] = max(0.001,max(xVal-move,min(1.,min(xVal+move, xVal*sqrt(-(*dc)[i]*invLmid)))));
        }
        vector<int>::iterator iter = passiveNoMaterial.begin();
        for (;iter != passiveNoMaterial.end();iter++) {
            int index = *iter;
            (*xNew)[index] = 0.001;
        }
        if (xNew->sum() - volfrac*nelx*nely > 0) {
            l1 = lmid;
        } else {
            l2 = lmid;
        }   
    }
    swap(x,xNew); // xNew is now 'assigned' to x
}

// Set element stiffness matrix
void TopOpt::initKE() {
    int kIndexArray[] = {
                  0,1,2,3,4,5,6,7,
                  1,0,7,6,5,4,3,2,
                  2,7,0,5,6,3,4,1,
                  3,6,5,0,7,2,1,4,
                  4,5,6,7,0,1,2,3,
                  5,4,3,2,1,0,7,6,
                  6,3,4,1,2,7,0,5,
                  7,2,1,4,3,6,5,0
    };
    tfloat E = 1;
    tfloat nu = 0.3;
    tfloat k[] = {
        1.f/2.f-nu/6,
        1.f/8.f+nu/8,
        -1.f/4.f-nu/12,
        -1.f/8.f+3*nu/8,
        -1.f/4.f+nu/12,
        -1.f/8.f-nu/8,
        nu/6,
        1.f/8.f-3*nu/8,
    };
    for (int y=0; y < 8; y++) {
        for (int x=0; x < 8; x++) {
            int kIndex = kIndexArray[x*8+y];
            tfloat value = E/(1-(nu*nu))*k[kIndex];
            KE.set(y,x,value);
        }
    }
}

vector<int> TopOpt::setDiff(std::vector<int> &set1, std::vector<int> &set2) {
    // assumes that set1 and set2 are sorted in increasing order
    vector<int> res;
    vector<int>::iterator set1Iterator = set1.begin();
    vector<int>::iterator set2Iterator = set2.begin();
    while (set1Iterator != set1.end() && set2Iterator != set2.end()) {
        if (set1Iterator == set1.end()) {
            res.push_back(*set2Iterator);
            set2Iterator++;
        } else if (set2Iterator == set2.end()) {
            res.push_back(*set1Iterator);
            set1Iterator++;
        } else if (*set1Iterator == *set2Iterator) {
            set1Iterator++;
            set2Iterator++;
        } else if (*set1Iterator < *set2Iterator) {
            res.push_back(*set1Iterator);
            set1Iterator++;
        } else { // (*set1Iterator > *set2Iterator)
            res.push_back(*set2Iterator);
            set2Iterator++;
        }
    }
    return res;
}

void printData(tfloat *data, int size) {
    for (int i=0;i<size;i++){
        printf("%10.4f ",data[i]);
    }
    printf("\n");
}

void printVector(vector<int> &v) {
    for (int i=0;i<v.size();i++){
        printf("%i ",v[i]);
    }
    printf("\n");
}


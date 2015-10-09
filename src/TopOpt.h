#pragma once

#include <vector>
#include <set>
#include "Matrix.h"
#include "MatrixBand.h"

#ifndef tfloat
#define tfloat double
#endif

using std::vector;

class TopOpt {
public:
    // nelx, nely = number of elements in x and y direction 
    // volfract = volume fraction
    // penal = penalization value
    // rmin = filter radius
    TopOpt(int nelx, int nely, tfloat volfrac, tfloat penal, tfloat rmin, tfloat maxChange = 0.01);
	~TopOpt();

	bool step(float& compliance, float& volume, float& change);
	const Matrix& getMatrix()const{ return *x; }

private:
	int mIter;
	tfloat mMaxChange;
	tfloat mChange;
	Matrix xold;
	tfloat mCurCompliance;
	tfloat mCurVolume;
private:
    void FEAnalysis();
    void initKE();
    void meshIndependencyFilter(); // Also known as check
    void optimalityCriteriaBasedOptimization();  // Also known as OC
    vector<int> setDiff(vector<int> &set1, vector<int> &set2);
    void initFreeDofs();
    void defineLoads();
    void calculatePassive();
    void exportResultMatlab(Matrix *x, int iteration);
	void exportResultTxt(Matrix *x, int iteration);
private:

    tfloat objectiveFunctionAndSensitivityAnalyses();
    tfloat interpolate(int x, int y);
    tfloat interpolateDiff(int x, int y);
    int nelx; // number of element along x-axis
    int nely; // number of element along y-axis
    tfloat volfrac; // volume fraction
    tfloat penal;
    tfloat rmin;
    Matrix *x;
    Matrix *xNew;
    tfloat *U; // Displacement vectors
    Matrix KE; // stiffness matrix
    Matrix *dc;
    Matrix *dcNew;
    MatrixBand K;
    vector<int> passiveNoMaterial;
    vector<int> fixeddofs;
    vector<int> freedofs;
    tfloat *F;
};
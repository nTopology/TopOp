#pragma once

#include <vector>
#include <memory>
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
	TopOpt(int nelx, int nely,
		tfloat volfrac, tfloat penal,
		tfloat rmin, tfloat maxChange = 0.01);
	~TopOpt();

	bool step(tfloat& compliance, tfloat& volume, tfloat& change, bool verbose = false);
	const Matrix& getX()const { return *x; }
	void setPassiveRadius(tfloat r) { mPassiveRadius = r; }

	void exportResults()const;
private:
	int mIter;
	tfloat mMaxChange;
	tfloat mChange;
	Matrix xold;
	tfloat mCurCompliance;
	tfloat mCurVolume;
	tfloat mPassiveRadius;
private:
	void FEAnalysis();
	void initKE();
	void meshIndependencyFilter(); // Also known as check
	void optimalityCriteriaBasedOptimization();  // Also known as OC
	vector<int> setDiff(vector<int> &set1, vector<int> &set2);
	void initFreeDofs();
	void defineLoads();
	void calculatePassive();
	void exportResultMatlab(Matrix *x, int iteration)const;
	void exportResultTxt(Matrix *x, int iteration)const;
private:

	tfloat objectiveFunctionAndSensitivityAnalyses();
	tfloat interpolate(int x, int y);
	tfloat interpolateDiff(int x, int y);
	int nelx; // number of element along x-axis
	int nely; // number of element along y-axis
	tfloat volfrac; // volume fraction
	tfloat penal;
	tfloat rmin;
	std::shared_ptr<Matrix> x;
	std::shared_ptr<Matrix> xNew;

	std::shared_ptr<Matrix> dc;
	std::shared_ptr<Matrix> dcNew;

	std::vector<tfloat> U; // Displacement vectors
	std::vector<tfloat> F; // Displacement vectors

	Matrix KE; // stiffness matrix

	MatrixBand K;
	vector<int> passiveNoMaterial;
	vector<int> fixeddofs;
	vector<int> freedofs;
};
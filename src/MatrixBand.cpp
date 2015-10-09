#include "MatrixBand.h"
#include <iostream>
#include <cassert>
#include <cmath>

#define K(i,j) data[(i)+(j)*bandwidth]

MatrixBand::MatrixBand(int size, int bandwidth)
:size(size), bandwidth(bandwidth), dataCount(size*bandwidth)
{
    data = new tfloat[dataCount];
    memset(data, 0, sizeof(tfloat)*dataCount);
}

MatrixBand::MatrixBand(const MatrixBand& m)
:size(m.size),bandwidth(m.bandwidth),dataCount(m.dataCount){
    data = new tfloat[dataCount];
    set(m);
}

MatrixBand::~MatrixBand(){
    delete data;
}


void MatrixBand::set(const MatrixBand& m){
    assert(m.size == size);
    assert(m.bandwidth == bandwidth);
    memcpy(data, m.data, sizeof(tfloat)*dataCount);
}

void MatrixBand::add(int i, int j, tfloat value){
    set(i,j,value+get(i,j));
}


void MatrixBand::set(int i, int j, tfloat value){
    int index = getIndex(i, j);
    assert(index >= 0);
    data[index] = value;
}

tfloat MatrixBand::get(int i, int j) const {
    int index = getIndex(i, j);
    if (index < 0){
        return 0;
    }
    return data[index];    
}

void MatrixBand::fill(tfloat value){
    for (int i=0;i<dataCount;i++){
        data[i] = value;
    }
}

void MatrixBand::print() {
    for (int i=0;i<size;i++){
        for (int j=0;j<size;j++){
            printf("%10.4f ",get(i,j));
        }
        printf("\n");
    }
}

void MatrixBand::printRaw(){
    for (int i=0;i<dataCount;i++){
        printf("%10.4f ",data[i]);
    }
    printf("\n");
}

void MatrixBand::setBandDataColumnOrder(tfloat *d){
    int index = 0;
    for (int i=0;i<bandwidth;i++){
        for (int j=0;j<size;j++){
            if (i+j<size){
                set(i+j,j, d[index]);
            }
            index ++;
        }
    }
}

void MatrixBand::setDataColumnOrder(tfloat *d){
    for (int i=0;i<size*size;i++){
        int row = i/size;
        int column = i%size;
        int horizontalDistanceFromDiagonal = row-column;
        if (abs(horizontalDistanceFromDiagonal) < bandwidth){
            set(row,column,d[i]);
        }
    }
}

void MatrixBand::fixedDofs(std::vector<int> &fixeddofs){
    for (std::vector<int>::iterator iter = fixeddofs.begin();iter != fixeddofs.end();iter++){
        int j = *iter;
        set(j,j, 1);
        for (int i=1;i<bandwidth;i++){
            if (i+j<size){
                set(j+i,j, 0);
            }
            if (j-i>=0){
                set(j-i,j, 0);
            }
        }
    }
}

bool MatrixBand::solve(tfloat *b, tfloat *x, std::vector<int> fixeddofs, bool destroyMatrixAndB){
    fixedDofs(fixeddofs);
    //print();
    return solve(b,x, destroyMatrixAndB);
}

tfloat MatrixBand::sum(){
    tfloat sum = 0;
    for (int i=0;i<dataCount;i++){
        tfloat factor = 2;
        if (i%bandwidth==0){
            factor = 1;
        }
        sum += factor*data[i];
    }
    return sum;
}

void MatrixBand::CholeskyFactorization(){
    for (int s = 0; s < size; s++) {
        for (int i = 1;i<bandwidth;i++){
            if (K(i, s) == 0) continue;
            int t = s+i;
            tfloat a = K(i, s) / K(0, s);
            int m = 0;
            for (int j = i; j < bandwidth; j++){
                K(m,t) = K(m,t) - a*K(j,s);
                m++;
            }
            K(i,s) = a;
        }
    }
}

void MatrixBand::forwardSubstitution(tfloat *x){
    for (int s = 0; s < size; s++){
        for (int i = 1; i < bandwidth; i++){
            if (K(i,s) == 0) continue;
            int t = s+i;
            x[t] = x[t] - K(i,s) * x[s];
        }
        x[s] = x[s] / K(0,s);
    }
}

void MatrixBand::backwardSubstitution(tfloat *x){
    for (int j = 1; j < size; j++) {
        int s = size - j -1;
        for (int i = 1; i < bandwidth; i++) {
            if (K(i,s) == 0) continue;
            int t = s + i;
            x[s] = x[s] - K(i,s) * x[t];
        }
    }
}

// Solve Ab=x
bool MatrixBand::solve(tfloat *b, tfloat *x, bool destroyMatrixAndB){
    tfloat *tmp = NULL;
    if (!destroyMatrixAndB){
        // Create temporary matrix and right-hand-side and use it as main matrix
        tmp = new tfloat[dataCount];
        memcpy(tmp, data, sizeof(tfloat)*dataCount);
        std::swap(tmp,data); 
    }
    // copy b to x
    memcpy(x, b, sizeof(tfloat)*size);
    
    CholeskyFactorization();
    
    forwardSubstitution(x);
    backwardSubstitution(x);
    
    if (!destroyMatrixAndB){
        // use main matrix and delete temporary matrix
        std::swap(data,tmp);
        delete tmp;
    }
    return true;
}

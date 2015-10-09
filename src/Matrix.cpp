#include "Matrix.h"
//#include "mkl.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>

Matrix::Matrix(int rows, int columns)
:rows(rows), columns(columns),dataCount(rows*columns){
    data = new tfloat[dataCount];
    memset(data, 0, sizeof(tfloat)*dataCount);
}

Matrix::Matrix(const Matrix& m)
:rows(m.rows),columns(m.columns),dataCount(m.dataCount){
    data = new tfloat[dataCount];
    set(m);
}

Matrix::~Matrix(){
    delete data;
}

void Matrix::add(int i, int j, tfloat value){
    int index = getIndex(i,j);
    data[index] += value;
}

/// M_ij is the i-th row and the j-th column
void Matrix::set(int i, int j, tfloat value){
    int index = getIndex(i,j);
    data[index] = value;
}

/// M_ij is the i-th row and the j-th column
tfloat Matrix::get(int i, int j) const{
    int index = getIndex(i,j);
    return data[index];
}

void Matrix::fixedDofs(std::vector<int> &fixeddofs){
    for (int i=0;i<fixeddofs.size();i++){
        int fixeddof = fixeddofs[i];
        for (int i=0;i<rows;i++){
            set(i, fixeddof, i==fixeddof?1:0);
        }
        for (int j=0;j<columns;j++){
            set(fixeddof, j, j==fixeddof?1:0);
        }
    }
}

void Matrix::print(){
    printf("%% Matrix %i rows x %i columns\n", rows, columns);
    for (int i=0;i<rows;i++){
        for (int j=0;j<columns;j++){
            printf("%10.4f ",get(i,j));
        }
        printf("\n");
    }
}

void Matrix::multiply(const Matrix &m, Matrix *dest){
//#if true
     //  cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, rows, m.columns, columns, 1.0, data, rows, m.data, m.rows, 0.0, dest->data, dest->rows);
// #else
//         cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, rows, m.columns, columns, 1.0, data, rows, m.data, m.rows, 0.0, dest->data, dest->rows);
// #endif
  
    for (int i=0;i<dest->rows;i++){
        for (int j=0;j<dest->columns;j++){
            tfloat product = 0;
            for (int n=0;n<columns;n++){
                product += get(i,n) * m.get(n, j); 
            }
            dest->set(i,j,product);
        }
    }
  
}

Matrix Matrix::multiply(const Matrix &m){
    Matrix res(rows,m.columns);
    multiply(m, &res);
    return res;
}

void Matrix::fill(tfloat value){
    for (int i=0;i<dataCount;i++){
        data[i] = value;
    }
}

void Matrix::set(const Matrix& m){
    assert(m.columns == columns);
    assert(m.rows == rows);
    memcpy(data, m.data, sizeof(tfloat)*dataCount);
}

tfloat Matrix::maxDiff(const Matrix &m){
    assert(m.columns == columns);
    assert(m.rows == rows);
    tfloat res = -1;

    for (int i=0;i<dataCount;i++){
        tfloat delta = fabs(data[i]-m.data[i]);
        res = std::max(res, delta);
    }
    return res;
}

tfloat Matrix::sum(){
    tfloat sum = 0;
    for (int i=0;i<dataCount;i++){
        sum += data[i];
    }
    return sum;
}

tfloat& Matrix::operator[] (const int nIndex){
    return data[nIndex];
}

tfloat Matrix::vTransposeMultMMultV(tfloat *v){
    assert(rows == columns);
    tfloat sum = 0;
    for (int i=0;i<rows;i++){
        tfloat vi = v[i];
        for (int j=0;j<rows;j++){
            sum += vi*get(i, j)*v[j];
        }
    }
    return sum;
}

bool Matrix::solve(tfloat const * const b, tfloat *x){
    assert(rows == columns);
    return solveGaussianElimination(b, x);
}

bool Matrix::solve(tfloat const * const b, tfloat *x, std::vector<int> &fixeddofs){
    fixedDofs(fixeddofs);
    return solve(b,x);
}

void Matrix::setDataColumnOrder(tfloat *d){
    for (int i=0;i<rows*columns;i++){
        int row = i/columns;
        int column = i%columns;
        set(row,column,d[i]);
    }
}

bool Matrix::solveGaussianElimination(tfloat const * const b, tfloat *x) {
    // Based on 6.1.1 Gaussian Elimination [page 35]

    // Create temporary matrix and right-hand-side and use it as main matrix
    tfloat *tmp = new tfloat[dataCount];
    memcpy(tmp, data, sizeof(tfloat)*dataCount);
    std::swap(tmp,data); 
    // create temporary right-hand-side
    tfloat *B = new tfloat[columns];
    memcpy(B, b, sizeof(tfloat)*columns);
    
    // reduce to upper triangular form 
    for (int s=0;s<columns-1;s++){
        for (int i=s+1;i<rows;i++){
            if (get(s,s) == 0){
                return false; // currently swapping row is not implemented - the matrix may be singular
            }
            float a = get(i,s)/get(s,s); //pivot value
            // eliminate unknows
            B[i] = B[i] - a * B[s];
            for (int j=s+1;j<columns;j++){
                set(i,j,get(i,j)-a*get(s,j));
            }
        }
    }

    // backward substitution
    x[columns-1] = B[columns - 1] / get(rows - 1, columns - 1);
    for (int i = columns - 2; i >= 0; i--){
        float a = B[i];
        for (int j = i + 1; j < rows; j++){
            a = a - get(i,j) * x[j];
        }
        x[i] = a / get(i,i);
    }
    
    // use main matrix and delete temporary matrix
    std::swap(data,tmp);
    delete tmp;
    delete B;
    return true;
}

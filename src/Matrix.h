#pragma once

#include <vector>
#include <cassert>

#ifndef tfloat
#define tfloat double
#endif

/// Column-major order storage of matrix 
/// Example m x n (2 x 3)
///  1 2 3
///  4 5 6
/// is stored [1 4 2 5 3 6]
///
/// M_ij is the i-th row and the j-th column
class Matrix {
public:
    Matrix(int rows, int columns);
    Matrix(const Matrix& m);
    ~Matrix();
    

    void set(const Matrix& m);
    void add(int i, int j, tfloat value);
    void set(int i, int j, tfloat value);
    tfloat get(int i, int j) const;
    void fixedDofs(std::vector<int> &fixeddofs);
    
    tfloat maxDiff(const Matrix &m);
    tfloat sum();

    void print();
    void multiply(const Matrix &m, Matrix *dest);
    Matrix multiply(const Matrix &m);
    
    tfloat& operator[] (const int nIndex);
    
    void fill(tfloat value);
    
    int getDataCount(){ return dataCount; }
    int getRows()const{ return rows; }
    int getColumns()const{ return columns; }
    
    tfloat vTransposeMultMMultV(tfloat *v);
    // Solve Ab=x
    bool solve(tfloat const * const b, tfloat *x);
    bool solve(tfloat const * const b, tfloat *x, std::vector<int> &freedofs);
    
    void setDataColumnOrder(tfloat *d);
    bool isBanded(){ return false; }
    inline int getIndex(int i, int j) const {
        assert(i >= 0);
        assert(j >= 0);
        assert(i < rows);
        assert(j < columns);
        return i+j*rows;
    }
private:
    tfloat *data;
    int rows;
    int columns;
    int dataCount;    
    bool solveGaussianElimination(tfloat const * const b, tfloat *x);
};

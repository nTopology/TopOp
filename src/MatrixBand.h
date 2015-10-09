#pragma once
#include <vector>
#include <cassert>

#ifndef tfloat
#define tfloat double
#endif

/// Column-major order storage of band-matrix 
/// Example m x n (3 x 3 - bandwidth 2)
///  1 4 0
///  4 5 8
///  0 8 9 
/// is stored as banded  
///  1 5 9 
///  4 8 0
/// is stored in array
///  [1 4 5 8 9 0]
/// M_ij is the i-th row and the j-th column
class MatrixBand {
public:
    MatrixBand(int size, int bandwidth);
    MatrixBand(const MatrixBand& m);
    ~MatrixBand();
    
    
    void set(const MatrixBand& m);
    void add(int i, int j, tfloat value);
    void set(int i, int j, tfloat value);
    tfloat get(int i, int j) const;
    
    void fixedDofs(std::vector<int> &fixeddofs);
    void fill(tfloat value);
    
    void print();
    void printRaw();
        
    int getDataCount(){ return dataCount; }
    int getSize(){ return size; }
    int getRows() { return size; }
    tfloat sum();
    int getColumns() { return size; }
    int getBandwidth(){ return bandwidth; }
    void setDataColumnOrder(tfloat *d);
    void setBandDataColumnOrder(tfloat *d);
    
    // Solve Ab=x
    bool solve(tfloat *b, tfloat *x, bool destroyMatrixAndB = true);
    bool solve(tfloat *b, tfloat *x, std::vector<int> fixeddofs, bool destroyMatrixAndB = true);
    
    tfloat *getDataPointer() { return data; }
    bool isBanded(){ return true; }
    inline int getIndex(int i, int j) const {
        assert(i >= 0 && i < size);
        assert(j >= 0 && j < size);
        bool upperTriangularIndex = j > i;
        int horizontalDistanceFromDiagonal = i-j;
        if (upperTriangularIndex){ // in 
            i -= horizontalDistanceFromDiagonal;
            j += horizontalDistanceFromDiagonal;
            horizontalDistanceFromDiagonal = -horizontalDistanceFromDiagonal;
        }
        if (horizontalDistanceFromDiagonal >= bandwidth) {
            return -1; // not in band return -1 to indicate 0 value
        }
        
        int index = horizontalDistanceFromDiagonal+j*bandwidth;
        return index;
    }
private:
    void CholeskyFactorization();
    void forwardSubstitution(tfloat *x);
    void backwardSubstitution(tfloat *x);
    tfloat *data;
    int size;
    int bandwidth;
    int dataCount;    
};
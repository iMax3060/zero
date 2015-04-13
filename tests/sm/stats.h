#ifndef STATS_H
#define STATS_H

#include "w_defines.h"

#define SM_SOURCE
#include "sm_int_1.h"

#include <boost/random.hpp>
#include <vector>
#include "logtype_gen.h"

class Stats {
public:
    //TODO: make nexType() and nextLength() return the appropriate types. */
    Stats();
    unsigned nextType();
    unsigned nextLength(unsigned _type);

protected:
    Stats(unsigned _lengthDim);
    
    /* probType[i] = probability of type i */
    vector<float> probType;
    
    /* probLengthIndex[i][j] = probability of length with index j for type i */
    vector<vector<float> > probLengthIndex;

    /* lengthIndexToLength[i][j] = translate length index j of type i to actual length */
    vector<vector<unsigned> > lengthIndexToLength;

private:
    boost::random::mt19937 gen; /* Random Number Generator */
    boost::random::discrete_distribution<> typeDist;
};

#endif
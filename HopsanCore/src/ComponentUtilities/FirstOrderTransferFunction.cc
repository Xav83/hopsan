/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   FirstOrderTransferFunction.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-23
//!
//! @brief Contains the Core First Order Transfer Function class
//!
//$Id$

//#include <iostream>
//#include <cassert>
#include <algorithm>
#include "ComponentUtilities/FirstOrderTransferFunction.h"

using namespace hopsan;

//! @class hopsan::FirstOrderTransferFunction
//! @ingroup ComponentUtilityClasses
//! @brief The FirstOrderTransferFunction class implements a first order time discrete transfer function using bilinear transform
//!
//! To declare a filter like \f[G=\frac{a_1 s + a_0}{b_1 s + b_0}\f]
//! the syntax is filter.setNumDen(num, den)
//! where \f$num[0]=a_0\f$, \f$num[1]=a_1\f$
//! and \f$den[0]=b_0\f$, \f$den[1]=b_1\f$
//!


void FirstOrderTransferFunction::initialize(double timestep, double num[2], double den[2], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mValue = y0;
    mDelayU = u0;
    mDelayY = std::max(std::min(y0, mMax), mMin);
    mTimeStep = timestep;
    setNumDen(num, den);
}


void FirstOrderTransferFunction::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void FirstOrderTransferFunction::setNum(double num[2])
{
    mCoeffU[0] = num[0]*mTimeStep-2.0*num[1];
    mCoeffU[1] = num[0]*mTimeStep+2.0*num[1];
}


void FirstOrderTransferFunction::setDen(double den[2])
{
    mCoeffY[0] = den[0]*mTimeStep-2.0*den[1];
    mCoeffY[1] = den[0]*mTimeStep+2.0*den[1];
}


void FirstOrderTransferFunction::setNumDen(double num[2], double den[2])
{
    mCoeffU[0] = num[0]*mTimeStep-2.0*num[1];
    mCoeffU[1] = num[0]*mTimeStep+2.0*num[1];

    mCoeffY[0] = den[0]*mTimeStep-2.0*den[1];
    mCoeffY[1] = den[0]*mTimeStep+2.0*den[1];
}


void FirstOrderTransferFunction::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
    mValue = y0;
}


double FirstOrderTransferFunction::update(double u)
{
    //Filter equation
    //Bilinear transform is used

    mValue = 1.0/mCoeffY[1]*(mCoeffU[1]*u + mCoeffU[0]*mDelayU - mCoeffY[0]*mDelayY);

    if (mValue > mMax)
    {
        mDelayY = mMax;
        mDelayU = mMax;
        mValue = mMax;
    }
    else if (mValue < mMin)
    {
        mDelayY = mMin;
        mDelayU = mMin;
        mValue = mMin;
    }
    else
    {
        mDelayY = mValue;
        mDelayU = u;
    }

    return mValue;
}


//! Read current filter output value
//! @return The filtered actual value.
double FirstOrderTransferFunction::value()
{
    return mValue;
}








void FirstOrderTransferFunctionVariable::initialize(double *pTimestep, double num[2], double den[2], double u0, double y0, double min, double max)
{
    mMin = min;
    mMax = max;
    mValue = y0;
    mDelayU = u0;
    mDelayY = std::max(std::min(y0, mMax), mMin);
    mpTimeStep = pTimestep;
    mPrevTimeStep = *pTimestep;
    mNum[0] = num[0];
    mNum[1] = num[1];
    mDen[0] = den[0];
    mDen[1] = den[1];

    recalculateCoefficients();
}


void FirstOrderTransferFunctionVariable::setMinMax(double min, double max)
{
    mMin = min;
    mMax = max;
}


void FirstOrderTransferFunctionVariable::setNum(double num[2])
{
    mNum[0] = num[0];
    mNum[1] = num[1];
    recalculateCoefficients();
}


void FirstOrderTransferFunctionVariable::setDen(double den[2])
{
    mDen[0] = den[0];
    mDen[1] = den[1];
    recalculateCoefficients();
}


void FirstOrderTransferFunctionVariable::setNumDen(double num[2], double den[2])
{
    mNum[0] = num[0];
    mNum[1] = num[1];
    mDen[0] = den[0];
    mDen[1] = den[1];
    recalculateCoefficients();
}

void FirstOrderTransferFunctionVariable::recalculateCoefficients()
{
    mCoeffU[0] = mNum[0]*(*mpTimeStep)-2.0*mNum[1];
    mCoeffU[1] = mNum[0]*(*mpTimeStep)+2.0*mNum[1];

    mCoeffY[0] = mDen[0]*(*mpTimeStep)-2.0*mDen[1];
    mCoeffY[1] = mDen[0]*(*mpTimeStep)+2.0*mDen[1];
}


void FirstOrderTransferFunctionVariable::initializeValues(double u0, double y0)
{
    mDelayU = u0;
    mDelayY = y0;
    mValue = y0;
}


double FirstOrderTransferFunctionVariable::update(double u)
{
    //Filter equation
    //Bilinear transform is used

    if((*mpTimeStep) != mPrevTimeStep)
    {
        mPrevTimeStep = (*mpTimeStep);
        recalculateCoefficients();
    }

    mValue = 1.0/mCoeffY[1]*(mCoeffU[1]*u + mCoeffU[0]*mDelayU - mCoeffY[0]*mDelayY);

    if (mValue > mMax)
    {
        mDelayY = mMax;
        mDelayU = mMax;
        mValue = mMax;
    }
    else if (mValue < mMin)
    {
        mDelayY = mMin;
        mDelayU = mMin;
        mValue = mMin;
    }
    else
    {
        mDelayY = mValue;
        mDelayU = u;
    }

    return mValue;
}


//! Read current filter output value
//! @return The filtered actual value.
double FirstOrderTransferFunctionVariable::value()
{
    return mValue;
}
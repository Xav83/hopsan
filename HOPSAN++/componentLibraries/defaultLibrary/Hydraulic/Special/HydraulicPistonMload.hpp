#ifndef HYDRAULICPISTONMLOAD_HPP_INCLUDED
#define HYDRAULICPISTONMLOAD_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file HydraulicPistonMload.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Wed 29 May 2013 13:43:39
//! @brief This is piston with an inertia load
//! @ingroup HydraulicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, HOPSAN++, CompgenModels}/HydraulicComponents.nb*/

using namespace hopsan;

class HydraulicPistonMload : public ComponentQ
{
private:
     double A1;
     double A2;
     double SL;
     double Cip;
     double Bp;
     double ML;
     double BL;
     double xmin;
     double xmax;
     Port *mpP1;
     Port *mpP2;
     Port *mpPmp;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     double delayParts6[9];
     double delayParts7[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[7];
     int mNstep;
     //Port P1 variable
     double p1;
     double q1;
     double T1;
     double dE1;
     double c1;
     double Zc1;
     //Port P2 variable
     double p2;
     double q2;
     double T2;
     double dE2;
     double c2;
     double Zc2;
     //Port Pmp variable
     double fmp;
     double xmp;
     double vmp;
     double cmp;
     double Zcmp;
     double eqMassmp;
//==This code has been autogenerated using Compgen==
     //inputVariables
     //outputVariables
     //Port P1 pointer
     double *mpND_p1;
     double *mpND_q1;
     double *mpND_T1;
     double *mpND_dE1;
     double *mpND_c1;
     double *mpND_Zc1;
     //Port P2 pointer
     double *mpND_p2;
     double *mpND_q2;
     double *mpND_T2;
     double *mpND_dE2;
     double *mpND_c2;
     double *mpND_Zc2;
     //Port Pmp pointer
     double *mpND_fmp;
     double *mpND_xmp;
     double *mpND_vmp;
     double *mpND_cmp;
     double *mpND_Zcmp;
     double *mpND_eqMassmp;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     //inputParameters pointers
     double *mpA1;
     double *mpA2;
     double *mpSL;
     double *mpCip;
     double *mpBp;
     double *mpML;
     double *mpBL;
     double *mpxmin;
     double *mpxmax;
     //outputVariables pointers
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart12;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     Delay mDelayedPart30;
     Delay mDelayedPart40;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new HydraulicPistonMload();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(7,7);
        systemEquations.create(7);
        delayedPart.create(8,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpP1=addPowerPort("P1","NodeHydraulic");
        mpP2=addPowerPort("P2","NodeHydraulic");
        mpPmp=addPowerPort("Pmp","NodeMechanic");
        //Add inputVariables to the component

        //Add inputParammeters to the component
            addInputVariable("A1", "&mpPiston area 1", "m2", 0.001,&mpA1);
            addInputVariable("A2", "&mpPiston area 2", "m2", 0.001,&mpA2);
            addInputVariable("SL", "&mpStroke", "m", 0.5,&mpSL);
            addInputVariable("Cip", "&mpLeak coeff.", "m3/(s Pa)", \
0.,&mpCip);
            addInputVariable("Bp", "&mpVisc. friction coeff.", "N/m/s", \
0.,&mpBp);
            addInputVariable("ML", "&mpInertia", "kg", 1000.,&mpML);
            addInputVariable("BL", "&mpViscous friction coefficient of load", \
"Ns/m", 0.,&mpBL);
            addInputVariable("xmin", "&mpLimitation on stroke", "m", \
0.,&mpxmin);
            addInputVariable("xmax", "&mpLimitation on stroke", "m", \
0.5,&mpxmax);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,7);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port P1
        mpND_p1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Pressure);
        mpND_q1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Flow);
        mpND_T1=getSafeNodeDataPtr(mpP1, NodeHydraulic::Temperature);
        mpND_dE1=getSafeNodeDataPtr(mpP1, NodeHydraulic::HeatFlow);
        mpND_c1=getSafeNodeDataPtr(mpP1, NodeHydraulic::WaveVariable);
        mpND_Zc1=getSafeNodeDataPtr(mpP1, NodeHydraulic::CharImpedance);
        //Port P2
        mpND_p2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Pressure);
        mpND_q2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Flow);
        mpND_T2=getSafeNodeDataPtr(mpP2, NodeHydraulic::Temperature);
        mpND_dE2=getSafeNodeDataPtr(mpP2, NodeHydraulic::HeatFlow);
        mpND_c2=getSafeNodeDataPtr(mpP2, NodeHydraulic::WaveVariable);
        mpND_Zc2=getSafeNodeDataPtr(mpP2, NodeHydraulic::CharImpedance);
        //Port Pmp
        mpND_fmp=getSafeNodeDataPtr(mpPmp, NodeMechanic::Force);
        mpND_xmp=getSafeNodeDataPtr(mpPmp, NodeMechanic::Position);
        mpND_vmp=getSafeNodeDataPtr(mpPmp, NodeMechanic::Velocity);
        mpND_cmp=getSafeNodeDataPtr(mpPmp, NodeMechanic::WaveVariable);
        mpND_Zcmp=getSafeNodeDataPtr(mpPmp, NodeMechanic::CharImpedance);
        mpND_eqMassmp=getSafeNodeDataPtr(mpPmp, \
NodeMechanic::EquivalentMass);

        //Read variables from nodes
        //Port P1
        p1 = (*mpND_p1);
        q1 = (*mpND_q1);
        T1 = (*mpND_T1);
        dE1 = (*mpND_dE1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);
        //Port P2
        p2 = (*mpND_p2);
        q2 = (*mpND_q2);
        T2 = (*mpND_T2);
        dE2 = (*mpND_dE2);
        c2 = (*mpND_c2);
        Zc2 = (*mpND_Zc2);
        //Port Pmp
        fmp = (*mpND_fmp);
        xmp = (*mpND_xmp);
        vmp = (*mpND_vmp);
        cmp = (*mpND_cmp);
        Zcmp = (*mpND_Zcmp);
        eqMassmp = (*mpND_eqMassmp);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        A1 = (*mpA1);
        A2 = (*mpA2);
        SL = (*mpSL);
        Cip = (*mpCip);
        Bp = (*mpBp);
        ML = (*mpML);
        BL = (*mpBL);
        xmin = (*mpxmin);
        xmax = (*mpxmax);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (-2*A1*c1*Power(mTimestep,2) + \
2*A2*c2*Power(mTimestep,2) + 2*cmp*Power(mTimestep,2) + \
2*BL*Power(mTimestep,2)*vmp + 2*Bp*Power(mTimestep,2)*vmp - 8*ML*xmp - \
2*A1*Power(mTimestep,2)*q1*Zc1 + 2*A2*Power(mTimestep,2)*q2*Zc2 + \
2*Power(mTimestep,2)*vmp*Zcmp)/(4.*ML);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts1[2] = (-(A1*c1*Power(mTimestep,2)) + \
A2*c2*Power(mTimestep,2) + cmp*Power(mTimestep,2) + BL*Power(mTimestep,2)*vmp \
+ Bp*Power(mTimestep,2)*vmp + 4*ML*xmp - A1*Power(mTimestep,2)*q1*Zc1 + \
A2*Power(mTimestep,2)*q2*Zc2 + Power(mTimestep,2)*vmp*Zcmp)/(4.*ML);
        mDelayedPart12.initialize(mNstep,delayParts1[2]);
        delayParts2[1] = (-(A1*c1*mTimestep) + A2*c2*mTimestep + \
cmp*mTimestep - 2*ML*vmp + BL*mTimestep*vmp + Bp*mTimestep*vmp - \
A1*mTimestep*q1*Zc1 + A2*mTimestep*q2*Zc2 + mTimestep*vmp*Zcmp)/(2*ML + \
BL*mTimestep + Bp*mTimestep + mTimestep*Zcmp);
        mDelayedPart21.initialize(mNstep,delayParts2[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[1][2] = mDelayedPart12.getIdx(1);
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(7);
        Vec stateVark(7);
        Vec deltaStateVar(7);

        //Read variables from nodes
        //Port P1
        T1 = (*mpND_T1);
        c1 = (*mpND_c1);
        Zc1 = (*mpND_Zc1);
        //Port P2
        T2 = (*mpND_T2);
        c2 = (*mpND_c2);
        Zc2 = (*mpND_Zc2);
        //Port Pmp
        cmp = (*mpND_cmp);
        Zcmp = (*mpND_Zcmp);

        //Read inputVariables from nodes

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = xmp;
        stateVark[1] = vmp;
        stateVark[2] = q1;
        stateVark[3] = q2;
        stateVark[4] = p1;
        stateVark[5] = p2;
        stateVark[6] = fmp;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //PistonMload
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =xmp - limit(-(Power(mTimestep,2)*(cmp + BL*vmp \
+ Bp*vmp - A1*(c1 + q1*Zc1) + A2*(c2 + q2*Zc2) + vmp*Zcmp))/(4.*ML) - \
delayedPart[1][1] - delayedPart[1][2],xmin,xmax);
          systemEquations[1] =vmp - dxLimit(limit(-(Power(mTimestep,2)*(cmp + \
BL*vmp + Bp*vmp - A1*(c1 + q1*Zc1) + A2*(c2 + q2*Zc2) + vmp*Zcmp))/(4.*ML) - \
delayedPart[1][1] - \
delayedPart[1][2],xmin,xmax),xmin,xmax)*(-((mTimestep*(cmp - A1*(c1 + q1*Zc1) \
+ A2*(c2 + q2*Zc2)))/(2*ML + mTimestep*(BL + Bp + Zcmp))) - \
delayedPart[2][1]);
          systemEquations[2] =Cip*(p1 - p2) + q1 + A1*vmp;
          systemEquations[3] =Cip*(-p1 + p2) + q2 - A2*vmp;
          systemEquations[4] =p1 - lowLimit(c1 + q1*Zc1,0);
          systemEquations[5] =p2 - lowLimit(c2 + q2*Zc2,0);
          systemEquations[6] =-cmp + fmp - vmp*Zcmp;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = (Power(mTimestep,2)*(BL + Bp + \
Zcmp)*dxLimit(-(Power(mTimestep,2)*(cmp + BL*vmp + Bp*vmp - A1*(c1 + q1*Zc1) \
+ A2*(c2 + q2*Zc2) + vmp*Zcmp))/(4.*ML) - delayedPart[1][1] - \
delayedPart[1][2],xmin,xmax))/(4.*ML);
          jacobianMatrix[0][2] = \
-(A1*Power(mTimestep,2)*Zc1*dxLimit(-(Power(mTimestep,2)*(cmp + BL*vmp + \
Bp*vmp - A1*(c1 + q1*Zc1) + A2*(c2 + q2*Zc2) + vmp*Zcmp))/(4.*ML) - \
delayedPart[1][1] - delayedPart[1][2],xmin,xmax))/(4.*ML);
          jacobianMatrix[0][3] = \
(A2*Power(mTimestep,2)*Zc2*dxLimit(-(Power(mTimestep,2)*(cmp + BL*vmp + \
Bp*vmp - A1*(c1 + q1*Zc1) + A2*(c2 + q2*Zc2) + vmp*Zcmp))/(4.*ML) - \
delayedPart[1][1] - delayedPart[1][2],xmin,xmax))/(4.*ML);
          jacobianMatrix[0][4] = 0;
          jacobianMatrix[0][5] = 0;
          jacobianMatrix[0][6] = 0;
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = \
-((A1*mTimestep*Zc1*dxLimit(limit(-(Power(mTimestep,2)*(cmp + BL*vmp + Bp*vmp \
- A1*(c1 + q1*Zc1) + A2*(c2 + q2*Zc2) + vmp*Zcmp))/(4.*ML) - \
delayedPart[1][1] - delayedPart[1][2],xmin,xmax),xmin,xmax))/(2*ML + \
mTimestep*(BL + Bp + Zcmp)));
          jacobianMatrix[1][3] = \
(A2*mTimestep*Zc2*dxLimit(limit(-(Power(mTimestep,2)*(cmp + BL*vmp + Bp*vmp - \
A1*(c1 + q1*Zc1) + A2*(c2 + q2*Zc2) + vmp*Zcmp))/(4.*ML) - delayedPart[1][1] \
- delayedPart[1][2],xmin,xmax),xmin,xmax))/(2*ML + mTimestep*(BL + Bp + \
Zcmp));
          jacobianMatrix[1][4] = 0;
          jacobianMatrix[1][5] = 0;
          jacobianMatrix[1][6] = 0;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = A1;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[2][4] = Cip;
          jacobianMatrix[2][5] = -Cip;
          jacobianMatrix[2][6] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = -A2;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = -Cip;
          jacobianMatrix[3][5] = Cip;
          jacobianMatrix[3][6] = 0;
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = 0;
          jacobianMatrix[4][2] = -(Zc1*dxLowLimit(c1 + q1*Zc1,0));
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
          jacobianMatrix[4][5] = 0;
          jacobianMatrix[4][6] = 0;
          jacobianMatrix[5][0] = 0;
          jacobianMatrix[5][1] = 0;
          jacobianMatrix[5][2] = 0;
          jacobianMatrix[5][3] = -(Zc2*dxLowLimit(c2 + q2*Zc2,0));
          jacobianMatrix[5][4] = 0;
          jacobianMatrix[5][5] = 1;
          jacobianMatrix[5][6] = 0;
          jacobianMatrix[6][0] = 0;
          jacobianMatrix[6][1] = -Zcmp;
          jacobianMatrix[6][2] = 0;
          jacobianMatrix[6][3] = 0;
          jacobianMatrix[6][4] = 0;
          jacobianMatrix[6][5] = 0;
          jacobianMatrix[6][6] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          xmp=stateVark[0];
          vmp=stateVark[1];
          q1=stateVark[2];
          q2=stateVark[3];
          p1=stateVark[4];
          p2=stateVark[5];
          fmp=stateVark[6];
        }

        //Calculate the delayed parts
        delayParts1[1] = (-2*A1*c1*Power(mTimestep,2) + \
2*A2*c2*Power(mTimestep,2) + 2*cmp*Power(mTimestep,2) + \
2*BL*Power(mTimestep,2)*vmp + 2*Bp*Power(mTimestep,2)*vmp - 8*ML*xmp - \
2*A1*Power(mTimestep,2)*q1*Zc1 + 2*A2*Power(mTimestep,2)*q2*Zc2 + \
2*Power(mTimestep,2)*vmp*Zcmp)/(4.*ML);
        delayParts1[2] = (-(A1*c1*Power(mTimestep,2)) + \
A2*c2*Power(mTimestep,2) + cmp*Power(mTimestep,2) + BL*Power(mTimestep,2)*vmp \
+ Bp*Power(mTimestep,2)*vmp + 4*ML*xmp - A1*Power(mTimestep,2)*q1*Zc1 + \
A2*Power(mTimestep,2)*q2*Zc2 + Power(mTimestep,2)*vmp*Zcmp)/(4.*ML);
        delayParts2[1] = (-(A1*c1*mTimestep) + A2*c2*mTimestep + \
cmp*mTimestep - 2*ML*vmp + BL*mTimestep*vmp + Bp*mTimestep*vmp - \
A1*mTimestep*q1*Zc1 + A2*mTimestep*q2*Zc2 + mTimestep*vmp*Zcmp)/(2*ML + \
BL*mTimestep + Bp*mTimestep + mTimestep*Zcmp);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[1][2] = mDelayedPart12.getIdx(0);
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];

        //Write new values to nodes
        //Port P1
        (*mpND_p1)=p1;
        (*mpND_q1)=q1;
        (*mpND_dE1)=dE1;
        //Port P2
        (*mpND_p2)=p2;
        (*mpND_q2)=q2;
        (*mpND_dE2)=dE2;
        //Port Pmp
        (*mpND_fmp)=fmp;
        (*mpND_xmp)=xmp;
        (*mpND_vmp)=vmp;
        (*mpND_eqMassmp)=eqMassmp;
        //outputVariables

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart12.update(delayParts1[2]);
        mDelayedPart21.update(delayParts2[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // HYDRAULICPISTONMLOAD_HPP_INCLUDED

#ifndef ACTIVITYDIAGRAMFINAL_HPP_INCLUDED
#define ACTIVITYDIAGRAMFINAL_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file ActivityDiagramFinal.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor **Not yet audited by a second person**
//! @date Wed 27 Nov 2019 13:11:18
//! @brief Edge for an activity diagram
//! @ingroup ActivityDiagramComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, Users, petkr14, Dropbox, HopsanComponents, StateMachine0, \
ActivityDiagram, ActivityDiagram}/ActivityDiagramFinal.nb*/

using namespace hopsan;

class ActivityDiagramFinal : public ComponentQ
{
private:
     double diffEvent;
     Port *mpPpn1;
     int mNstep;
     //Port Ppn1 variable
     double spn1;
     double qpn1;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double event;
     //outputVariables
     //InitialExpressions variables
     double oldEvent;
     //Expressions variables
     double dEvent;
     //Port Ppn1 pointer
     double *mpP_spn1;
     double *mpP_qpn1;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpevent;
     //inputParameters pointers
     double *mpdiffEvent;
     //outputVariables pointers
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new ActivityDiagramFinal();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;

        //Add ports to the component
        mpPpn1=addPowerPort("Ppn1","NodePetriNet");
        //Add inputVariables to the component
            addInputVariable("event","event ","",1.,&mpevent);

        //Add inputParammeters to the component
            addInputVariable("diffEvent", "Trigg on level (0) or flank (1)", \
"", 0.,&mpdiffEvent);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Ppn1
        mpP_spn1=getSafeNodeDataPtr(mpPpn1, NodePetriNet::State);
        mpP_qpn1=getSafeNodeDataPtr(mpPpn1, NodePetriNet::Flow);

        //Read variables from nodes
        //Port Ppn1
        spn1 = (*mpP_spn1);
        qpn1 = (*mpP_qpn1);

        //Read inputVariables from nodes
        event = (*mpevent);

        //Read inputParameters from nodes
        diffEvent = (*mpdiffEvent);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        oldEvent = event;


        //Initialize delays


        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        //Read variables from nodes
        //Port Ppn1
        spn1 = (*mpP_spn1);

        //Read inputVariables from nodes
        event = (*mpevent);

        //Read inputParameters from nodes
        diffEvent = (*mpdiffEvent);

        //LocalExpressions

        //Expressions
        dEvent = onPositive(-0.5 + event - diffEvent*oldEvent);
        qpn1 = -(dEvent*onPositive(-0.5 + spn1));
        oldEvent = event;

        //Calculate the delayed parts


        //Write new values to nodes
        //Port Ppn1
        (*mpP_qpn1)=qpn1;
        //outputVariables

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // ACTIVITYDIAGRAMFINAL_HPP_INCLUDED

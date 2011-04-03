//!
//! @file   FileAccess.cc
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-03-20
//!
//! @brief Contains the HopsanCore hmf loader functions
//!
//$Id$

#ifndef FILEACCESS_H_INCLUDED
#define FILEACCESS_H_INCLUDED


#include <string>

#include "../win32dll.h"
#include "../ComponentEssentials.h"

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
//#include "rapidxml_print.hpp"

namespace hopsan {

    class DLLIMPORTEXPORT FileAccess
    {
    public:
        FileAccess();
        ComponentSystem* loadModel(std::string filename, double &rStartTime, double &rStopTime);

    private:
        void loadSystemContents(rapidxml::xml_node<> *pSysNode, ComponentSystem *pSystem);
        void loadComponent(rapidxml::xml_node<> *pComponentNode, ComponentSystem *pSystem);
        void loadConnection(rapidxml::xml_node<> *pConnectNode, ComponentSystem *pSystem);

    };
}

#endif // FILEACCESS_INCLUDED


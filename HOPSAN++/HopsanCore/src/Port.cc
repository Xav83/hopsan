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
//! @file   Port.cc
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-01-16
//!
//! @brief Contains Port base class as well as Sub classes
//!
//$Id$

#include "Port.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <math.h>
#include "HopsanEssentials.h"
#include "Component.h"
#include "ComponentSystem.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/StringUtilities.h"

using namespace std;
using namespace hopsan;

//! @brief Port base class constructor
Port::Port(const string nodeType, const string portName, Component *pPortOwner, Port *pParentPort)
{
    mPortType = UndefinedPortType;
    mPortName = portName;
    mNodeType = nodeType;
    mpComponent = pPortOwner;
    mpParentPort = pParentPort; //Only used by subports in multiports
    mConnectionRequired = true;
    mConnectedPorts.clear();
    mpNode = 0;
    mpStartNode = 0;
    mpNCDummyNode = 0;
}


//Destructor
Port::~Port()
{
    if (mpStartNode != 0)
    {
        //! Remove the mapping to eventual system parameters to avoid cowboy-writing in memory after deleted port.
//        for(size_t i = 0; i < mpStartNode->getNumDataVariables(); ++i)
//        {
//FIXA, the parameters will probably be deleted when parent component is deleted -> no problems            getComponent()->getSystemParent()->getSystemParameters().unMapParameter(mpStartNode->getDataPtr(i));
//        }
    }

    //Remove dummy node if it exists
    if (mpNCDummyNode != 0)
    {
        delete mpNCDummyNode;
    }
}



//! @brief Returns the type of node that can be connected to this port
const string Port::getNodeType() const
{
    return mNodeType;
}


//! @brief Returns the parent component
Component* Port::getComponent() const
{
    if(mpParentPort)
        return mpParentPort->getComponent();
    else
        return mpComponent;
}


//! @brief Returns a pointer to the connected node or 0 if no node exist
Node* Port::getNodePtr(const size_t /*portIdx*/)
{
    return mpNode;
}

//! @brief Adds a subport to a multiport
Port* Port::addSubPort()
{
    mpComponent->addWarningMessage("Port::addSubPort(): This should only be implemented and called from multiports.");
    //assert("This should only be implemented and called from multiports" == 0);
    return 0;
}

//! @brief Removes a subport from multiport
void Port::removeSubPort(Port* /*ptr*/)
{
    mpComponent->addWarningMessage("Port::removeSubPort(): This should only be implemented and called from multiports.");
    //assert("This should only be implemented and called from multiports" == 0);
}


//! @brief Load start values by copying the start values from the port to the node
void Port::loadStartValues()
{
    if((isConnected()) && mpStartNode)
    {
        this->mpStartNode->copyNodeDataValuesTo(mpNode);
    }
}


//! @brief Load start values to the start value container from the node (last values from simulation)
void Port::loadStartValuesFromSimulation()
{
    if((isConnected()) && mpStartNode)
    {
        this->mpNode->copyNodeDataValuesTo(mpStartNode);
    }
}


//! @brief Reads a value from the connected node
//! @param [in] idx The data id of the data to read
//! @return The data value
double Port::readNodeSafe(const size_t idx, const size_t /*portIdx*/)
{
    //! @note This if-statement will slow simulation down, but if optimization is desired readNode and writeNode shall not be used anyway.
    if(!isConnected())
    {
        std::stringstream ss;
        ss << "Attempted to call readNode() for non-connected port \"" << this->getName() << "\".";
        mpComponent->addErrorMessage(ss.str());
        mpComponent->getSystemParent()->stopSimulation();     //Read attempt from non-connected port; abort simulation and give error message
        return 0;
    }
    else
    {
        return mpNode->mDataValues[idx];
    }
}


//! @brief Writes a value to the connected node
//! @param [in] idx The data id of the data to write
//! @param [in] value The value of the data to read
void Port::writeNodeSafe(const size_t &idx, const double &value, const size_t /*portIdx*/)
{
    //! @note This if-statement will slow simulation down, but if optimization is desired readNode and writeNode shall not be used anyway.
    if(isConnected())
    {
        mpNode->mDataValues[idx] = value;       //Write to node if there is a node to write to
    }
}


double *Port::getNodeDataPtr(const size_t idx, const size_t /*portIdx*/) const
{
    return mpNode->getDataPtr(idx);
}

//! @brief Get a ptr to the data variable in the node, if node is not created (port not connected) return ptr to dummy node data
//! @param [in] idx The id of the data variable to return ptr to
//! @param [in] defaultValue Default value if port not connected
double *Port::getSafeNodeDataPtr(const size_t idx, const double defaultValue, const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        return mpNode->getDataPtr(idx);
    }
    else
    {
        if (mpNCDummyNode == 0)
        {
            mpNCDummyNode = getComponent()->getHopsanEssentials()->createNode(mNodeType);
            //mpNCDummyNode = createNodeTemp(getComponent()->getHopsanEssentials(), mNodeType);
        }
        mpNCDummyNode->setDataValue(idx, defaultValue);
        return mpNCDummyNode->getDataPtr(idx);
    }
}



//! @brief Set the node that the port is connected to
//! @param [in] pNode A pointer to the Node
void Port::setNode(Node* pNode, const size_t /*portIdx*/)
{
    mpNode = pNode;
}


//! @brief Adds a pointer to an other connected port to a port
//! @param [in] pPort A pointer to the other port
void Port::addConnectedPort(Port* pPort, const size_t /*portIdx*/)
{
    mConnectedPorts.push_back(pPort);
}


//! @brief Removes a pointer to an other connected port from a port
//! @param [in] pPort The pointer to the other port to be removed
void Port::eraseConnectedPort(Port* pPort, const size_t /*portIdx*/)
{
    vector<Port*>::iterator it;
    bool found = false;
    for (it=mConnectedPorts.begin(); it!=mConnectedPorts.end(); ++it)
    {
        if (*it == pPort)
        {
            mConnectedPorts.erase(it);
            found = true;
            //If this was the last port removed the clear rhe node ptr
            if (mConnectedPorts.size() == 0)
            {
                mpNode = 0;
            }
            break;
        }
    }
    if (!found)
    {
        cout << "Error: You tried to erase port ptr that did not exist in the connected ports list" << endl;
    }
}


//! @brief Get a vector of pointers to all other ports connected connected to this one
//! @returns A refernce to the internal vector of connected port pointers
//! @todo maybe should return const vector so that contents my not be changed
vector<Port*> &Port::getConnectedPorts(const int /*portIdx*/)
{
    return mConnectedPorts;
}


void Port::createStartNode(std::string nodeType)
{
    //mpStartNode = createNodeTemp(getComponent()->getHopsanEssentials(), nodeType);
    mpStartNode = getComponent()->getHopsanEssentials()->createNode(nodeType);
    //!< @todo Maye I dont even need to create startnodes for subports in multiports, in that case, move this line into if bellow

    // Prevent registering startvalues for subports in multiports, It will be very difficult to ensure that those would actually work as expected
    if (mpParentPort == 0)
    {
        for(size_t i = 0; i < mpStartNode->getNumDataVariables(); ++i)
        {
            const NodeDataDescription* pDesc = mpStartNode->getDataDescription(i);
            stringstream ssName, ssDesc;
            ssDesc << "startvalue:" << "Port " << getName();
            ssName << getName() << "::" << pDesc->name;
            getComponent()->registerParameter(ssName.str(), ssDesc.str(), pDesc->unit, *(mpStartNode->getDataPtr(pDesc->id)), Constant);
        }
    }
}

//! @note This one should be called by system, do not call this manually (that will create a mess)
void Port::setVariableAlias(const string alias, const int id)
{
    //! @todo check id
    // First remove it if already set
    std::map<std::string, int>::iterator it = mVariableAliasMap.begin();
    while (it!=mVariableAliasMap.end())
    {
        if (it->second == id)
        {
            mVariableAliasMap.erase(it);
            // Restart search if something was removed as itterator breaks
            it = mVariableAliasMap.begin();
        }
        else
        {
             ++it;
        }
    }

    // Replace with new name, if not empty
    if (!alias.empty())
    {
        mVariableAliasMap.insert(std::pair<std::string, int>(alias, id));
    }
}

char* Port::getVariableAlias(const int id) const
{
    char* retval;
    std::map<std::string, int>::const_iterator it;
    for(it=mVariableAliasMap.begin();it!=mVariableAliasMap.end();++it)
    {
        if (it->second == id)
        {
            copyString(&retval, it->first);
            return retval;
            //return it->first;
        }
    }
    copyString(&retval, "");
    return retval;
    //return string();
}

int Port::getVariableIdByAlias(const string alias) const
{
    std::map<std::string, int>::const_iterator it = mVariableAliasMap.find(alias);
    {
        if (it!=mVariableAliasMap.end())
        {
            return it->second;
        }
    }
    return -1;
}


//! @brief Debug function to dump logged node data to a file
//! @param [in] filename The name of the file to write to
void Port::saveLogData(string filename, const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        string header = getComponentName() + "::" + getName();

        ofstream out_file;
        out_file.open(filename.c_str());
        if (out_file.good())
        {
            vector<double>* pTimeStorage = mpNode->getOwnerSystem()->getLogTimeVector();
            if(pTimeStorage->size() != mpNode->mDataStorage.size())
            {
                mpComponent->addFatalMessage("Port::saveLogData(): pTimeStorage->size() != mpNode->mDataStorage.size()");
            }
            //assert(pTimeStorage->size() == mpNode->mDataStorage.size());

            //First write HEADER info containing node info
            out_file << header << " " << mpNode->getNodeType() << endl;
            out_file << "time";
            for (size_t i=0; i<mpNode->getNumDataVariables(); ++i)
            {
                out_file << " " << mpNode->getDataDescription(i)->name;
            }
            out_file << endl;

            //Write log data to file
            for (size_t row=0; row<pTimeStorage->size(); ++row)
            {
                out_file << pTimeStorage->at(row);
                for (size_t datacol=0; datacol<mpNode->getNumDataVariables(); ++datacol)
                {
                    out_file << " " << mpNode->mDataStorage[row][datacol];
                }
                out_file << endl;
            }
            out_file.close();
            cout << "Done! Saving node data to file: " << filename << endl;
        }
        else
        {
            cout << "Warning! Could not open out file for writing: " << filename << endl;
        }
    }
    else
    {
        cout << getComponentName() << "-port:" << mPortName << " can not log data, the Port has no Node connected" << endl;
    }
}

bool Port::haveLogData(const size_t /*portIdx*/)
{
    if (mpNode)
    {
        // Here we assume that timevector DOES exist. If simulation code is correct it should exist
        return !mpNode->mDataStorage.empty();
    }
    return false;
}


//! @brief Get all data names and units from the connected node
//! @param [in,out] rNames This vector will contain the names
//! @param [in,out] rUnits This vector will contain the units
const std::vector<NodeDataDescription>* Port::getNodeDataDescriptions(const size_t /*portIdx*/)
{
    if(this->isConnected())
    {
        return mpNode->getDataDescriptions();
    }
    return 0;
}


//! @brief Get node data name and unit for specific node data
//! @param [in] dataid The node data id
//! @returns A pointer to teh node data description, or 0 if no node exist
const NodeDataDescription* Port::getNodeDataDescription(const size_t dataid, const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        return mpNode->getDataDescription(dataid);
    }
    return 0;
}


//! @brief Wraper for the Node function
int Port::getNodeDataIdFromName(const string name, const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        return mpNode->getDataIdFromName(name);
    }
    else
    {
        return -1;
    }
}

void Port::setSignalNodeUnitAndDescription(const string &rUnit, const string &rName)
{
    //! @todo multiport version needed
    if (mpNode != 0)
    {
        mpNode->setSignalDataUnitAndDescription(rUnit, rName);
    }
}


vector<double> *Port::getLogTimeVectorPtr(const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        ComponentSystem *pOwnerSys = mpNode->getOwnerSystem();
        if (pOwnerSys)
        {
            return pOwnerSys->getLogTimeVector();
        }
    }
    return 0; //Nothing found return 0
}


vector<vector<double> > *Port::getLogDataVectorPtr(const size_t /*portIdx*/)
{
    if (mpNode != 0)
    {
        return &(mpNode->mDataStorage);
    }
    else
    {
        return 0;
    }
}


vector<double> *Port::getDataVectorPtr(const size_t /*portIdx*/)
{
    if(mpNode != 0)
    {
        return &(mpNode->mDataValues);
    }
    else
    {
        return 0;
    }
}


////! @brief Read the start values to a start value node in the port
////! @param[out] rNames is the Vector of names of the star values
////! @param[out] rValues is the Vector of values of the star values, if it is mapped to a System parameter the value of this will be here
////! @param[out] rUnits is the Vector of units of the star values
//void Port::getStartValueDataNamesValuesAndUnits(vector<string> &rNames, std::vector<double> &rValues, vector<string> &rUnits, const size_t /*portIdx*/)
//{
//    if(mpStartNode)
//    {
//        mpStartNode->getDataNamesValuesAndUnits(rNames, rValues, rUnits);
//    }
//}


////! @brief Read the start values to a start value node in the port
////! @param[out] rNames is the Vector of names of the star values
////! @param[out] rValuesTxt is the Vector of values of the star values, if it is mapped to a System parameter the name of this will be here
////! @param[out] rUnits is the Vector of units of the star values
//void Port::getStartValueDataNamesValuesAndUnits(vector<string> &rNames, std::vector<std::string> &rValuesTxt, std::vector<std::string> &rUnits, const size_t /*portIdx*/)
//{
//    if(mpStartNode)
//    {
//        std::vector<double> values;
//        getStartValueDataNamesValuesAndUnits(rNames, values, rUnits);
//        rValuesTxt.resize(values.size());
//        for(size_t i = 0; i < rNames.size(); ++i)
//        {
//            //Get a pointer to the actual node data
//            //! @note This pointer is commented to avoid a warning, recomment it before using it...
//            //double *nodeDataPtr = mpStartNode->getDataPtr(mpStartNode->getDataIdFromName(rNames[i]));


//            assert(false); //If you crash here, please report it to Bjorn who don't think it will happen :-)


//            //Check if the nodeDataPtr is in the System parameters
//            //! @todo Should be done by Bjorn!

//            std::string valueTxt;//FIXA = getComponent()->getSystemParent()->getSystemParameters().findOccurrence(nodeDataPtr);
//            if(!(valueTxt.empty()))
//            {
//                //The nodeDataPrt is connected to a System parameter, read out this name
//                rValuesTxt[i] = valueTxt;
//            }
//            else
//            {
//                //The nodeDataPrt is not connected to a System parameter, read out the node data value to the string
//                std::ostringstream oss;
//                oss << values[i];
//                rValuesTxt[i] = oss.str();
//            }
//        }
//    }
//}


//! @brief Get the actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @returns the start value
double Port::getStartValue(const size_t idx, const size_t /*portIdx*/)
{
    if(mpStartNode && !mpComponent->getSystemParent()->doesKeepStartValues())
        return mpStartNode->getDataValue(idx);
    else if(mpStartNode)
    {
        return mpNode->getDataValue(idx);
    }
    mpComponent->addFatalMessage("Port::getStartValue(): Port does not have a start value.");
    //assert(false);
    return 0.0;
}


//! @brief Set the an actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @param[in] value is the start value that should be written
void Port::setStartValue(const size_t idx, const double value, const size_t /*portIdx*/)
{
    if(mpStartNode)
    {
        mpStartNode->setDataValue(idx, value);

        //! @todo I commented the code bellow to avoid previously dissabled startvalues from reapearing after simulation wher setStartValue was called, I hope this does not screw something up
//        vector<string> dataNames, units;
//        mpStartNode->getDataNamesAndUnits(dataNames, units);
//        stringstream ssName, ssDesc;
//        ssDesc << "startvalue:" << "Port " << getName();
//        ssName << getName() << "::" << dataNames[idx];
//        getComponent()->registerParameter(ssName.str(), ssDesc.str(), units[idx], *mpStartNode->getDataPtr(idx));
    }
    else
    {
        getComponent()->addWarningMessage("Tried to add StartValue for port: " + getName() + " This was ignored because this port does not have any StartValue to set.");
    }
}


//! @brief Disables start value for specified data type
//! @param idx Data index of start value to be disabled
void Port::disableStartValue(const size_t idx)
{
    std::stringstream name, ss;
    //The start value has already been registered as a parameter in the component, so we must unregister it. This is probably not the most beautiful solution.
    name << getName() << "::" << mpStartNode->getDataDescription(idx)->name;
    ss << "Disabling_StartValue: " << name.str();
    mpComponent->addDebugMessage(ss.str());
    mpComponent->unRegisterParameter(name.str());

    //! @todo this is an ugly hack
    mpStartNode->mDataDescriptions.at(idx).name = "";
}


//! @brief Check if the port is curently connected
bool Port::isConnected()
{
    return (mConnectedPorts.size() > 0);
}

//! @brief Check if this port is connected to other port
//! @todo how do we handle multiports
bool Port::isConnectedTo(Port *pOtherPort)
{
    std::vector<Port*>::iterator pit;
    for(pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        if ( *pit == pOtherPort )
        {
            return true;
        }
    }
    return false;
}


//! @brief Check if the port MUST be connected
bool Port::isConnectionRequired()
{
    return mConnectionRequired;
}

size_t Port::getNumPorts()
{
    return 1;
}

//! @brief Convenience functin to check if port is multiport
bool Port::isMultiPort() const
{
    return (mPortType > MultiportType);
}

Port *Port::getParentPort() const
{
    return mpParentPort;
}

//! @brief Get the port type
PortTypesEnumT Port::getPortType() const
{
    return mPortType;
}

//! @brief Get the External port type (virtual, should be overloaded in systemports only)
PortTypesEnumT Port::getExternalPortType()
{
    return getPortType();
}

//! @brief Get the Internal port type (virtual, should be overloaded in systemports only)
PortTypesEnumT Port::getInternalPortType()
{
    return getPortType();
}


//! @brief Get the port name
const string Port::getName() const
{
    return mPortName;
}


//! @brief Get the name of the commponent that the port is attached to
const string Port::getComponentName() const
{
    return getComponent()->getName();
}


//! @brief SystemPort constructor
SystemPort::SystemPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = SystemPortType;
}

//! @brief Get the External port type (virtual, should be overloaded in systemports only)
PortTypesEnumT SystemPort::getExternalPortType()
{
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        //External ports component parents will belong to the same system as our component parent
        if ( (*pit)->getComponent()->getSystemParent() == this->getComponent()->getSystemParent() )
        {
            //! @todo for now we return the first one we find, usually thi is corect except when you are mixing powerports and readports, powerports should be returned in that case but I dont know how to fix this except going through ALL ports every time
            return (*pit)->getPortType();
        }
    }

    //If no external ports found return our actual type (systemport)
    return getPortType();
}

//! @brief Get the Internal port type (virtual, should be overloaded in systemports only)
PortTypesEnumT SystemPort::getInternalPortType()
{
    std::vector<Port*>::iterator pit;
    for (pit=mConnectedPorts.begin(); pit!=mConnectedPorts.end(); ++pit)
    {
        //Internal ports component parents will belong to our component parent
        if ( (*pit)->getComponent()->getSystemParent() == this->getComponent() )
        {
            //! @todo for now we return the first one we find, usually thi is corect except when you are mixing powerports and readports, powerports should be returned in that case but I dont know how to fix this except going through ALL ports every time
            return (*pit)->getPortType();
        }
    }

    //If no internal ports found return our actual type (systemport)
    return getPortType();
}


//! @brief PowerPort constructor
PowerPort::PowerPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = PowerPortType;
    if(getComponent()->isComponentC())
    {
        createStartNode(mNodeType);
    }
}


ReadPort::ReadPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = ReadPortType;
}


void ReadPort::writeNodeSafe(const size_t /*idx*/, const double /*value*/)
{
    mpComponent->addWarningMessage("ReadPort::writeNodeSafe(): Could not write to port, this is a ReadPort.");
    //assert("Could not write to port, this is a ReadPort" == 0);
}


void ReadPort::writeNode(const size_t /*idx*/, const double /*value*/) const
{
    mpComponent->addWarningMessage("ReadPort::writeNode(): Could not write to port, this is a ReadPort.");
    //assert("Could not write to port, this is a ReadPort" == 0);
}


WritePort::WritePort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = WritePortType;
    createStartNode(mNodeType);
}


double WritePort::readNodeSafe(const size_t /*idx*/)
{
    mpComponent->addWarningMessage("WritePort::readNodeSafe(): Could not read to port, this is a WritePort");
    //assert("Could not read from port, this is a WritePort" == 0);
    return 0;
}

double WritePort::readNode(const size_t /*idx*/) const
{
    mpComponent->addWarningMessage("WritePort::readNode(): Could not read to port, this is a WritePort");
    //assert("Could not read from port, this is a WritePort" == 0);
    return 0;
}

MultiPort::MultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : Port(node_type, portname, portOwner, pParentPort)
{
    mPortType = MultiportType;
}

MultiPort::~MultiPort()
{
    //Deleate all subports thay may remain, if everything is working this shoudl be zero
    //! @todo removed assert, BUT problem needs to be fixed /Peter
    if (mSubPortsVector.size() != 0)
    {
        getComponent()->addFatalMessage("~MultiPort(): mSubPortsVector.size() != 0 in multiport destructor (will fix later)");
    }
    //assert(mSubPortsVector.size() == 0); //should be removed by other code, use this assert to check if that is working
}


double MultiPort::readNodeSafe(const size_t idx, const size_t portIdx)
{
    //! @todo handle portIdx ot of range
    return mSubPortsVector[portIdx]->readNodeSafe(idx);
}

void MultiPort::writeNodeSafe(const size_t &idx, const double &value, const size_t portIdx)
{
    return mSubPortsVector[portIdx]->writeNode(idx,value);
}


double MultiPort::readNode(const size_t idx, const size_t portIdx) const
{
    //! @todo handle portIdx ot of range
    return mSubPortsVector[portIdx]->readNode(idx);
}

void MultiPort::writeNode(const size_t &idx, const double &value, const size_t portIdx) const
{
    return mSubPortsVector[portIdx]->writeNode(idx,value);
}

double *MultiPort::getNodeDataPtr(const size_t idx, const size_t portIdx) const
{
    return mSubPortsVector[portIdx]->getNodeDataPtr(idx);
}

double *MultiPort::getSafeNodeDataPtr(const size_t idx, const double defaultValue, const size_t portIdx)
{
    //If we try to access node data for subport that does not exist then return multiport shared dummy safe ptr
    if (portIdx >= mSubPortsVector.size())
    {
        return Port::getSafeNodeDataPtr(idx, defaultValue, portIdx);
    }
    else
    {
        return mSubPortsVector[portIdx]->getSafeNodeDataPtr(idx, defaultValue);
    }
}

void MultiPort::saveLogData(std::string filename, const size_t portIdx)
{
    return mSubPortsVector[portIdx]->saveLogData(filename);
}

const std::vector<NodeDataDescription>* MultiPort::getNodeDataDescriptions(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getNodeDataDescriptions();
    }
    return 0;
}

const NodeDataDescription* MultiPort::getNodeDataDescription(const size_t dataid, const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getNodeDataDescription(dataid);
    }
    return 0;
}

int MultiPort::getNodeDataIdFromName(const std::string name, const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getNodeDataIdFromName(name);
    }
    return -1;
}

bool MultiPort::haveLogData(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->haveLogData();
    }
    return false;
}

std::vector<double> *MultiPort::getLogTimeVectorPtr(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getLogTimeVectorPtr();
    }
    return 0;
}

std::vector<std::vector<double> > *MultiPort::getLogDataVectorPtr(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getLogDataVectorPtr();
    }
    return 0;
}

std::vector<double> *MultiPort::getDataVectorPtr(const size_t portIdx)
{
    if (isConnected())
    {
        return mSubPortsVector[portIdx]->getDataVectorPtr();
    }
    return 0;
}

//! @brief Get the an actual start value of the port
//! @param[in] idx is the index of the start value e.g. NodeHydraulic::PRESSURE
//! @returns the start value
double MultiPort::getStartValue(const size_t idx, const size_t portIdx)
{
    if(mpStartNode && !mpComponent->getSystemParent()->doesKeepStartValues())
        return mpStartNode->getDataValue(idx);
    else if(mpStartNode)
    {
        return mSubPortsVector[portIdx]->mpNode->getDataValue(idx);
    }
    mpComponent->addFatalMessage("MultiPort::getStartValue(): Port does not have a start value.");
    //assert(false);
    return 0.0;
}

void MultiPort::loadStartValues()
{
    //! @todo what should we do here actaully, from where should we copy the starvalues and where to, maybe we should tell the component programmer to fix this
}

void MultiPort::loadStartValuesFromSimulation()
{
    //! @todo what about this one then how should we handle this
}

//! Check if the port is curently connected
bool MultiPort::isConnected()
{
    //! @todo actaully we should check all subports if they are connected (but a subport should not exist if not connected)
    return (mSubPortsVector.size() > 0);
}

size_t MultiPort::getNumPorts()
{
    return mSubPortsVector.size();
}

//! Removes a specific subport
void MultiPort::removeSubPort(Port* ptr)
{
    std::vector<Port*>::iterator spit;
    for (spit=mSubPortsVector.begin(); spit!=mSubPortsVector.end(); ++spit)
    {
        if ( *spit == ptr )
        {
            mSubPortsVector.erase(spit);
            delete ptr;
            break;
        }
    }
}

//! Retreives Node Ptr from given subnode
Node *MultiPort::getNodePtr(const size_t portIdx)
{
    //assert(mSubPortsVector.size() > portIdx);
    if(mSubPortsVector.size() <= portIdx)
    {
        mpComponent->addWarningMessage("MultiPort::getNodePtr(): mSubPortsSVector.size() <= portIdx");
        return 0;
    }
    return mSubPortsVector[portIdx]->getNodePtr();
}

//! we use -1 as portindex to indicate that we want all subports
std::vector<Port*> &MultiPort::getConnectedPorts(const int portIdx)
{
    if (portIdx<0)
    {
        //Ok lets return ALL connected ports
        //! @todo since this function returns a reference to the internal vector we need a new memberVector
        mAllConnectedPorts.clear();
        for (size_t i=0; i<mSubPortsVector.size(); ++i)
        {
            for (size_t j=0; j<mSubPortsVector[i]->getConnectedPorts().size(); ++j)
            {
                mAllConnectedPorts.push_back(mSubPortsVector[i]->getConnectedPorts()[j]);
            }
        }

        return mAllConnectedPorts;
    }
    else
    {
        return mSubPortsVector[portIdx]->getConnectedPorts();
    }
}

PowerMultiPort::PowerMultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : MultiPort(node_type, portname, portOwner, pParentPort)
{
    mPortType = PowerMultiportType;
    if(getComponent()->isComponentC())
    {
        createStartNode(mNodeType);
    }
}

//! Adds a subport to a powermultiport
Port* PowerMultiPort::addSubPort()
{
    mSubPortsVector.push_back( createPort(PowerPortType, mNodeType, "noname_subport", 0, this) );
    return mSubPortsVector.back();
}

ReadMultiPort::ReadMultiPort(std::string node_type, std::string portname, Component *portOwner, Port *pParentPort) : MultiPort(node_type, portname, portOwner, pParentPort)
{
    mPortType = ReadMultiportType;
}

//! Adds a subport to a readmultiport
Port* ReadMultiPort::addSubPort()
{
    mSubPortsVector.push_back( createPort(ReadPortType, mNodeType, "noname_subport", 0, this) );
    return mSubPortsVector.back();
}

//! @brief A very simple port factory, no need to complicate things with the more advanced one as we will only have a few fixed port types.
//! @param [in] portType The type of port to create
//! @param [in] nodeType The type of node that the port should contain
//! @param [in] name The name of the port
//! @param [in] pPortOwner A pointer to the owner component
//! @param [in] pParentPort A pointer to the parent port in case of creation of a subport to a multiport
//! @return A pointer to the created port
Port* hopsan::createPort(const PortTypesEnumT portType, const std::string nodeType, const string name, Component *pPortOwner, Port *pParentPort)
{
    switch (portType)
    {
    case PowerPortType :
        return new PowerPort(nodeType, name, pPortOwner, pParentPort);
        break;
    case WritePortType :
        return new WritePort(nodeType, name, pPortOwner, pParentPort);
        break;
    case ReadPortType :
        return new ReadPort(nodeType, name, pPortOwner, pParentPort);
        break;
    case SystemPortType :
        return new SystemPort(nodeType, name, pPortOwner, pParentPort);
        break;
    case PowerMultiportType :
        return new PowerMultiPort(nodeType, name, pPortOwner, pParentPort);
        break;
    case ReadMultiportType :
        return new ReadMultiPort(nodeType, name, pPortOwner, pParentPort);
        break;
    default :
       return 0;
    }
}

//! @brief Converts a PortTypeEnum to string
//! @param [in] type The port type enum
//! @return The port type in string format
std::string hopsan::portTypeToString(const PortTypesEnumT type)
{
    switch (type)
    {
    case PowerPortType :
        return "POWERPORT";
        break;
    case ReadPortType :
        return "READPORT";
        break;
    case WritePortType :
        return "WRITEPORT";
        break;
    case SystemPortType :
        return "SYSTEMPORT";
        break;
    case MultiportType:
        return "MULTIPORT";
        break;
    case PowerMultiportType:
        return "POWERMULTIPORT";
        break;
    case ReadMultiportType:
        return "READMULTIPORT";
        break;
    default :
        return "UNDEFINEDPORT";
    }
}

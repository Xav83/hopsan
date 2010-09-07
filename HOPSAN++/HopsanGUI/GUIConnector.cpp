/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//$Id$


#include <QDebug>
#include <QStyleOptionGraphicsItem>

#include "GUIPort.h"
#include "GraphicsView.h"
#include "GUIUtilities.h"
#include "GUIObject.h"
#include "GUIConnector.h"
#include "UndoStack.h"
#include "ProjectTabWidget.h"
#include "GUISystem.h"
#include <math.h>

//! Constructor.
//! @param startpos defines the start position of the connector, normally the center of the starting port.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(GUIPort *startPort, GUISystem *parentSystem, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    mpParentSystem = parentSystem;
    mpParentSystem->scene()->addItem(this);
    startPort->getGuiObject()->rememberConnector(this);

    setFlags(QGraphicsItem::ItemIsFocusable);
    connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(zoomChange()), this, SLOT(adjustToZoom()));
    connect(mpParentSystem, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
    connect(mpParentSystem, SIGNAL(setAllGfxType(graphicsType)), this, SLOT(setIsoStyle(graphicsType)));

    QPointF startPos = startPort->mapToScene(startPort->boundingRect().center());
    this->setPos(startPos);
    this->updateStartPoint(startPos);
    mpGUIConnectorAppearance = new GUIConnectorAppearance("notconnected", mpParentSystem->mGfxType);
    mEndPortConnected = false;
    this->drawConnector();
    mMakingDiagonal = false;
    this->setStartPort(startPort);
    this->addPoint(startPos);
    this->addPoint(startPos);
}


//! Constructor used to create a whole connector at once. Used when loading models.
//! @param *startPort is a pointer to the start port.
//! @param *endPort is a pointer to the end port.
//! @param points is the point vector for the connector.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(GUIPort *startPort, GUIPort *endPort, QVector<QPointF> points, GUISystem *parentSystem, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    mpParentSystem = parentSystem;
    setFlags(QGraphicsItem::ItemIsFocusable);
    mpStartPort = startPort;
    mpEndPort = endPort;
    mpStartPort->isConnected = true;
    mpEndPort->isConnected = true;
    connect(mpStartPort->getGuiObject(),SIGNAL(componentSelected()),this,SLOT(selectIfBothComponentsSelected()));
    connect(mpEndPort->getGuiObject(),SIGNAL(componentSelected()),this,SLOT(selectIfBothComponentsSelected()));
    QPointF startPos = getStartPort()->mapToScene(getStartPort()->boundingRect().center());
    this->setPos(startPos);

    mpGUIConnectorAppearance = new GUIConnectorAppearance(startPort->getPortType(), mpParentSystem->mGfxType);

    mPoints = points;

        //Setup the geometries vector based on the point geometry
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        if(mPoints[i].x() == mPoints[i+1].x())
            mGeometries.push_back(HORIZONTAL);
        else if(mPoints[i].y() == mPoints[i+1].y())
            mGeometries.push_back(VERTICAL);
        else
            mGeometries.push_back(DIAGONAL);
    }

    mEndPortConnected = true;
    emit endPortConnected();
    this->setPassive();
    connect(mpEndPort->getGuiObject(),SIGNAL(componentDeleted()),this,SLOT(deleteMeWithNoUndo()));

        //Create the lines, so that drawConnector has something to work with
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        mpTempLine = new GUIConnectorLine(mapFromScene(mPoints[i]).x(), mapFromScene(mPoints[i]).y(),
                                          mapFromScene(mPoints[i+1]).x(), mapFromScene(mPoints[i+1]).y(),
                                          mpGUIConnectorAppearance, i, this);
        //qDebug() << "Creating line from " << mPoints[i].x() << ", " << mPoints[i].y() << " to " << mPoints[i+1].x() << " " << mPoints[i+1].y();
        mpLines.push_back(mpTempLine);
        mpTempLine->setConnected();
        mpTempLine->setPassive();
        connect(mpTempLine,SIGNAL(lineSelected(bool, int)),this,SLOT(doSelect(bool, int)));
        connect(mpTempLine,SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
        connect(mpTempLine,SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
        connect(mpTempLine,SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
        connect(this,SIGNAL(endPortConnected()),mpTempLine,SLOT(setConnected()));
    }

    this->determineAppearance();
    this->drawConnector();

        //Make all lines selectable and all lines except first and last movable
    for(int i=1; i<mpLines.size()-1; ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    for(int i=0; i<mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);



//      //Add arrow to the connector if it is of signal type
//    if(mpEndPort->getPortType() == "READPORT" && mpEndPort->getNodeType() == "NodeSignal")
//        this->getLastLine()->addEndArrow();
//    else if(mpEndPort->getPortType() == "WRITEPORT" && mpEndPort->getNodeType() == "NodeSignal")
//        mpLines[0]->addStartArrow();

    mpStartPort->getGuiObject()->rememberConnector(this);
    mpEndPort->getGuiObject()->rememberConnector(this);

    connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(zoomChange()), this, SLOT(adjustToZoom()));
    connect(mpParentSystem, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
    connect(mpParentSystem, SIGNAL(setAllGfxType(graphicsType)), this, SLOT(setIsoStyle(graphicsType)));
}


//! Destructor.
GUIConnector::~GUIConnector()
{
    delete mpGUIConnectorAppearance;

    mpStartPort->getGuiObject()->forgetConnector(this);
    mpEndPort->getGuiObject()->forgetConnector(this);
}


//! Inserts a new point to the connector and adjusts the previous point accordingly, depending on the geometry vector.
//! @param point is the position where the point shall be inserted, normally the cursor position.
//! @see removePoint(bool deleteIfEmpty)
void GUIConnector::addPoint(QPointF point)
{
    //point = this->mapFromScene(point);
    mPoints.push_back(point);


    qDebug() << "the enum: " << getStartPort()->getPortDirection();

    if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == TOPBOTTOM)
    {
        mGeometries.push_back(HORIZONTAL);
    }
    else if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == LEFTRIGHT)
    {
        mGeometries.push_back(VERTICAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == HORIZONTAL)
    {
        mGeometries.push_back(VERTICAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == VERTICAL)
    {
        mGeometries.push_back(HORIZONTAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == DIAGONAL)
    {
        mGeometries.push_back(DIAGONAL);
        //Give new line correct angle!
    }
    if(mPoints.size() > 1)
        drawConnector();
}


//! Removes the last point in the connecetor. Asks to delete the connector if deleteIfEmpty is true and if no lines or only one non-diagonal lines remains.
//! @param deleteIfEmpty tells whether or not the connector shall be deleted if too few points remains.
//! @see addPoint(QPointF point)
void GUIConnector::removePoint(bool deleteIfEmpty)
{
    mPoints.pop_back();
    mGeometries.pop_back();
    //qDebug() << "removePoint, getNumberOfLines = " << getNumberOfLines();
    if(getNumberOfLines() > 3 and !mMakingDiagonal)
    {
        if((mGeometries[mGeometries.size()-1] == DIAGONAL) or ((mGeometries[mGeometries.size()-2] == DIAGONAL)))
        {
            //if(mGeometries[mGeometries.size()-3] == HORIZONTAL)
            if(abs(mPoints[mPoints.size()-3].x() - mPoints[mPoints.size()-4].x()) > abs(mPoints[mPoints.size()-3].y() - mPoints[mPoints.size()-4].y()))
            {
                mGeometries[mGeometries.size()-2] = HORIZONTAL;
                mGeometries[mGeometries.size()-1] = VERTICAL;
                mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-3].x(), mPoints[mPoints.size()-1].y());
            }
            else
            {
                mGeometries[mGeometries.size()-2] = VERTICAL;
                mGeometries[mGeometries.size()-1] = HORIZONTAL;
                mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-1].x(), mPoints[mPoints.size()-3].y());
            }
        }
    }
    else if(getNumberOfLines() == 3 and !mMakingDiagonal)
    {
        if(getStartPort()->getPortDirection() == LEFTRIGHT)
        {
            mGeometries[1] = HORIZONTAL;
            mGeometries[0] = VERTICAL;
            mPoints[1] = QPointF(mPoints[2].x(), mPoints[0].y());
        }
        else
        {
            mGeometries[1] = VERTICAL;
            mGeometries[0] = HORIZONTAL;
            mPoints[1] = QPointF(mPoints[0].x(), mPoints[2].y());
        }
    }

    if(mPoints.size() == 2 and !mMakingDiagonal)
    {
        mPoints.pop_back();
        mGeometries.pop_back();
    }
    drawConnector();
    if(mPoints.size() == 1 && deleteIfEmpty)
    {
        deleteMeWithNoUndo();
    }
}


//! Sets the pointer to the start port of a connector.
//! @param *port is the pointer to the new start port.
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setStartPort(GUIPort *port)
{
    mpStartPort = port;
    mpStartPort->isConnected = true;
    connect(mpStartPort->getGuiObject(),SIGNAL(componentDeleted()),this,SLOT(deleteMeWithNoUndo()));
    connect(mpStartPort->getGuiObject(),SIGNAL(componentSelected()),this,SLOT(selectIfBothComponentsSelected()));
}


//! Sets the pointer to the end port of a connector, and executes the final tasks before creation of the connetor is complete. Then flags that the end port is connected.
//! @param *port is the pointer to the new end port.
//! @see setStartPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setEndPort(GUIPort *port)
{
    mEndPortConnected = true;
    mpEndPort = port;
    mpEndPort->isConnected = true;

    if( ( ((mpEndPort->getPortDirection() == LEFTRIGHT) and (mGeometries.back() == HORIZONTAL)) or
          ((mpEndPort->getPortDirection() == TOPBOTTOM) and (mGeometries.back() == VERTICAL)) ) or
          (mGeometries[mGeometries.size()-2] == DIAGONAL))
    {
            //Wrong direction of last line, so remove last point. It will be fine.
        this->removePoint();
        this->mpLines.pop_back();
    }
    else
    {
            //Move second last line a bit outwards from the component
        QPointF offsetPoint = getOffsetPointfromPort(mpEndPort);
        mPoints[mPoints.size()-2] += offsetPoint;
        mPoints[mPoints.size()-3] += offsetPoint;
        this->drawConnector();
        //mpParentSystem->setBackgroundBrush(mpParentSystem->mBackgroundColor);
        mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
    }

    this->updateEndPoint(port->mapToScene(port->boundingRect().center()));
    connect(mpEndPort->getGuiObject(),SIGNAL(componentDeleted()),this,SLOT(deleteMeWithNoUndo()));
    connect(mpEndPort->getGuiObject(),SIGNAL(componentSelected()),this,SLOT(selectIfBothComponentsSelected()));

        //Make all lines selectable and all lines except first and last movable
    if(mpLines.size() > 1)
    {
        for(int i=1; i!=mpLines.size()-1; ++i)
            mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    }
    for(int i=0; i!=mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

    emit endPortConnected();
    this->determineAppearance();
    this->setPassive();
}


//! Slot that tells the connector lines whether or not to use ISO style.
//! @param gfxType tells whether or not iso graphics is to be used
void GUIConnector::setIsoStyle(graphicsType gfxType)
{
    mpGUIConnectorAppearance->setIsoStyle(gfxType);
    for (int i=0; i!=mpLines.size(); ++i )
    {
        //Refresh each line by setting to passive (primary) appearance
        mpLines[i]->setPassive();
    }
}


//! Slot that tells the lines to adjust their size to the zoom factor. This is to make sure line will not become invisible when zooming out.
void GUIConnector::adjustToZoom()
{
    mpGUIConnectorAppearance->adjustToZoom(mpParentSystem->mpParentProjectTab->mpGraphicsView->mZoomFactor);
    for (int i=0; i!=mpLines.size(); ++i )
    {
        //Refresh each line by setting to passive (primary) appearance
        mpLines[i]->setPassive();
    }
}


//! Returns the number of lines in a connector.
int GUIConnector::getNumberOfLines()
{
    return mpLines.size();
}


//! Returns the geometry type of the specified line.
//! @param lineNumber is the number of the specified line in the mpLines vector.
connectorGeometry GUIConnector::getGeometry(int lineNumber)
{
    return mGeometries[lineNumber];
}


//! Returns the point vector used by the connector.
QVector<QPointF> GUIConnector::getPointsVector()
{
    return mPoints;
}


//! Returns a pointer to the start port of a connector.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getEndPort()
GUIPort *GUIConnector::getStartPort()
{
    return mpStartPort;
}


//! Returns a pointer to the end port of a connector.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
GUIPort *GUIConnector::getEndPort()
{
    return mpEndPort;
}


//! Returns the name of the start port of a connector.
//! @see getEndPortName()
QString GUIConnector::getStartPortName()
{
    return mpStartPort->getName();
}


//! Returns the name of the end port of a connector.
//! @see getStartPortName()
QString GUIConnector::getEndPortName()
{
    return mpEndPort->getName();
}


//! Returns the name of the start component of a connector.
//! @see getEndComponentName()
QString GUIConnector::getStartComponentName()
{
    return mpStartPort->getGUIComponentName();
}


//! Returns the name of the end component of a connector.
//! @see getStartComponentName()
QString GUIConnector::getEndComponentName()
{
    return mpEndPort->getGUIComponentName();
}


//! Returns the line with specified number.
//! @param line is the number of the wanted line.
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLastLine()
GUIConnectorLine *GUIConnector::getLine(int line)
{
    return mpLines[line];
}


//! Returns the last line of the connector.
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getLastLine()
{
    return mpLines[mpLines.size()-1];
}


//! Returns the second last line of the connector.
//! @see getThirdLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getSecondLastLine()
{
    return mpLines[mpLines.size()-2];
}


//! Returns the third last line of the connector.
//! @see getSecondLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getThirdLastLine()
{
    return mpLines[mpLines.size()-3];
}


//! Returns true if the connector is connected at both ends, otherwise false.
bool GUIConnector::isConnected()
{
    //qDebug() << "Entering isConnected()";
    //return (getStartPort()->isConnected and getEndPort()->isConnected);
    return (getStartPort()->isConnected and mEndPortConnected);
}


//! Returns true if the line currently being drawn is a diagonal one, otherwise false.
//! @see makeDiagonal(bool enable)
bool GUIConnector::isMakingDiagonal()
{
    return mMakingDiagonal;
}


//! Returns true if the connector is active ("selected").
bool GUIConnector::isActive()
{
    return mIsActive;
}


//! Saves all necessary information about the connetor to a text stream. Used for save, undo and copy operations.
//! @param QTextSream is the text stream with the information.
void GUIConnector::saveToTextStream(QTextStream &rStream, QString prepend)
{
    QString startObjName = getStartComponentName();
    QString endObjName = getEndComponentName();
    QString startPortName  = getStartPortName();
    QString endPortName = getEndPortName();
    if (!prepend.isEmpty())
    {
        rStream << prepend << " ";
    }
    rStream << ( addQuotes(startObjName) + " " + addQuotes(startPortName) + " " + addQuotes(endObjName) + " " + addQuotes(endPortName) );
    for(int j = 0; j != mPoints.size(); ++j)
    {
        rStream << " " << mPoints[j].x() << " " << mPoints[j].y();
    }
    rStream << "\n";
}


//! Draws lines between the points in the mPoints vector, and stores them in the mpLines vector.
void GUIConnector::drawConnector()
{
    if(!mEndPortConnected)        //End port is not connected, which means we are creating a new line
    {
            //Remove lines if there are too many
        while(mpLines.size() > mPoints.size()-1)
        {
            this->scene()->removeItem(mpLines.back());
            mpLines.pop_back();
        }
            //Add lines if there are too few
        while(mpLines.size() < mPoints.size()-1)
        {
            mpTempLine = new GUIConnectorLine(0, 0, 0, 0, mpGUIConnectorAppearance, mpLines.size(), this);
            mpTempLine->setPassive();
            connect(mpTempLine,SIGNAL(lineSelected(bool, int)),this,SLOT(doSelect(bool, int)));
            connect(mpTempLine,SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
            connect(mpTempLine,SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
            connect(mpTempLine,SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
            connect(this,SIGNAL(endPortConnected()),mpTempLine,SLOT(setConnected()));
            mpLines.push_back(mpTempLine);
        }
    }
    else        //End port is connected, so the connector is modified or has moved
    {
        if(mpStartPort->getGuiObject()->isSelected() and mpEndPort->getGuiObject()->isSelected() and this->isActive())
        {
                //Both components and connector are selected, so move whole connector along with components
            moveAllPoints(getStartPort()->mapToScene(getStartPort()->boundingRect().center()).x()-mPoints[0].x(),
                          getStartPort()->mapToScene(getStartPort()->boundingRect().center()).y()-mPoints[0].y());
        }
        else
        {
                //Retrieve start and end points from ports in case components have moved
            updateStartPoint(getStartPort()->mapToScene(getStartPort()->boundingRect().center()));
            updateEndPoint(getEndPort()->mapToScene(getEndPort()->boundingRect().center()));
        }
    }

       //Redraw the lines based on the mPoints vector
    for(int i = 0; i != mPoints.size()-1; ++i)
    {
        if(mpLines[i]->line().p1() != mPoints[i] or mpLines[i]->line().p2() != mPoints[i+1]);   //Don't redraw the line if it has not changed
        mpLines[i]->setLine(mapFromScene(mPoints[i]), mapFromScene(mPoints[i+1]));
    }

    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Updates the first point of the connector, and adjusts the second point accordingly depending on the geometry vector.
//! @param point is the new start point.
//! @see updateEndPoint(QPointF point)
void GUIConnector::updateStartPoint(QPointF point)
{
    if(mPoints.size() == 0)
        mPoints.push_back(point);
    else
        mPoints[0] = point;

    if(mPoints.size() != 1)
    {
        if(mGeometries[0] == HORIZONTAL)
            mPoints[1] = QPointF(mPoints[0].x(),mPoints[1].y());
        else if(mGeometries[0] == VERTICAL)
            mPoints[1] = QPointF(mPoints[1].x(),mPoints[0].y());
    }
}


//! Updates the last point of the connector, and adjusts the second last point accordingly depending on the geometry vector.
//! @param point is the new start point.
//! @see updateEndPoint(QPointF point)
void GUIConnector::updateEndPoint(QPointF point)
{
    mPoints.back() = point;
    if(mGeometries.back() == HORIZONTAL)
    {
        mPoints[mPoints.size()-2] = QPointF(point.x(),mPoints[mPoints.size()-2].y());
    }
    else if(mGeometries.back() == VERTICAL)
    {
        mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x(),point.y());
    }
}


//! Updates the mPoints vector when a line has been moved. Used to make lines follow each other when they are moved, and to make sure horizontal lines can only move vertically and vice versa.
//! @param lineNumber is the number of the line that has moved.
void GUIConnector::updateLine(int lineNumber)
{
   if ((mEndPortConnected) && (lineNumber != 0) && (lineNumber != int(mpLines.size())))
    {
        if(mGeometries[lineNumber] == HORIZONTAL)
        {
            mPoints[lineNumber] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).x(), mPoints[lineNumber].y());
            mPoints[lineNumber+1] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).x(), mPoints[lineNumber+1].y());
        }
        else if (mGeometries[lineNumber] == VERTICAL)
        {
            mPoints[lineNumber] = QPointF(mPoints[lineNumber].x(), getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).y());
            mPoints[lineNumber+1] = QPointF(mPoints[lineNumber+1].x(), getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).y());
        }
    }

    drawConnector();
}


//! Slot that moves all points in the connector a specified distance in a specified direction. This is used in copy-paste operations.
//! @param offsetX is the distance to move in X direction.
//! @param offsetY is the distance to move in Y direction.
void GUIConnector::moveAllPoints(qreal offsetX, qreal offsetY)
{
    for(int i=0; i != mPoints.size(); ++i)
    {
        mPoints[i] = QPointF(mPoints[i].x()+offsetX, mPoints[i].y()+offsetY);
    }
}


//! Tells the connector to create one diagonal lines instead of the last two horizontal/vertical, or to return to horizontal/diagonal mode.
//! @param enable indicates whether diagonal mode shall be enabled or disabled.
//! @see isMakingDiagonal()
void GUIConnector::makeDiagonal(bool enable)
{
    QCursor cursor;
    if(enable)
    {
        mMakingDiagonal = true;
        removePoint();
        mGeometries.back() = DIAGONAL;
        mPoints.back() = mpParentSystem->mpParentProjectTab->mpGraphicsView->mapToScene(mpParentSystem->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos()));
        drawConnector();
    }
    else
    {
        if(this->getNumberOfLines() > 1)
        {
            if(mGeometries[mGeometries.size()-2] == HORIZONTAL)
            {
                mGeometries.back() = VERTICAL;
                mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
            }
            else if(mGeometries[mGeometries.size()-2] == VERTICAL)
            {
                mGeometries.back() = HORIZONTAL;
                mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
            }
            else if(mGeometries[mGeometries.size()-2] == DIAGONAL)
            {
                if(abs(mPoints[mPoints.size()-2].x() - mPoints[mPoints.size()-3].x()) > abs(mPoints[mPoints.size()-2].y() - mPoints[mPoints.size()-3].y()))
                {
                    mGeometries.back() = HORIZONTAL;
                    mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
                }
                else
                {
                    mGeometries.back() = VERTICAL;
                    mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
                }

            }
            addPoint(mpParentSystem->mapToScene(mpParentSystem->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos())));
        }
        else    //Only one (diagonal) line exist, so special solution is required
        {
            addPoint(mpParentSystem->mapToScene(mpParentSystem->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos())));
            if(getStartPort()->getPortDirection() == LEFTRIGHT)
            {
                mGeometries[0] = VERTICAL;
                mGeometries[1] = HORIZONTAL;
                mPoints[1] = QPointF(mPoints[2].x(), mPoints[0].y());
            }
            else
            {
                mGeometries[0] = HORIZONTAL;
                mGeometries[1] = VERTICAL;
                mPoints[1] = QPointF(mPoints[0].x(), mPoints[2].y());
            }
        }

        drawConnector();
        mMakingDiagonal = false;
    }
}


//! Slot that activates or deactivates the connector if one of its lines is selected or deselected.
//! @param lineSelected tells whether the signal was induced by selection or deselection of a line.
//! @see setActive()
//! @see setPassive()
void GUIConnector::doSelect(bool lineSelected, int lineNumber)
{
    if(mEndPortConnected)     //Non-finished connectors shall not be selectable
    {
        if(lineSelected)
        {
            if(!mpParentSystem->mSelectedSubConnectorsList.contains(this))
            {
                mpParentSystem->mSelectedSubConnectorsList.append(this);
            }
            connect(mpParentSystem, SIGNAL(deselectAllGUIConnectors()), this, SLOT(deselect()));
            disconnect(mpParentSystem, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            this->setActive();
            for (int i=0; i != mpLines.size(); ++i)
            {
               if(i != lineNumber)     //This makes sure that only one line in a connector can be "selected" at one time
               {
                   mpLines[i]->setSelected(false);
               }
            }
        }
        else
        {
            bool noneSelected = true;
            for (int i=0; i != mpLines.size(); ++i)
            {
               if(mpLines[i]->isSelected())
                {
                   noneSelected = false;
               }
            }
            if(noneSelected)
            {
                this->setPassive();
                mpParentSystem->mSelectedSubConnectorsList.removeOne(this);
                disconnect(mpParentSystem, SIGNAL(deselectAllGUIConnectors()), this, SLOT(deselect()));
                connect(mpParentSystem, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
                disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            }
        }
    }
}


//! Slot that selects a connector if both its components are selected.
//! @see doSelect(bool lineSelected, int lineNumber)
void GUIConnector::selectIfBothComponentsSelected()
{
    if(mEndPortConnected and mpStartPort->getGuiObject()->isSelected() and mpEndPort->getGuiObject()->isSelected())
    {
        mpLines[0]->setSelected(true);
        doSelect(true,0);
    }
}


//! Activates a connector, activates each line and connects delete function with delete key.
//! @see setPassive()
void GUIConnector::setActive()
{
    connect(mpParentSystem, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
    if(mEndPortConnected)
    {
        mIsActive = true;
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setActive();
            //mpLines[i]->setSelected(true);         //???
        }
    }
}


//! Deactivates a connector, deactivates each line and disconnects delete function with delete key.
//! @see setActive()
void GUIConnector::setPassive()
{
    disconnect(mpParentSystem, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
    if(mEndPortConnected)
    {
        mIsActive = false;
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
            mpLines[i]->setSelected(false);       //OBS! Kanske inte blir bra...
        }
    }
}


//! Changes connector style to hovered if it is not active. Used when mouse starts hovering a line.
//! @see setUnHovered()
void GUIConnector::setHovered()
{
    if(mEndPortConnected && !mIsActive)
    {
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setHovered();
        }
    }
}


//! Changes connector style back to normal if it is not active. Used when mouse stops hovering a line.
//! @see setHovered()
//! @see setPassive()
void GUIConnector::setUnHovered()
{
    if(mEndPortConnected && !mIsActive)
    {
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
        }
    }
}


//! Asks my parent to delete myself.
void GUIConnector::deleteMe()
{
    //qDebug() << "deleteMe()";
    mpParentSystem->removeConnector(this, UNDO);
}


//! Asks my parent to delete myself, and tells it to not add me to the undo stack.
void GUIConnector::deleteMeWithNoUndo()
{
    mpParentSystem->removeConnector(this, NOUNDO);
}

//! Uppdate the appearance of the connector (setting its type and line endings)
//! @todo right now this only set the type and ending arrows, maybe should handle ALLA appearance update like switching when howering, or maybe have two different update appearance functions (this one only needs to be run once when a conector is created)
void GUIConnector::determineAppearance()
{
    //! @todo problem when connecting outside systemport  with internal powerport to readport, will not know that internal port is powerport and line will be signalline
    //! @todo need to figure out a new way to handle this
    if(mpStartPort->getPortType() == "POWERPORT" or mpEndPort->getPortType() == "POWERPORT")
    {
        mpGUIConnectorAppearance->setType("POWERPORT");
    }
    else if (mpStartPort->getPortType() == "READPORT" or mpEndPort->getPortType() == "READPORT")
    {
        mpGUIConnectorAppearance->setType("SIGNALPORT");
    }
    else if (mpStartPort->getPortType() == "WRITEPORT" or mpEndPort->getPortType() == "WRITEPORT")
    {
        mpGUIConnectorAppearance->setType("SIGNALPORT");
    }
    else
    {
        //! @todo this maight be bad if unknown not handled
        mpGUIConnectorAppearance->setType("UNKNOWN");
    }

    //Add arrow to the connector if it is of signal type
    if(mpEndPort->getPortType() == "READPORT" && mpEndPort->getNodeType() == "NodeSignal")
    {
        this->getLastLine()->addEndArrow();
    }
    else if(mpEndPort->getPortType() == "WRITEPORT" && mpEndPort->getNodeType() == "NodeSignal")
    {
        //Assumes that the startport was a read port
        mpLines[0]->addStartArrow();
    }

    //Run this to actually change the pen
    this->setPassive(); //!< @todo Not sure if setPassive is allways correct, but it is a good guess
}

//
////! Defines what shall happen if the line is selected or moved.
//QVariant GUIConnector::itemChange(GraphicsItemChange change, const QVariant &value)
//{
//
//    return value;
//}


//! Slot that deselects the connector. Used for signal-slot-connections.
void GUIConnector::deselect()
{
    this->setPassive();
}


//! Slot that selects the connector. Used for signal-slot-connections.
void GUIConnector::select()
{
    this->doSelect(true, -1);
}



//------------------------------------------------------------------------------------------------------------------------//


//! Constructor.
//! @param x1 is the x-coordinate of the start position of the line.
//! @param y1 is the y-coordinate of the start position of the line.
//! @param x2 is the x-coordinate of the end position of the line.
//! @param y2 is the y-coordinate of the end position of the line.
////! @param primaryPen defines the default color and width of the line.
////! @param activePen defines the color and width of the line when it is selected.
////! @param hoverPen defines the color and width of the line when it is hovered by the mouse cursor.
//! @param pConnApp A pointer to the connector appearance data, containing pens
//! @param lineNumber is the number of the line in the connector.
//! @param *parent is a pointer to the parent object.
GUIConnectorLine::GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, GUIConnectorAppearance* pConnApp, int lineNumber, GUIConnector *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    mpParentGUIConnector = parent;
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    mpConnectorAppearance = pConnApp;
    mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    mParentConnectorEndPortConnected = false;
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    //mpParentGUIConnector->mGeometries.push_back(HORIZONTAL);
    mHasStartArrow = false;
    mHasEndArrow = false;
    mArrowSize = 8.0;
    mArrowAngle = 0.5;
}


//! Destructor
GUIConnectorLine::~GUIConnectorLine()
{
}


//! Reimplementation of paint function. Removes the ugly dotted selection box.
void GUIConnectorLine::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*>(o);
    _o->state &= ~QStyle::State_Selected;
    QGraphicsLineItem::paint(p,_o,w);
}


//! Changes the style of the line to active.
//! @see setPassive()
//! @see setHovered()
void GUIConnectorLine::setActive()
{
        this->setPen(mpConnectorAppearance->getPen("Active"));
}


//! Changes the style of the line to default.
//! @see setActive()
//! @see setHovered()
void GUIConnectorLine::setPassive()
{
    if(!mpParentGUIConnector->isConnected())
    {
        this->setPen(mpConnectorAppearance->getPen("NonFinished"));
    }
    else
    {
        this->setPen(mpConnectorAppearance->getPen("Primary"));
    }
}


//! Changes the style of the line to hovered.
//! @see setActive()
//! @see setPassive()
void GUIConnectorLine::setHovered()
{
        this->setPen(mpConnectorAppearance->getPen("Hover"));
}


//! Defines what shall happen if a mouse key is pressed while hovering a connector line.
void GUIConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //emit lineClicked();
    if(event->button() == Qt::LeftButton)
    {
        mOldPos = this->pos();
    }
    QGraphicsLineItem::mousePressEvent(event);
}


//! Defines what shall happen if a mouse key is released while hovering a connector line.
void GUIConnectorLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if((this->pos() != mOldPos) and (event->button() == Qt::LeftButton))
    {
        mpParentGUIConnector->mpParentSystem->mUndoStack->newPost();
        mpParentGUIConnector->mpParentSystem->mpParentProjectTab->hasChanged();
        mpParentGUIConnector->mpParentSystem->mUndoStack->registerModifiedConnector(mOldPos, this->pos(), mpParentGUIConnector, getLineNumber());
    }
    QGraphicsLineItem::mouseReleaseEvent(event);
}

//! Devines what shall happen if the mouse cursor enters the line. Change cursor if the line is movable.
//! @see hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(this->flags().testFlag((QGraphicsItem::ItemIsMovable)))
    {
        if(mParentConnectorEndPortConnected && mpParentGUIConnector->getGeometry(getLineNumber()) == VERTICAL)
        {
            this->setCursor(Qt::SizeVerCursor);
        }
        else if(mParentConnectorEndPortConnected && mpParentGUIConnector->getGeometry(getLineNumber()) == HORIZONTAL)
        {
            this->setCursor(Qt::SizeHorCursor);
        }
    }
    emit lineHoverEnter();
}


//! Defines what shall happen when mouse cursor leaves the line.
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverLeave();
}


//! Returns the number of the line in the connector.
int GUIConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! Defines what shall happen if the line is selected or moved.
QVariant GUIConnectorLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        emit lineSelected(this->isSelected(), mLineNumber);
    }
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit lineMoved(mLineNumber);
    }
    return value;
}


//! Tells the line that its parent connector has been connected at both ends
void GUIConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}


//! Reimplementation of setLine; stores the start and end positions before changing them
//! @param x1 is the x-coordinate of the start position.
//! @param y1 is the y-coordinate of the start position.
//! @param x2 is the x-coordinate of the end position.
//! @param y2 is the y-coordinate of the end position.
void GUIConnectorLine::setLine(QPointF pos1, QPointF pos2)
{
    this->startPos = this->mapFromParent(pos1);
    this->endPos = this->mapFromParent(pos2);
    if(mHasEndArrow)
    {
        delete(mArrowLine1);
        delete(mArrowLine2);
        this->addEndArrow();
    }
    else if(mHasStartArrow)
    {
        delete(mArrowLine1);
        delete(mArrowLine2);
        this->addStartArrow();
    }
    QGraphicsLineItem::setLine(this->mapFromParent(pos1).x(),this->mapFromParent(pos1).y(),
                               this->mapFromParent(pos2).x(),this->mapFromParent(pos2).y());
}


//! Adds an arrow at the end of the line.
//! @see addStartArrow()
void GUIConnectorLine::addEndArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle+mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle-mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    mHasEndArrow = true;
}


//! Adds an arrow at the start of the line.
//! @see addEndArrow()
void GUIConnectorLine::addStartArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle+mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle-mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    mHasStartArrow = true;
}


//! Reimplementation of inherited setPen function to include arrow pen too.
void GUIConnectorLine::setPen (const QPen &pen)
{
    QGraphicsLineItem::setPen(pen);
    if(mHasStartArrow | mHasEndArrow)       //Update arrow lines two, but ignore dashes
    {
        QPen tempPen = this->pen();
        tempPen = QPen(tempPen.color(), tempPen.width(), Qt::SolidLine);
        mArrowLine1->setPen(tempPen);
        mArrowLine2->setPen(tempPen);
        mArrowLine1->line();
    }
}


////! Set function for all three pen styles (primary, active and hover).
//void GUIConnectorLine::setPens(QPen primaryPen, QPen activePen, QPen hoverPen)
//{
//    mPrimaryPen = primaryPen;
//    mActivePen = activePen;
//    mHoverPen = hoverPen;
//    this->setPassive();
//}

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
//! @file   UndoStack.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains classes for the undo stack and the undo widget (which displays the stack)
//!
//$Id$

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>
#include <QObject>
#include <QGridLayout>

#include <QDomElement>
#include <QDomDocument>

//Undo defines
#define UNDO_ADDEDOBJECT "addedobject"
#define UNDO_ADDEDCONNECTOR "addedconnector"
#define UNDO_DELETEDOBJECT "deletedobject"
#define UNDO_DELETEDCONNECTOR "deletedconnector"
#define UNDO_MOVEDOBJECT "movedobject"
#define UNDO_RENAME "rename"
#define UNDO_MODIFIEDCONNECTOR "modifiedconnector"
#define UNDO_ROTATE "rotate"
#define UNDO_VERTICALFLIP "verticalflip"
#define UNDO_HORIZONTALFLIP "horizontalflip"
#define UNDO_NAMEVISIBILITYCHANGE "namevisibilitychange"
#define UNDO_PASTE "paste"
#define UNDO_MOVEDMULTIPLE "movedmultiple"
#define UNDO_CUT "cut"
#define UNDO_CHANGEDPARAMETERS "changedparameters"
#define UNDO_HIDEALLNAMES "hideallnames"
#define UNDO_SHOWALLNAMES "showallnames"
#define UNDO_MOVEDWIDGET "movedwidget"
#define UNDO_MOVEDMULTIPLEWIDGETS "movedmultiplewidgets"
#define UNDO_ALIGNX "alignx"
#define UNDO_ALIGNY "aligny"
#define UNDO_DISTRIBUTEX "distributex"
#define UNDO_DISTRIBUTEY "distributey"
#define UNDO_DELETEDCONTAINERPORT "deletedcontainerport"
#define UNDO_DELETEDSUBSYSTEM "deletedsubsystem"
#define UNDO_ADDEDCONTAINERPORT "addedcontainerport"
#define UNDO_ADDEDSUBSYSTEM "addedsubsystem"
#define UNDO_MOVEDCONNECTOR "movedconnector"
#define UNDO_CHANGEDPARAMETER "changedparameter"
#define UNDO_ADDEDTEXTBOXWIDGET "addedtextboxwidget"
#define UNDO_DELETEDTEXTBOXWIDGET "deletedtextboxwidget"
#define UNDO_RESIZEDTEXTBOXWIDGET "resizedtextboxwidget"
#define UNDO_MODIFIEDTEXTBOXWIDGET "modifiedtextboxwidget"

    //Forward Declarations
class ModelObject;
class GraphicsView;
class Connector;
class MainWindow;
class ContainerObject;
class Widget;
class UndoWidget;

class UndoStack : public QObject
{
friend class UndoWidget;

public:
    UndoStack(ContainerObject *parentSystem);

    QDomElement toXml();
    void fromXml(QDomElement &undoElement);
    void clear(QString errorMsg = "");
    void newPost(QString type = "");
    void insertPost(QString str);
    void undoOneStep();
    void redoOneStep();

    void registerDeletedObject(ModelObject *item);
    void registerDeletedConnector(Connector *item);
    void registerAddedObject(ModelObject *item);
    void registerAddedConnector(Connector *item);
    void registerRenameObject(QString oldName, QString newName);
    void registerModifiedConnector(QPointF oldPos, QPointF newPos, Connector *item, int lineNumber);
    void registerMovedObject(QPointF oldPos, QPointF newPos, QString objectName);
    void registerRotatedObject(const QString objectName, const double angle);
    void registerVerticalFlip(QString objectName);
    void registerHorizontalFlip(QString objectName);
    void registerChangedParameter(QString objectName, QString parameterName, QString oldValueTxt, QString newValueTxt);
    void registerChangedStartValue(QString objectName, QString portName, QString parameterName, QString oldValueTxt, QString newValueTxt);
    void registerNameVisibilityChange(QString objectName, bool isVisible);

    void registerAddedWidget(Widget *item);
    void registerDeletedWidget(Widget *item);
    void registerMovedWidget(Widget *item, QPointF oldPos, QPointF newPos);
    void registerResizedTextBoxWidget(const int index, const double w_old, const double h_old, const double w_new, const double h_new, const QPointF oldPos, const QPointF newPos);
    void registerModifiedTextBoxWidget(Widget *pItem);

private:
    ContainerObject *mpParentContainerObject;
    int mCurrentStackPosition;

    void addTextboxwidget(const QDomElement &rStuffElement);
    void removeTextboxWidget(const QDomElement &rStuffElement);
    void modifyTextboxWidget(QDomElement &rStuffElement);

    QDomElement getCurrentPost();
    QDomDocument mDomDocument;
    QDomElement mUndoRoot;
};


#endif // UNDOSTACK_H

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

//#include "scheme.h"

//#include "qioport.h"

#include "schememode.h"
#include <QGraphicsScene>
#include "qport.h"
//#include "qconnection.h"


QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QColor;
class QGraphicsObject;
QT_END_NAMESPACE

class Port;
class QPort;
class QIOPort;
class SwitchingElement;
class QSwitchingElement;
class Connection;
class QConnection;
class DiagramItem;
class Scheme;
class Topology;
class PropertiesTable;

class DiagramScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum Mode { InsertItem, InsertLine, InsertText, MoveItem, InsertInputPort, InsertOutputPort };
//    enum SchemeMode { edit, run, lockSimulation };

    explicit DiagramScene(Topology* topology, QMenu *itemMenu = nullptr, QObject *parent = nullptr);

    static PropertiesTable* _propertiesTable;

    QString getReferenceString();

    Scheme* getScheme(){ return scheme; }
    void createScheme(QString refStr);
    QList<QString> checkScheme();
    void startScheme(QString portNumber, QString controlPacket);
    QPort* getQPorttoPort(Port* port);
    QIOPort* getInQIOPorttoPort(Port* port);
    QIOPort* getOutQIOPorttoPort(Port* port);

    void addConnectionToQConnection(Connection* conn, QConnection* qconn);


    QConnection *addConnection(QPort *startItem, QPort *endItem, bool inConnection);

    void deleteElement(QGraphicsItem* item);
    DiagramItem* addElement(QString _state);
    void updateElement(QMap<QString, QString> properties, DiagramItem *CurrentObject);

    QString getCurrStr();

    QList<Port*> getPathPorts() { return pathBuildPorts; }


    //DiagramItem* changeScheme(QString operation, QGraphicsItem* item = nullptr);

    /*
    QFont font() const { return myFont; }
    QColor textColor() const { return myTextColor; }
    QColor itemColor() const { return myItemColor; }
    QColor lineColor() const { return myLineColor; }
    void setLineColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setItemColor(const QColor &color);
    void setFont(const QFont &font);
    void addPort(Port *port);
    Mode getMode() const { return myMode; }
    QList<IOPort *> getInputPorts() { return inputPorts; }
    QList<IOPort *> getOutputPorts() { return outputPorts; }
    IOPort * getInputPort(int index) { return inputPorts[index]; }
    IOPort * getOutputPort(int index) { return outputPorts[index]; }
    QList<Arrow*> getArrows() { return arrows; }
    void addArrows(Arrow *arrow) { arrows.append(arrow); }
    QList<SwitchingElement* > getSwitchingElements(){ return switchingElements; }
    void addSwitchingElement(SwitchingElement *switchingElement) { switchingElements.append(switchingElement); }



    void clearPorts(){ ports.clear(); } //очистить выбранные порты для рисования линии
*/

public slots:
    void setState(QString _state);
    void addPortForConnectoin(QPort* port);
    void setSchemeMode(SchemeMode _mode, bool changeMode = true);
    //void setMode(Mode mode);
    //void editorLostFocus(DiagramTextItem *item);

    DiagramItem* changeScheme(QString operation = "", QGraphicsItem* item = nullptr, QMap<QString, QString> properties = QMap<QString, QString>());


signals:
    void changeSchemeMode(SchemeMode _mode);
    void schemeChanged(bool passed = false);

    void pathEnd();
 /*
   void itemInserted(SwitchingElement *item);
    void textInserted(QGraphicsTextItem *item);
    void itemSelected(QGraphicsItem *item);
    void IOPortSelected(IOPort* _port);
    void hidePropertiesPanel();
    void showPropertiesPanel();
*/

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

private:
    bool isItemChange(int type) const;
    QSwitchingElement* addSwitchingElement(QMap<QString, QString> properties = QMap<QString, QString>());
    QIOPort *addIOPort(bool inputPort, QMap<QString, QString> propertiesPort = QMap<QString, QString>());

    void generatePathCoordinates(QList<Connection*>& pathConnections, QList<QPointF>& pathCoordinates);
    void startTokenAnimation(QList<QPointF>& pathCoordinates, bool isBlock);

    void parseElementData(const QString& data,
                            QMap<QString, QString>& elementProperties,
                            QList<QMap<QString, QString>>& inputs,
                            QList<QMap<QString, QString>>& outputs);


    QMenu *myItemMenu;

    QString state;
    Mode myMode;

    bool leftButtonDown;
    QPointF startPoint;

    QFont myFont;
    QColor myTextColor;
    QColor myItemColor;
    QColor myLineColor;


    QHash<Port*, QPort*> qports;
    QHash<Port*, QIOPort*> inputQPorts;
    QHash<Port*, QIOPort*> outputQPorts;
    QHash<Connection*, QConnection*> qconnections;
    QHash<SwitchingElement*, QSwitchingElement*> qswitchingElements;

    QList<QPort *> qportsForConnection;

    Scheme* scheme;
    //Topology *topology;

    SchemeMode mode = SchemeMode::Edit;

    QList<Port*> pathBuildPorts;

};

#endif // DIAGRAMSCENE_H

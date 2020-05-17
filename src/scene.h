/*!
 * @file scene.h
 * @author Peter Koprda (xkoprd00)
 * @author Adam Múdry (xmudry01)
 * @brief Definition of scene functions
 */

#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QByteArray>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMap>
#include <QVector>
#include <QPushButton>
#include <QTimer>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QSet>
#include <cmath>
#include <algorithm>
#include <tuple>

#include "pathfinding.h"

/*!
 * \brief Implements the logic behing rendering and animating items on QGraphicsScene.
 * \details Loads data from JSON file into the program's datastructures, renders and moves them accordingly.
 * Manages mouse click events on the scene.
 */
class Scene : public QGraphicsScene
{
    Q_OBJECT
private:
    int speed = 1;                  ///< Constrols a speed of the simulation
    int countTime = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int waitStop = 3000;            ///< How much milliseconds a bus waits after arriving into the end station
    int waitBeforeStart = 10000;    ///< How much milliseconds a bus waits when leaving the start station after another bus leaves

    QJsonObject json;
    Pathfinding p;      ///< Variable for Pathfinding object
    QPen pen;

    bool editMode = false;              ///< Switch for enabling/disabling line editation mode
    line* selectedLine = nullptr;
    street* selectedStreet = nullptr;
    QVector<QString> routeEditTemp;     ///< Temporary variable for new line when in line edit mode

    KeyGen renderedItemsKeyGen; ///< Generates key for all rendered items stored in variable "renderedItems"
    KeyGen busKeyGen;           ///< Generates key for buses stored in variable "buses"

    QMap<QString, street> streets;                  ///< Stores all streets
    QMap<QString, stop> stops;                      ///< Stores all stops (key is the coordinate)
    QMap<std::tuple<int,int>, stop> stopsReversed;  ///< Stores all stops (key is the name)
    QMap<int, line> lines;                          ///< Stores all lines
    QMap<int, bus> buses;                           ///< Stores all buses
    QVector<std::tuple<int,int>> points;            ///< Stores all points on the map (which are not stations)
    QGraphicsItem * selectedItem = nullptr;
    QGraphicsItem * lastSelectedItem = nullptr;
    QMap<int,container> renderedItems;              ///< Stores all rendered items

    /*!
     * \brief loads background, lines and vehicles
     */
    void loadAll();

    /*!
     * \brief loads background
     */
    void loadBackground();

    /*!
     * \brief loads lines
     */
    void loadLines();

    /*!
     * \brief loads vehicles
     */
    void loadVehicles();

    /*!
     * \brief renders streets
     */
    void renderStreets();

    /*!
     * \brief renders stops
     */
    void renderStops();

    /*!
     * \brief renders lines
     */
    void renderLines();

    /*!
     * \brief renders vehicles
     */
    void renderVehicles();

    /*!
     * \brief resets vehicles
     */
    void resetVehicles();

    /*!
     * \brief sets new position for bus
     * \param bus
     * \param step
     */
    void setNewPosition(bus &bus, double step = 1);

    /*!
     * \brief gets new path for bus
     * \details uses the A* pathfinding algorithm declared in the "pathfinding.h" file
     * \param bus
     * \return path for a bus to follow
     */
    QVector<std::tuple<int,int,int,int>> getPath(bus &bus);

    /*!
     * \brief returns a name of a station where the bus is heading to
     * \param bus b
     * \return the name of next station
     */
    QString getBusHeadingTo(const bus &b);

public slots:

    /*!
     * \brief updates time
     */
    void updateTime();

    /*!
     * \brief resets time
     */
    void resetTime();

    /*!
     * \brief resets lines
     */
    void resetLines();

    /*!
     * \brief sets speed
     * \param s speed of the simulation
     */
    void setSpeed(int s);

    /*!
     * \brief sets traffic speed on the selected street
     * \param s speed
     */
    void setTraffic(int s);

    /*!
     * \brief simulates the scene
     * \param step speed of the simulation
     */
    void simulate(double step = 1);

public:
    /*!
     * \brief constuctor
     * \param parent
     * \param path
     * \param interval
     */
    explicit Scene(QObject *parent = nullptr, QString path = nullptr, int interval = 50);

    /*!
     * \brief destructor
     */
    ~Scene();

    int interval_ms;

    /*!
     * \brief returns time in seconds
     * \return time
     */
    int getTime();
    QTimer *timer;

    /*!
     * \brief gets buses from scene
     * \return buses
     */
    QMap<int, bus> getBuses();

    /*!
     * \brief gets info about specific bus
     * \param key id of the bus
     * \return info about bus
     */
    QString getBusInfo(int key);

    /*!
     * \brief gets lines from scene
     * \return lines
     */
    QMap<int, line> getLines();

    /*!
     * \brief gets info about specific line
     * \param key id of the line
     * \return line info
     */
    QString getLineInfo(int key);

    /*!
     * \brief gets streets from the scene
     * \return streets
     */
    QMap<QString, street> getStreets();

    /*!
     * \brief gets name of the specific street
     * \param key id of the street
     * \return street name
     */
    QString getStreetInfo(QString key);

    /*!
     * \brief gets selected line
     * \return selected line
     */
    line* getSelectedLine();

    /*!
     * \brief blocks specific street
     * \param key id of the street
     * \return true if the street is already blocked, otherwise false
     */
    bool blockStreet(QString key);

    /*!
     * \brief unblocks specific street
     * \param key id of the street
     * \return true if the street is already unblocked, otherwise true
     */
    bool unblockStreet(QString key);

    /*!
     * \brief show path of the line
     * \param key
     */
    void showLine(int key);

    /*!
     * \brief hide path of the line
     */
    void hideLines();

    /*!
     * \brief selects path
     * \param key id of the bus
     * \return true if the bus has path, otherwise false
     */
    bool selectLine(int key);

    /*!
     * \brief setEditMode
     * \param val
     */
    void setEditMode(bool val);

    /*!
     * \brief getEditMode
     * \return edit mode
     */
    bool getEditMode();

    /*!
     * \brief selects line
     * \param key
     * \return true if the bus is on the path, otherwise false
     */
    bool selectLineViaBus(int key);

    /*!
     * \brief deselects line
     */
    void deselectLine();

    /*!
     * \brief selectStreet
     * \param key
     * \return true if the line is on street, otherwise false
     */
    bool selectStreet(QString key);

    /*!
     * \brief deselects street
     */
    void deselectStreet();

    /*!
     * \brief hides buses
     * \param val
     */
    void hideBuses(bool val);

    /*!
     * \brief saves edited route
     * \return true if the route has two or more edited streets, otherwise false
     */
    bool saveEdit();


protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void valueChanged(int newValue);
    void timeValueChanged(QString newValue);
    void infoLabelChanged(QString newValue);
    void trafficEnabledChanged(bool newValue);
    void lineEditEnabledChanged(bool newValue);
    void trafficValueChanged(int newValue);

};

#endif // SCENE_H

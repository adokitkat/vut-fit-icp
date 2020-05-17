/*!
 * @file datastructures.h
 * @author Peter Koprda (xkoprd00)
 * @author Adam Múdry (xmudry01)
 * @brief Data structures
 */

#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <QGraphicsScene>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QString>
#include <QString>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>
#include <QMetaType>
#include <QPushButton>
#include <tuple>

/*!
 * \brief The bus struct
 * \details Stores all information about a certain bus.
 */
struct bus{
    int no;
    int lineno;
    double pos_x;
    double pos_y;
    double d;               ///< Used for a movement calculation
    bool reversed = false;  ///< True if bus is going back to start station
    bool halt = false;      ///< If the calculated route is not correct, halt (stop) all buses on the line
    int slow = 1;           ///< Higher the number, slower the bus moves
    int wait = 0;
    int initWait = 0;
    QString startStation;
    QString lastStation;
    QString headingStation;
    QString endStation;
    QString currStreet;
    QVector<std::tuple<int,int>> visited;       ///< Contains last visited points on map
    QVector<std::tuple<int,int,int,int>> path;  ///< Path which rendered bus dot is following
    QGraphicsItemGroup * renderedItem = nullptr;
};
Q_DECLARE_METATYPE(bus);

/*!
 * \brief The line struct
 * \details Stores all information about a certain line.
 */
struct line{
    int no;
    QString color;
    QString start;
    QString startOriginal;
    QVector<QString> stopsAt;
    QVector<QString> stopsAtOriginal;
    QString end;
    QString endOriginal;
    QVector<std::tuple<int,int,int,int>> pathLines;
    QGraphicsItemGroup * renderedPath = nullptr;
};
Q_DECLARE_METATYPE(line);

/*!
 * \brief The street struct
 * \details Stores all information about a certain street.
 */
struct street{
    QString name;
    int traffic = 1;        ///< Controls how fast is trafic on the street (higher the number, the slower buses on the street are)
    bool isBlocked = false;
    QVector<std::tuple<int,int>> mid;
    QVector<std::tuple<int,int,int,int>> pathLines;
    QGraphicsItemGroup * renderedPath = nullptr;
};
Q_DECLARE_METATYPE(street);

/*!
 * \brief The stop struct
 * \details Stores all information about a certain stop.
 */
struct stop{
    QString name;
    QVector<int> linesNo;
    std::tuple<int,int> coord;
    QGraphicsItemGroup * rendered = nullptr;
};
Q_DECLARE_METATYPE(stop);

/*!
 * \brief The container struct
 * \details Used to connect rendered item and data structures.
 */
struct container{
    QString type = "";
    QString stringKey = "";
    int intKey = 0;
};
Q_DECLARE_METATYPE(container);

/*!
 * \brief The KeyGen class
 * \details Simple class for generating integer keys.
 */
class KeyGen
{
private:
    int generatedKey = INT_MIN;
public:
    /*!
     * \brief KeyGen::gen()
     * \return
     */
    int gen() {
        return generatedKey++;
    }
};

#endif // DATASTRUCTURES_H

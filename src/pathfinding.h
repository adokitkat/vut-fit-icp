/*!
 * @file pathfinding.h
 * @author Peter Koprda (xkoprd00)
 * @author Adam Múdry (xmudry01)
 * @brief Definition of path finding
 */

#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <tuple>
#include <cmath>
#include <list>
#include <vector>

#include "datastructures.h"

/*!
 * \brief Implementation of the A* pathfinding algorithm
 * \details Creates a graph from nodes from given input of points and paths between them and
 * performs the A* algorithm to find the shortest route for given start and end nodes.
 * The resulting path is used by a line and buses following the line.
 */
class Pathfinding : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief The Node structure
     */
    struct Node {
        bool obstacle = false;
        bool visited = false;
        float globalGoal;
        float localGoal;
        int x;
        int y;
        QVector<Node*> neighbours;
        Node* parent;
    };

    /*!
     * \brief constructor
     * \param parent
     */
    explicit Pathfinding(QObject *parent = nullptr);

    /*!
     * \brief loads points from map into nodes
     * \param points
     */
    void loadPoints(const QVector<std::tuple<int,int>>& points);

    /*!
     * \brief resets nodes
     */
    void resetPoints();

    /*!
     * \brief loads paths
     * \details loads paths (connections between nodes) from map and creates extra nodes in middle of each path, those extra points are also returned to the "points" variable
     * \param streets
     * \param points
     */
    void loadPaths(QMap<QString, street>& streets, QVector<std::tuple<int,int>>& points);

    /*!
     * \brief loads a goal
     * \details loads a goal (start & end points) for a pathfinding algorithm to find a round between
     * \param start node coordinates
     * \param end node coordinates
     */
    void loadGoal(std::tuple<int,int> start, std::tuple<int,int> end);

    /*!
     * \brief finds the route between start & end points
     * \return
     */
    bool solveAStar();

    /*!
     * \brief returns the solution calculated by solveAStart() method
     * \return
     */
    QVector<std::tuple<int,int>> getSolution();

    /*!
     * \brief debugging method used to return info about created graph
     * \return
     */
    QMap<std::tuple<int,int>, Node> returnGraph();

    /*!
     * \brief sets/unsets node as an obstacle
     * \param node's coordinates
     * \param true if set node as obstacle, false if unset node as obstcle
     * \return true if succeeds, otherwise false
     */
    bool setNodeObstacle(std::tuple<int,int> point, bool obstacle);

private:
    QMap<std::tuple<int,int>, Node> nodesMap;   ///< container for all nodes (key is nodes coordinates)
    int nodeNum = 0;
    Node *nodeStart = nullptr;
    Node *nodeEnd = nullptr;

};

#endif // PATHFINDING_H

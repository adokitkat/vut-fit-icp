/*!
 * @file pathfinding.cpp
 * @author Peter Koprda (xkoprd00)
 * @author Adam Múdry (xmudry01)
 * @brief Find path to bus stop
 */

#include "pathfinding.h"

Pathfinding::Pathfinding(QObject *parent) : QObject(parent) {}


void Pathfinding::loadPoints(const QVector<std::tuple<int,int>>& points)
{
    int i = 0;
    for (const auto& point : points)
    {
        if (nodesMap.contains(point)) continue;

        Node node;
        node.x = std::get<0>(point);
        node.y = std::get<1>(point);
        node.obstacle = false;
        node.visited = false;
        node.parent = nullptr;

        nodesMap.insert(point, node);
        ++nodeNum;
        ++i;
    }
}


void Pathfinding::resetPoints()
{
    for (auto& node : nodesMap)
    {
        node.visited = false;
        node.obstacle = false;
        node.parent = nullptr;
    }
}


void Pathfinding::loadPaths(QMap<QString, street>& streets, QVector<std::tuple<int,int>>& points)
{
    for (auto& street : streets)
    {
        for (const auto& path : street.pathLines)
        {
            auto x1 = std::get<0>(path);
            auto y1 = std::get<1>(path);
            auto x2 = std::get<2>(path);
            auto y2 = std::get<3>(path);

            // adds extra node inbetween nodes which are going to be connected to allow block/unblock street functionality
            for (auto& node : nodesMap)
            {
                if (node.x == x1 and node.y == y1) {
                    auto start_node_coord = std::tuple<int,int>(node.x, node.y);
                    auto end_node_coord = std::tuple<int,int>(x2, y2);

                    int new_x;
                    int new_y;

                    if (node.x >= x2)
                        new_x = (node.x - x2)/2 + x2;
                    else
                        new_x = (x2 - node.x)/2 + node.x;

                    if (node.y >= y2)
                        new_y = (node.y - y2)/2 + y2;
                    else
                        new_y = (y2 - node.y)/2 + node.y;

                    auto new_coord = std::tuple<int,int>(new_x, new_y);

                    if (!nodesMap.contains(new_coord)) {
                        Node new_node;
                        new_node.x = new_x;
                        new_node.y = new_y;
                        node.obstacle = false;
                        node.visited = false;
                        node.parent = nullptr;

                        if (!points.contains(new_coord))
                            points.push_back(new_coord);

                        if (!street.mid.contains(new_coord))
                            street.mid.push_back(new_coord);

                        new_node.neighbours.push_back(&nodesMap[start_node_coord]);
                        new_node.neighbours.push_back(&nodesMap[end_node_coord]);

                        nodesMap.insert(new_coord, new_node);
                        ++nodeNum;
                    }

                    node.neighbours.push_back(&nodesMap[new_coord]);


                }
                else if (node.x == x2 and node.y == y2) {
                    auto start_node_coord = std::tuple<int,int>(node.x, node.y);
                    auto end_node_coord = std::tuple<int,int>(x1, y1);

                    int new_x;
                    int new_y;

                    if (node.x >= x1)
                        new_x = (node.x - x1)/2 + x1;
                    else
                        new_x = (x1 - node.x)/2 + node.x;

                    if (node.y >= y1)
                        new_y = (node.y - y1)/2 + y1;
                    else
                        new_y = (y1 - node.y)/2 + node.y;

                    auto new_coord = std::tuple<int,int>(new_x, new_y);

                    if (!nodesMap.contains(new_coord)) {
                        Node new_node;
                        new_node.x = new_x;
                        new_node.y = new_y;
                        node.obstacle = false;
                        node.visited = false;
                        node.parent = nullptr;

                        if (!points.contains(new_coord))
                            points.push_back(new_coord);

                        if (!street.mid.contains(new_coord))
                            street.mid.push_back(new_coord);

                        new_node.neighbours.push_back(&nodesMap[start_node_coord]);
                        new_node.neighbours.push_back(&nodesMap[end_node_coord]);

                        nodesMap.insert(new_coord, new_node);
                        ++nodeNum;
                    }

                    node.neighbours.push_back(&nodesMap[new_coord]);
                }
            }
        }
    }
}


bool Pathfinding::setNodeObstacle(std::tuple<int,int> point, bool obstacle)
{
    auto x = std::get<0>(point);
    auto y = std::get<1>(point);

    for (auto& node : nodesMap)
    {
        if (node.x == x and node.y == y)
        {
            if (obstacle)
                node.obstacle = true;
            else
                node.obstacle = false;
            return true;
        }
    }
    return false;
}


QVector<std::tuple<int,int>> Pathfinding::getSolution()
{
    QVector<std::tuple<int,int>> solution;
    if (nodeEnd != nullptr)
    {
        Node *p = nodeEnd;
        while (p->parent != nullptr)
        {
            solution.push_front(std::tuple<int,int>(p->x,p->y));
            p = p->parent;
        }
        solution.push_front(std::tuple<int,int>(p->x,p->y));
    }
    return solution;
}


QMap<std::tuple<int, int>, Pathfinding::Node> Pathfinding::returnGraph()
{
    return nodesMap;
}


void Pathfinding::loadGoal(std::tuple<int, int> start, std::tuple<int, int> end)
{
    nodeStart = &nodesMap[start];
    nodeEnd = &nodesMap[end];
}


bool Pathfinding::solveAStar()
{
    for (auto& node : nodesMap)
    {
        node.visited = false;
        node.parent = nullptr;
        node.globalGoal = INFINITY;
        node.localGoal = INFINITY;
    }

    auto distance = [](Node *a, Node *b) {return sqrtf(powf(a->x - b->x, 2) + powf(a->y - b->y, 2));};
    auto heuristic = [distance](Node *a, Node *b) {return distance(a, b);};

    Node *nodeCurrent = nodeStart;
    nodeStart->localGoal = 0.0f;
    nodeStart->globalGoal = heuristic(nodeStart, nodeEnd);

    std::list<Node*> notTestedNodes;
    notTestedNodes.push_back(nodeStart);

    while (!notTestedNodes.empty()) {
        notTestedNodes.sort([](const Node* lhs, const Node* rhs) {return lhs->globalGoal < rhs->globalGoal;});

        while (!notTestedNodes.empty() and notTestedNodes.front()->visited)
        {
            notTestedNodes.pop_front();
        }

        if (notTestedNodes.empty()) break;

        nodeCurrent = notTestedNodes.front();
        nodeCurrent->visited = true;

        for (auto nodeNeighbour : nodeCurrent->neighbours)
        {
            if (!nodeNeighbour->visited and nodeNeighbour->obstacle == false)
                notTestedNodes.push_back(nodeNeighbour);

            float possiblyLowerGoal = nodeCurrent->localGoal + distance(nodeCurrent, nodeNeighbour);

            if (possiblyLowerGoal < nodeNeighbour->localGoal)
            {
                nodeNeighbour->parent = nodeCurrent;
                nodeNeighbour->localGoal = possiblyLowerGoal;
                nodeNeighbour->globalGoal = nodeNeighbour->localGoal + heuristic(nodeNeighbour, nodeEnd);
            }
        }
    }

    return true;
}

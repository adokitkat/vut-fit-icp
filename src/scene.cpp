/*!
 * @file scene.cpp
 * @author Peter Koprda (xkoprd00)
 * @author Adam Múdry (xmudry01)
 * @brief Creating scene
 */

#include "scene.h"

Scene::Scene(QObject *parent, QString path, int interval) : QGraphicsScene(parent)
{
    interval_ms = interval;
    // JSON file to JSON object
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    QByteArray rawData = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(rawData));
    json = doc.object();

    loadAll();
    renderStreets();
    renderStops();
    renderVehicles();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(simulate()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start(interval_ms);
}


Scene::~Scene()
{
    for (auto &bus : buses)
    {
        if (bus.renderedItem)
            delete bus.renderedItem;
    }

    for (auto &line : lines)
    {
        if (line.renderedPath)
            delete line.renderedPath;
    }

    for (auto &street : streets)
    {
        if (street.renderedPath)
            delete street.renderedPath;
    }

    for (auto &stop : stops)
    {
        if (stop.rendered)
            delete stop.rendered;
    }
}


QMap<int, bus> Scene::getBuses()
{
    return buses;
}


QMap<int, line> Scene::getLines()
{
    return lines;
}


QMap<QString, street> Scene::getStreets()
{
    return streets;
}


void Scene::updateTime()
{
    static QString val;
    if (speed != 0) {
        countTime += interval_ms;

        if (countTime >= 1000) {
            countTime = 0;
            ++seconds;

            if (seconds >= 60)
            {
                seconds = 0;
                ++minutes;

                if (minutes >= 60)
                {
                    minutes = 0;
                    ++hours;

                    if (hours >= 24)
                    {
                        hours = 0;
                    }
                }
            }
            val = QString::number(hours).rightJustified(2, '0') + QString(":") +
                  QString::number(minutes).rightJustified(2, '0') + QString(":") +
                  QString::number(seconds).rightJustified(2, '0');
            emit timeValueChanged(val);
        }
    }
}


void Scene::resetTime()
{
    countTime = 0;
    hours = 0;
    minutes = 0;
    seconds = 0;
    resetVehicles();

    for (auto& line : lines)
    {
        delete line.renderedPath;
        line.renderedPath = new QGraphicsItemGroup;
    }
    renderLines();

    for (auto &bus : buses)
    {
        bus.reversed = true;
        getPath(bus);
        bus.reversed = false;
    }
    renderLines();
}


int Scene::getTime()
{
    return hours*3600 + minutes*60 + seconds;
}


void Scene::setSpeed(int s)
{
    if (speed != s) {
        speed = s;
        emit valueChanged(s);
    }
}


void Scene::setTraffic(int s)
{
    if (selectedStreet)
    {
        selectedStreet->traffic = s;
    }
}


void Scene::showLine(int key)
{
    if (!lines.contains(key)) return;
    const auto& l = lines[key];
    l.renderedPath->setZValue(1);
    for (const auto& line : lines)
    {
        if (l.no != line.no and l.renderedPath->zValue() <= line.renderedPath->zValue())
        {
            l.renderedPath->setZValue(1);
            line.renderedPath->setZValue(0);
            line.renderedPath->hide();
        }
    }

    bool show = true;
    for (const auto& bus : buses)
    {
        if (bus.lineno == l.no and bus.halt)
            show = false;
    }
    if (show)
        l.renderedPath->show();
}


void Scene::hideLines()
{
    for (const auto& line : lines)
    {
        line.renderedPath->hide();
    }
}


bool Scene::selectLine(int key)
{
   if (lines.contains(key)) {
        selectedLine = &lines[key];
        return true;
   } else
        return false;
}


void Scene::deselectLine()
{
    selectedLine = nullptr;
}


line* Scene::getSelectedLine()
{
    return selectedLine;
}


bool Scene::selectLineViaBus(int key)
{
   if (!buses.contains(key)) return false;

   if (lines.contains(buses[key].lineno)) {
        selectedLine = &lines[buses[key].lineno];
        return true;
   } else
        return false;
}


bool Scene::selectStreet(QString key)
{
   if (streets.contains(key)) {
        selectedStreet = &streets[key];
        return true;
   } else
        return false;
}


void Scene::deselectStreet()
{
    selectedStreet = nullptr;
}


void Scene::hideBuses(bool val)
{
    for (const auto& b : buses)
    {
        if (val)
            b.renderedItem->hide();
        else
            b.renderedItem->show();
    }
}


void Scene::setEditMode(bool val)
{
    editMode = val;
    if (!val) deselectLine();
}


bool Scene::getEditMode()
{
    return editMode;
}


QString Scene::getLineInfo(int key)
{
    if (!lines.contains(key)) return "No info";
    const auto& l = lines[key];

    auto result = QString("Line no. %1 -- Goes through: %2").arg(QString::number(l.no), l.start);
    for (const auto& x: l.stopsAt)
    {
        result += QString(" - %1").arg(x);
    }
    result += QString(" - %1").arg(l.end);

    showLine(key);
    return result;
}


QString Scene::getBusInfo(int key)
{
    if (!buses.contains(key)) return "No info";
    const auto& b = buses[key];
    auto result = QString("Bus no. %1 -- Line no. %2 -- On street: %3 -- Start station: %4 -- ").arg(QString::number(b.no), QString::number(b.lineno), b.currStreet, b.startStation)
                  + QString("End station: %1 -- Last station: %2 -- Heading to: %3").arg(b.endStation, b.lastStation, b.headingStation);
    showLine(b.lineno);

    if (b.renderedItem)
        b.renderedItem->setSelected(true);

    return result;
}


QString Scene::getBusHeadingTo(const bus& b)
{
    QString headingTo;
    int index = -1;

    auto startStation = lines[b.lineno].start;
    auto endStation = lines[b.lineno].end;
    auto stopsAt = lines[b.lineno].stopsAt;

    if (b.reversed) {
        swap(startStation, endStation);
        std::reverse(stopsAt.begin(), stopsAt.end());
    }

    if (b.lastStation == startStation)
    {
        if (!stopsAt.empty())
            headingTo = stopsAt.first();
        else
            headingTo = endStation;
    }
    else if (b.lastStation == endStation)
    {
        if (!stopsAt.empty())
            headingTo = stopsAt.last();
        else
            headingTo = startStation;
    }
    else
    {
        index = stopsAt.indexOf(b.lastStation);
        if (index != -1)
        {
            if (stopsAt[index] == stopsAt.last())
                headingTo = b.endStation;
            else
                headingTo = stopsAt[index + 1];

        }
        else {
            headingTo = "unknown";
        }

    }
    return headingTo;
}


QString Scene::getStreetInfo(QString key)
{
    if (!streets.contains(key)) return "No info";
    auto& s = streets[key];

    emit trafficValueChanged(s.traffic);

    if (s.renderedPath)
        s.renderedPath->setSelected(true);

    return s.name;
}


bool Scene::blockStreet(QString key)
{
    QColor gray90 = Qt::black;
    gray90.setAlphaF(0.9);
    pen.setBrush(gray90);
    pen.setWidth(3);

    for (auto& street : streets)
    {
        if(street.name == key)
        {
            if(street.isBlocked)
                return true;

            for (const auto& line : street.pathLines)
            {
                this->addLine(std::get<0>(line), std::get<1>(line),std::get<2>(line), std::get<3>(line), pen);
            }

            for (const auto& point : street.mid)
            {
                p.setNodeObstacle(point, true);
            }

            street.isBlocked = true;
            return true;
        }
    }
    return false;
}


bool Scene::unblockStreet(QString key)
{
    pen.setColor(Qt::darkGray);
    pen.setWidth(3);

    for (auto& street : streets)
    {
        if(street.name == key)
        {
            if(!street.isBlocked)
                return true;

            for (const auto& line : street.pathLines)
            {
                this->addLine(std::get<0>(line), std::get<1>(line),std::get<2>(line), std::get<3>(line), pen);
            }

            for (const auto& point : street.mid)
            {
                p.setNodeObstacle(point, false);
            }

            street.isBlocked = false;
            return true;
        }
    }
    return false;
}


void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if((event->modifiers() & Qt::ControlModifier))
    {
        event->ignore();
    }
    else
    {
        QGraphicsScene::mousePressEvent(event);

        if (!this->selectedItems().empty())
        {
            lastSelectedItem = selectedItem;
            selectedItem = this->selectedItems().first();

            auto key = selectedItem->data(0);
            auto cont = renderedItems[key.toInt()];

            if (cont.type != "")
            {
                QString data;
                trafficEnabledChanged(false);

                if (cont.type == "street") {
                    data = getStreetInfo(cont.stringKey);

                    if (editMode == 0) {

                        selectStreet(cont.stringKey);

                        emit trafficEnabledChanged(true);
                        hideLines();
                        emit infoLabelChanged(data);
                    }

                } else if (cont.type == "stop") {
                    data = cont.stringKey;
                    deselectStreet();

                    if (editMode == false) {
                        hideLines();

                    } else if (editMode == true) {

                        if (!routeEditTemp.contains(data))
                            routeEditTemp.push_back(data);
                    }

                    emit infoLabelChanged(data);

                } else if (cont.type == "line") {
                    data = getLineInfo(cont.intKey);
                    deselectStreet();
                    if (editMode == 0) {

                        selectLine(cont.intKey);

                        emit lineEditEnabledChanged(true);
                        emit infoLabelChanged(data);
                    }

                } else if (cont.type == "bus") {
                    data = getBusInfo(cont.intKey);
                    deselectStreet();
                    if (editMode == 0) {

                        selectLineViaBus(cont.intKey);

                        emit lineEditEnabledChanged(true);
                        emit infoLabelChanged(data);
                    }
                }

            }
        } else {
            if (editMode == 0) {
                selectedItem = nullptr;
                deselectLine();
                deselectStreet();
                emit infoLabelChanged("Info (select item)");
                emit trafficEnabledChanged(false);
                emit lineEditEnabledChanged(false);
                hideLines();
            }
        }
    }
}


bool Scene::saveEdit()
{
    if (routeEditTemp.size() < 2 or !selectedLine) {
        deselectLine();
        return false;
    }

    auto start = routeEditTemp.first();
    auto end = routeEditTemp.last();

    routeEditTemp.pop_front();
    routeEditTemp.pop_back();

    selectedLine->start = start;
    selectedLine->end = end;
    selectedLine->stopsAt = routeEditTemp;

    routeEditTemp = QVector<QString>();

    for (auto &bus : buses)
    {
        if (bus.lineno == selectedLine->no) {
            bus.startStation = start;
            bus.lastStation = start;
            bus.endStation = end;

            bus.pos_x = std::get<0>(stops[start].coord);
            bus.pos_y = std::get<1>(stops[start].coord);
            bus.visited = QVector<std::tuple<int,int>>();
            bus.slow = 0;
            bus.wait = bus.initWait;
            bus.reversed = false;
            bus.path = getPath(bus);
        }
    }

    delete selectedLine->renderedPath;
    selectedLine->renderedPath = new QGraphicsItemGroup;

    deselectLine();
    return true;
}


void Scene::loadAll()
{
    loadBackground();
    loadLines();
    loadVehicles();

    p.loadPoints(points);
    p.loadPaths(streets, points);

    // render lines from both sides (may have different route)
    for (auto &bus : buses)
    {
        bus.reversed = true;
        getPath(bus);
    }
    renderLines();

    for (auto &bus : buses)
    {
        bus.reversed = false;
        bus.path = getPath(bus);
    }
    renderLines();
    //
}


void Scene::loadBackground()
{
    for (auto element : json["stops"].toArray())
    {
        auto stopObj = element.toObject();
        stop stopStruct;

        auto point = std::tuple<int,int>(stopObj["x"].toInt(), stopObj["y"].toInt());
        stopStruct.coord = point;
        stopStruct.name = stopObj["name"].toString();

        stops.insert(stopStruct.name, stopStruct);
        stopsReversed.insert(point, stopStruct);
        points.push_back(point);
    }

    for (auto element : json["streets"].toArray())
    {
        auto streetObj = element.toObject();
        street streetStruct;
        QVector<std::tuple<int,int,int,int>> pathLines;

        auto start = std::tuple<int,int>(streetObj["start"].toArray()[0].toInt(), streetObj["start"].toArray()[1].toInt());
        auto end = std::tuple<int,int>(streetObj["end"].toArray()[0].toInt(), streetObj["end"].toArray()[1].toInt());

        if (!points.contains(start))
            points.push_back(start);

        for (auto pos : streetObj["mid"].toArray())
        {
            auto mid = std::tuple<int,int>(pos.toArray()[0].toInt(), pos.toArray()[1].toInt());
            pathLines.push_back(std::tuple_cat(start, mid));
            start = mid;

            if (!points.contains(mid))
                points.push_back(mid);
        }
        if (!points.contains(end))
            points.push_back(end);

        pathLines.push_back(std::tuple_cat(start, end));

        streetStruct.name = streetObj["name"].toString();
        streetStruct.pathLines = pathLines;
        streets.insert(streetObj["name"].toString(), streetStruct);
    }
}


void Scene::loadLines()
{
    for (auto element : json["lines"].toArray())
    {
        auto lineObj = element.toObject();
        QVector<QString> stopsAt;
        QVector<std::tuple<int,int,int,int>> pathLines;

        auto start = lineObj["start"].toString();
        auto end = lineObj["end"].toString();

        for (auto goesThrough : lineObj["goes"].toArray())
        {
            stopsAt.push_back(goesThrough.toString());
        }

        line l {lineObj["no"].toInt(), lineObj["color"].toString(), start, start, stopsAt, stopsAt, end, end, pathLines, nullptr};
        lines.insert(lineObj["no"].toInt(), l);
    }
}


void Scene::renderStreets()
{
    QGraphicsItemGroup * renderedStreets;
    pen.setWidth(3);
    pen.setColor(Qt::darkGray);
    for (auto& street : streets)
    {
        renderedStreets = new QGraphicsItemGroup;
        container c;
        c.type = "street";
        for (const auto& line : street.pathLines)
        {
            auto streetLine = this->addLine(std::get<0>(line), std::get<1>(line),std::get<2>(line), std::get<3>(line), pen);
            streetLine->setToolTip(street.name);
            renderedStreets->addToGroup(streetLine);
        }
        renderedStreets->setFlag(QGraphicsItem::ItemIsSelectable);

        auto itemKey = renderedItemsKeyGen.gen();
        renderedStreets->setData(0, itemKey);
        this->addItem(renderedStreets);

        renderedStreets->setHandlesChildEvents(false);

        street.renderedPath = renderedStreets;

        c.stringKey = street.name;
        renderedItems.insert(itemKey, c);
    }
}


void Scene::renderStops()
{
    QGraphicsItemGroup * renderedStop;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    for (auto& stop : stops)
    {
        renderedStop = new QGraphicsItemGroup;
        container c;
        c.type = "stop";

        auto dot = this->addEllipse(std::get<0>(stop.coord)-9, std::get<1>(stop.coord)-9, 18, 18, pen, QBrush(Qt::white));
        auto label = this->addText(stop.name);
        label->setPos(std::get<0>(stop.coord)+6, std::get<1>(stop.coord)+2);

        renderedStop->setPos(std::get<0>(stop.coord), std::get<1>(stop.coord));
        renderedStop->addToGroup(dot);
        renderedStop->addToGroup(label);
        renderedStop->setFlag(QGraphicsItem::ItemIsSelectable);

        auto itemKey = renderedItemsKeyGen.gen();
        renderedStop->setData(0, itemKey);
        renderedStop->setZValue(2);

        stop.rendered = renderedStop;
        this->addItem(renderedStop);

        c.stringKey = stop.name;
        renderedItems.insert(itemKey, c);
    }
}


void Scene::renderLines()
{
    QGraphicsItemGroup * renderedLines;
    pen.setWidth(3);
    for (auto& line : lines)
    {
        if (!line.renderedPath)
            renderedLines = new QGraphicsItemGroup;
        else
            renderedLines = line.renderedPath;
        pen.setColor(line.color);

        for (const auto& path : line.pathLines)
        {
            auto lineDrawn = this->addLine(std::get<0>(path), std::get<1>(path),std::get<2>(path), std::get<3>(path), pen);
            renderedLines->addToGroup(lineDrawn);
        }
        renderedLines->hide();
        line.renderedPath = renderedLines;

        if (!this->items().contains(renderedLines))
            this->addItem(renderedLines);
    }
}


void Scene::resetLines()
{
    for (auto& line : lines)
    {
        line.start = line.startOriginal;
        line.stopsAt = line.stopsAtOriginal;
        line.end = line.endOriginal;
        line.pathLines = QVector<std::tuple<int,int,int,int>>();
        delete line.renderedPath;
        line.renderedPath = new QGraphicsItemGroup;
    }
    resetVehicles();
    renderLines();
}


void Scene::loadVehicles()
{
    auto nextBus = waitBeforeStart;
    for (const auto element : json["buses"].toArray())
    {
        auto busObj = element.toObject();
        QVector<std::tuple<int,int>> visited;
        QVector<std::tuple<int,int,int,int>> path;

        auto pos = stops[lines[busObj["lineno"].toInt()].start];
        auto startX = double(std::get<0>(pos.coord));
        auto startY = double(std::get<1>(pos.coord));
        auto startStation = lines[busObj["lineno"].toInt()].start;
        auto endStation = lines[busObj["lineno"].toInt()].end;
        auto startAt = busObj["startat"].toInt()*1000;

        for (int i = 0; i < 10; ++i)
        {
            auto key = busKeyGen.gen();
            bus b {busObj["no"].toInt(), busObj["lineno"].toInt(), startX, startY, 0.0, false, false, 1,
                        startAt + nextBus*i, startAt + nextBus*i, startStation, startStation, "", endStation, "", visited, path, nullptr};
            b.headingStation = getBusHeadingTo(b);
            buses.insert(key, b);
        }
    }
}


void Scene::resetVehicles()
{
    for (auto& bus : buses)
    {
        const auto& stop = stops[lines[bus.lineno].start];
        bus.pos_x = double(std::get<0>(stop.coord));
        bus.pos_y = double(std::get<1>(stop.coord));
        bus.d = 0.0f;
        bus.reversed = false;
        bus.startStation = lines[bus.lineno].start;
        bus.lastStation = lines[bus.lineno].start;
        bus.endStation = lines[bus.lineno].end;
        bus.headingStation = "";
        bus.currStreet = "";
        bus.visited = QVector<std::tuple<int,int>>();
        bus.halt = false;
        bus.slow = 1;
        bus.path = getPath(bus);
        bus.wait = bus.initWait;
        bus.renderedItem->setX(bus.pos_x);
        bus.renderedItem->setY(bus.pos_y);
    }
}


void Scene::renderVehicles()
{
    pen.setWidth(3);
    pen.setColor(Qt::black);

    for (auto& key : buses.keys())
    {
        QGraphicsItemGroup * renderedItem = new QGraphicsItemGroup;
        container c;
        c.type = "bus";

        pen.setColor(lines[buses[key].lineno].color);
        auto busDot = this->addEllipse(buses[key].pos_x-6, buses[key].pos_y-6, 12, 12, pen, QBrush(Qt::white));
        auto label = this->addText(QString::number(buses[key].no));
        label->setPos(buses[key].pos_x-12, buses[key].pos_y-30);

        renderedItem->setPos(buses[key].pos_x, buses[key].pos_y);
        renderedItem->addToGroup(busDot);
        renderedItem->addToGroup(label);
        renderedItem->setFlag(QGraphicsItem::ItemIsSelectable);
        renderedItem->setZValue(3);

        auto itemKey = renderedItemsKeyGen.gen();
        renderedItem->setData(0, itemKey);
        buses[key].renderedItem = renderedItem;
        this->addItem(renderedItem);

        c.intKey = key;
        renderedItems.insert(itemKey, c);
    }
}


QVector<std::tuple<int,int,int,int>> Scene::getPath(bus &bus)
{
    QVector<std::tuple<int,int>> path;
    auto start = stops[lines[bus.lineno].start].coord;
    auto end = stops[lines[bus.lineno].end].coord;
    auto mid = lines[bus.lineno].stopsAt;

    if (bus.reversed) swap(start, end);

    auto startSaved = start;

    if (!mid.empty())
    {
        if (bus.reversed) std::reverse(mid.begin(), mid.end());

        path.push_back(std::tuple<int,int>(0,0));
        for (const auto& key : mid)
        {
            path.pop_back();
            auto mid_point = stops[key].coord;
            p.loadGoal(start, mid_point);
            p.solveAStar();
            auto solution = p.getSolution();

            if (solution.first() != start) bus.halt = true;
            if (solution.last() != mid_point) bus.halt = true;

            path += solution;
            start = mid_point;
        }
        path.pop_back();
    }

    p.loadGoal(start, end);
    p.solveAStar();
    path += p.getSolution();

    if (path.first() != startSaved) bus.halt = true;
    if (path.last() != end) bus.halt = true;

    QVector<std::tuple<int,int,int,int>> result;

    if (path.size() > 1) {
        std::tuple<int,int> temp = path.first();

        for (int i = 1; i < path.size()-1; ++i)
        {
            result.push_back(std::tuple_cat(temp, path[i]));
            temp = path[i];
        }
        result.push_back(std::tuple_cat(temp, path.last()));
    }

    lines[bus.lineno].pathLines = result;

    return result;
}


void Scene::setNewPosition(bus &bus, double step)
{
    // if bus has to wait
    if (bus.wait > 0) {
        bus.wait -= interval_ms * step;
        if (bus.wait < 0) bus.wait = 0;
        return;
    }

    // bus is in the end station -> turn around
    if (bus.lastStation == bus.endStation){
        auto start = lines[bus.lineno].start;
        auto end = lines[bus.lineno].end;

        if (!bus.reversed) {
            bus.reversed = true;
            swap(start, end);
        } else {
            bus.reversed = false;
        }

        bus.startStation = start;
        bus.endStation = end;
        bus.path = getPath(bus);
        bus.visited = QVector<std::tuple<int,int>>();
        bus.wait = waitStop;
    }

    std::tuple<int,int,int,int> new_pos;
    std::tuple<int,int> point ;
    auto step_error = [](double s) {return int( s > 1 ? (s/2+1) : 2 );};

    // catches error in double -> int conversion
    bool contains {false};
    for (int x = -step_error(step); x <= step_error(step); ++x)
    {
        for (int y = -step_error(step); y <= step_error(step); ++y)
        {
            point = std::tuple<int,int>(int(bus.pos_x)+x, int(bus.pos_y)+y);
            if (points.contains(point))
            {
                contains = true;
                break;
            }
        }
        if (contains == true) break;
    }

    // if bus is on a certain point/stop
    if (contains and !bus.visited.contains(point)){

        if(!bus.path.empty())
            new_pos = bus.path.first();

        if(!bus.visited.empty())
            bus.visited.pop_front();

        bus.visited.push_back(point);

        if (stopsReversed.contains(point) and (lines[bus.lineno].start == stopsReversed[point].name
                                               or lines[bus.lineno].end == stopsReversed[point].name
                                               or lines[bus.lineno].stopsAt.contains(stopsReversed[point].name)))
            bus.lastStation = stopsReversed[point].name;

        bus.headingStation = getBusHeadingTo(bus);

        if(!bus.path.empty())
            bus.path.pop_front();

        auto new_x = std::get<2>(new_pos);
        auto new_y = std::get<3>(new_pos);

        auto dX = (new_x - bus.pos_x);
        auto dY = (new_y - bus.pos_y);
        bus.d = atan2(dY,dX);
    }

    if (bus.lastStation == bus.endStation) {
        return;
    }

    if (bus.halt == false)
    {
        // move the bus
        bus.renderedItem->setX(bus.pos_x + (step/bus.slow * cos(bus.d)));
        bus.renderedItem->setY(bus.pos_y + (step/bus.slow * sin(bus.d)));

        bus.pos_x = bus.renderedItem->pos().x();
        bus.pos_y = bus.renderedItem->pos().y();

        // get current street name
        int lastPoint_x, lastPoint_y, nextPoint_x, nextPoint_y;
        float A, B, C, res;
        for (const auto& street : streets)
        {
            for (const auto& path : street.pathLines)
            {
                lastPoint_x = std::get<0>(path);
                lastPoint_y = std::get<1>(path);
                nextPoint_x = std::get<2>(path);
                nextPoint_y = std::get<3>(path);

                A = sqrtf(powf(lastPoint_x - bus.pos_x,2) + powf(lastPoint_y - bus.pos_y,2));
                B = sqrtf(powf(nextPoint_x - bus.pos_x,2) + powf(nextPoint_y - bus.pos_y,2));
                C = sqrtf(powf(nextPoint_x - lastPoint_x,2) + powf(nextPoint_y - lastPoint_y,2));

                res = fabsf(A + B - C);

                // if result is correct (under treshold), save the current street into the bus
                if (res < 0.1) bus.currStreet = street.name;
            }

            // sets how fast the bus in on the current street
            if (bus.currStreet == street.name) bus.slow = street.traffic;
        }

    }
}


void Scene::simulate(double step)
{
    if (speed != 0) {
        for (auto &bus : buses)
        {
            setNewPosition(bus, step);
        }
    }
}

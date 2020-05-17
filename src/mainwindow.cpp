/*!
 * @file mainwindow.cpp
 * @author Peter Koprda (xkoprd00)
 * @author Adam Múdry (xmudry01)
 * @brief Initialization of the scene
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("ICP projekt 2019/2020 -- xkoprd00, xmudry01");

    initScene();
    initScrollBoxes();

    connect( ui->zoomSlider,          SIGNAL(valueChanged(int)), this,  SLOT(zoom(int))                    );
    connect( ui->speedSlider,         SIGNAL(valueChanged(int)), scene, SLOT(setSpeed(int))                );
    connect( ui->restartButton,       SIGNAL(clicked(bool)),     this,  SLOT(onClickedRestart(bool))       );
    connect( ui->playpauseButton,     SIGNAL(clicked(bool)),     this,  SLOT(onClickedPause(bool))         );
    connect( ui->clearButton,         SIGNAL(clicked(bool)),     this,  SLOT(onClickedClear(bool))         );
    connect( ui->rightButton,         SIGNAL(clicked(bool)),     this,  SLOT(onClickedForward(bool))       );
    connect( ui->leftButton,          SIGNAL(clicked(bool)),     this,  SLOT(onClickedBackward(bool))      );
    connect( ui->blockButton,         SIGNAL(clicked(bool)),     this,  SLOT(onClickedBlock(bool))         );
    connect( ui->unblockButton,       SIGNAL(clicked(bool)),     this,  SLOT(onClickedUnblock(bool))       );
    connect( ui->trafficSlider,       SIGNAL(valueChanged(int)), scene, SLOT(setTraffic(int))              );
    connect( ui->editOrSaveButton,    SIGNAL(clicked(bool)),     this,  SLOT(onClickedEditOrSave(bool))    );
    connect( ui->resetOrCancelButton, SIGNAL(clicked(bool)),     this,  SLOT(onClickedResetOrCancel(bool)) );

    connect( scene, SIGNAL(valueChanged(int)),            this,     SLOT(changeInterval(int))      );
    connect( scene, SIGNAL(timeValueChanged(QString)),    ui->time, SLOT(setText(QString))         );
    connect( scene, SIGNAL(infoLabelChanged(QString)),    this,     SLOT(setInfoLabel(QString))    );
    connect( scene, SIGNAL(trafficEnabledChanged(bool)),  this,     SLOT(setTrafficEnabled(bool))  );
    connect( scene, SIGNAL(lineEditEnabledChanged(bool)), this,     SLOT(setLineEditEnabled(bool)) );
    connect( scene, SIGNAL(trafficValueChanged(int)),     this,     SLOT(setTrafficSlider(int))    );
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::initScene()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    auto pathToFile = dialog.getOpenFileName(this, tr("Open JSON file"), qApp->applicationDirPath(), tr("JSON File (*.json)"));
    scene = new Scene(ui->graphicsView, pathToFile);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
}


void MainWindow::initScrollBoxes()
{
    ui->scrollAreaLines->setWidgetResizable(true);
    auto scrollLayoutLines = new QVBoxLayout(ui->scrollAreaLines);
    auto containerLines = new QWidget();
    ui->scrollAreaLines->setWidget(containerLines);
    scrollLayoutLines = new QVBoxLayout(containerLines);

    ui->scrollAreaBuses->setWidgetResizable(true);
    auto scrollLayoutBuses = new QVBoxLayout(ui->scrollAreaBuses);
    auto containerBuses = new QWidget();
    ui->scrollAreaBuses->setWidget(containerBuses);
    scrollLayoutBuses = new QVBoxLayout(containerBuses);

    ui->scrollAreaStreets->setWidgetResizable(true);
    auto scrollLayoutStreets = new QVBoxLayout(ui->scrollAreaStreets);
    auto containerStreets = new QWidget();
    ui->scrollAreaStreets->setWidget(containerStreets);
    scrollLayoutStreets = new QVBoxLayout(containerStreets);

    for (const auto& line : scene->getLines())
    {
        auto button = new QPushButton(containerLines);
        button->setText(QString::number(line.no));
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onClickedLine(bool)));
        scrollLayoutLines->addWidget(button, 0);
        button->show();
    }

    const auto& buses = scene->getBuses();
    for (const auto& key : buses.keys())
    {
        auto button = new QPushButton(containerBuses);
        button->setText(QString::number(buses[key].no));
        button->setProperty("intKey", key);
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onClickedBus(bool)));
        scrollLayoutBuses->addWidget(button, 0);
        button->show();
    }

    for (const auto& street : scene->getStreets())
    {
        auto button = new QPushButton(containerStreets);
        button->setText(street.name);
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onClickedStreet(bool)));
        scrollLayoutStreets->addWidget(button, 0);
        button->show();
    }
}


void MainWindow::zoom(int value)
{
    auto tr = ui->graphicsView->transform();
    ui->graphicsView->setTransform(QTransform(value, tr.m12(), tr.m21(), value, tr.dx(), tr.dy()));
}


void MainWindow::changeInterval(int value)
{
    if (value != 0) {
        scene->timer->setInterval(scene->interval_ms/value);
    }
}


void MainWindow::onClickedLine(bool val)
{
    auto buttonSender = qobject_cast<QPushButton*>(sender());
    auto result = scene->getLineInfo(buttonSender->text().toInt());
    ui->infoLabel->setText(result);

    scene->selectLine(buttonSender->text().toInt());
    scene->deselectStreet();

    setLineEditEnabled(true);
    setTrafficEnabled(false);
}


void MainWindow::onClickedBus(bool val)
{
    auto buttonSender = qobject_cast<QPushButton*>(sender());
    auto result = scene->getBusInfo(buttonSender->property("intKey").toInt());
    ui->infoLabel->setText(result);

    scene->selectLineViaBus(buttonSender->text().toInt());
    scene->deselectStreet();

    setLineEditEnabled(true);
    setTrafficEnabled(false);
}


void MainWindow::onClickedStreet(bool val)
{
    auto buttonSender = qobject_cast<QPushButton*>(sender());
    auto result = scene->getStreetInfo(buttonSender->text());
    ui->infoLabel->setText(result);

    scene->deselectLine();
    scene->selectStreet(buttonSender->text());

    setLineEditEnabled(false);
    setTrafficEnabled(true);
}


void MainWindow::onClickedClear(bool val)
{
    ui->infoLabel->setText("Info (select item)");
    scene->hideLines();
    setLineEditEnabled(false);
    setTrafficEnabled(false);

    scene->deselectLine();
    scene->deselectStreet();

    if (!scene->selectedItems().empty())
    {
        for (const auto& item : scene->selectedItems())
        {
            item->setSelected(false);
        }
    }
}


void MainWindow::onClickedRestart(bool val)
{
    scene->deselectLine();
    scene->deselectStreet();
    ui->time->setText("00:00:00");
    scene->resetTime();
    if (!scene->timer->isActive()) scene->simulate();
}


void MainWindow::onClickedPause(bool val)
{
    static bool isPaused = false;
    if (!isPaused) {
        scene->timer->stop();
        isPaused = true;
    } else {
        scene->timer->start();
        isPaused = false;
    }
}


void MainWindow::onClickedForward(bool val)
{
    for (int i = 0; i < (1000/scene->interval_ms); ++i)
    {
        scene->updateTime();
    }

    static const auto step = 3;
    for (int i = 0; i < (1000/scene->interval_ms/step); ++i)
    {
        scene->simulate(double(step));
    }
}


void MainWindow::onClickedBackward(bool val)
{
    auto time = scene->getTime();
    time = (time-1)*1000;
    ui->time->setText("00:00:00");
    scene->resetTime();
    for (int i = 0; i < (time/scene->interval_ms); ++i)
    {
        scene->updateTime();
    }
    static const auto step = 4;
    for (int i = 0; i < (time/scene->interval_ms/step); ++i)
    {
        scene->simulate(double(step));
    }
}


void MainWindow::onClickedBlock(bool val)
{

    QString blockedStreet = scene->getStreetInfo(ui->infoLabel->text());
    bool isBlocked = scene->blockStreet(blockedStreet);
    if(isBlocked)
    {
        scene->resetTime();
        ui->blockButton->setDisabled(true);
        ui->unblockButton->setEnabled(true);
    }
}


void MainWindow::onClickedUnblock(bool val)
{
    QString unblockedStreet = scene->getStreetInfo(ui->infoLabel->text());
    bool isBlocked = scene->unblockStreet(unblockedStreet);
    if(isBlocked)
    {
        scene->resetTime();
        ui->unblockButton->setDisabled(true);
    }
}


void MainWindow::onClickedEditOrSave(bool val)
{
    if (scene->getEditMode() == false)
    {
        scene->timer->stop();
        scene->resetTime();
        scene->hideBuses(true);

        ui->time->setText("00:00:00");
        setControlsEnabled(false);
        setInfoLabel("Select a new route for the selected line");
        ui->editOrSaveButton->setText("Save");
        ui->resetOrCancelButton->setText("Cancel");

        scene->setEditMode(true);
    }
    else if (scene->getEditMode() == true)
    {

        setControlsEnabled(true);
        setInfoLabel("Info (select item)");
        ui->editOrSaveButton->setText("Edit");
        ui->resetOrCancelButton->setText("Reset All");

        scene->saveEdit();

        scene->resetTime();
        scene->timer->start();
        scene->hideBuses(false);

        scene->setEditMode(false);
    }
}


void MainWindow::onClickedResetOrCancel(bool val)
{
    scene->deselectLine();
    scene->deselectStreet();

    if (scene->getEditMode() == false)
    {
        ui->time->setText("00:00:00");
        scene->resetLines();
        scene->resetTime();
    }
    else if (scene->getEditMode() == true)
    {
        scene->timer->start();
        scene->hideBuses(false);

        setControlsEnabled(true);
        setInfoLabel("Info (select item)");
        ui->editOrSaveButton->setText("Edit");
        ui->resetOrCancelButton->setText("Reset All");

        scene->setEditMode(false);
    }
}


void MainWindow::setInfoLabel(QString val)
{
    ui->infoLabel->setText(val);
}


void MainWindow::setTrafficEnabled(bool val)
{
    ui->unblockButton->setEnabled(val);
    ui->blockButton->setEnabled(val);
    ui->trafficSlider->setEnabled(val);
}


void MainWindow::setControlsEnabled(bool val)
{
    ui->leftButton->setEnabled(val);
    ui->restartButton->setEnabled(val);
    ui->playpauseButton->setEnabled(val);
    ui->rightButton->setEnabled(val);
    ui->clearButton->setEnabled(val);
    ui->scrollAreaLines->setEnabled(val);
    ui->scrollAreaBuses->setEnabled(val);
    ui->scrollAreaStreets->setEnabled(val);
}

void MainWindow::setTrafficSlider(int val)
{
    ui->trafficSlider->setValue(val);
}


void MainWindow::setLineEditEnabled(bool val)
{
    ui->editOrSaveButton->setEnabled(val);
    ui->resetOrCancelButton->setEnabled(true);

    if (!val)
    {
        ui->editOrSaveButton->setText("Edit");
        ui->resetOrCancelButton->setText("Reset All");
    }
}

/*!
 * @file mainwindow.h
 * @author Peter Koprda (xkoprd00)
 * @author Adam Múdry (xmudry01)
 * @brief Initialization header
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QFileDialog>
#include "scene.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/*!
 * \brief Main Window class
 * \details Implements the logic behind the main window and it's buttons, sliders, etc.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /*!
     * \brief constructor
     * \param parent
     */
    MainWindow(QWidget *parent = Q_NULLPTR);

    /*!
     * \brief destructor
     */
    ~MainWindow();

private slots:

    /*!
     * \brief changes zoom of scene
     * \param value
     */
    void zoom(int value);

    /*!
     * \brief changes speed of time
     * \param value
     */
    void changeInterval(int value);

    /*!
     * \brief gets info about selected line
     * \param val
     */
    void onClickedLine(bool val);

    /*!
     * \brief gets info about selected bus
     * \param val
     */
    void onClickedBus(bool val);

    /*!
     * \brief gets info about selected street
     * \param val
     */
    void onClickedStreet(bool val);

    /*!
     * \brief makes default map without any selection
     * \param val
     */
    void onClickedClear(bool val);

    /*!
     * \brief resets simulation and sets time to 00:00:00
     * \param val
     */
    void onClickedRestart(bool val);

    /*!
     * \brief pauses or resumes the scene
     * \param val
     */
    void onClickedPause(bool val);

    /*!
     * \brief scene is shifted forward by one second
     * \param val
     */
    void onClickedForward(bool val);

    /*!
     * \brief scene is shifted backward by one second
     * \param val
     */
    void onClickedBackward(bool val);

    /*!
     * \brief blocks selected street
     * \param val
     */
    void onClickedBlock(bool val);

    /*!
     * \brief unblocks selected street
     * \param val
     */
    void onClickedUnblock(bool val);

    /*!
     * \brief makes editable scene or the new route is saved
     * \param val
     */
    void onClickedEditOrSave(bool val);

    /*!
     * \brief cancels the new route
     * \param val
     */
    void onClickedResetOrCancel(bool val);

    /*!
     * \brief overwrites label with informations about scene
     * \param val
     */
    void setInfoLabel(QString val);

    /*!
     * \brief enables or disables unblock, block button and traffic slider
     * \param val
     */
    void setTrafficEnabled(bool val);

    /*!
     * \brief enables or disables scene controls
     * \param val
     */
    void setControlsEnabled(bool val);

    /*!
     * \brief sets value in traffic slider
     * \param val
     */
    void setTrafficSlider(int val);

    /*!
     * \brief changes line edit buttons
     * \param val
     */
    void setLineEditEnabled(bool val);

private:
    Ui::MainWindow *ui;
    QPointF pos;
    Scene * scene;

    /*!
     * \brief initializes scene
     */
    void initScene();

    /*!
     * \brief initializes scroll boxes
     */
    void initScrollBoxes();

    friend class Scene;

};
#endif // MAINWINDOW_H

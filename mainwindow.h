#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QTableWidget>
#include <QCloseEvent>
#include <QTimer>
#include <QTime>

#include "databasemgr.h"
#include "settings.h"
#include "updater.h"
#include "webservice.h"
#include "alertswidget.h"
#include "droneswidget.h"
#include "dronetaskswidget.h"
#include "locationswidget.h"
#include "softwarewidget.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
  bool isBusy();

protected:
  void closeEvent(QCloseEvent *event);
  void showEvent(QShowEvent *);
  void changeEvent(QEvent *e);

private slots:
  void exitProgram();
  void showOptions();
  void checkForUpdate();
  void manuallyCheckForUpdate();
  void checkForUpdateFinished(QString response);
  void showAbout();
  void updateStatus(QString message);

private:
  void createActions();
  void createMenus();

  QMenu *fileMenu;
  QMenu *toolsMenu;
  QMenu *helpMenu;
  QAction *optionsAction;
  QAction *softwareUpdateAction;
  //QAction *restartDevicesAction;
  QAction *exitAction;
  //QAction *documentationAction;
  QAction *aboutAction;

  QTabWidget *tabs;
  AlertsWidget *alertsWidget;
  DronesWidget *dronesWidget;
  DroneTasksWidget *droneTasksWidget;
  SoftwareWidget *softwareWidget;
  LocationsWidget *locationsWidget;

  WebService *webService;
  DatabaseMgr *dbMgr;  
  Settings *settings;
  Updater *softwareUpdater;
  QTimer *checkForUpdateTimer;  
  bool manualUpdateCheck;
};

#endif // MAINWINDOW_H

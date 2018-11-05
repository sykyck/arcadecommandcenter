#ifndef DRONETASKSWIDGET_H
#define DRONETASKSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QTimer>
#include <QProcess>
#include "databasemgr.h"
#include "settings.h"
#include "drone.h"
#include "webservice.h"
#include "editdronetaskwidget.h"

class DroneTasksWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DroneTasksWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent = 0);
  ~DroneTasksWidget();
  bool isBusy();


protected:
  void showEvent(QShowEvent *);

signals:
  
public slots:

  
private slots:  
  void clearTable();
  void downloadingTasks();
  void finishedDownloadingTasks(bool success, QVariantMap response);
  void updateTasks();
  void updateTasksClicked();
  void addTaskClicked();
  void deleteTasksClicked();
  void finishedDeletingTasks(bool success, QVariantMap response);

private:
  enum DroneTasksColumns
  {    
    Task_ID,
    Task_Name,
    Package_URL,
    Expiration,
    Num_Rules,
    Num_Drones_Finished,
    Num_Drones_Applicable
  };

  void insertRecord(DroneTask task);
  void populateTable();

  DatabaseMgr *dbMgr;
  Settings *settings;  
  WebService *webService;
  bool busy;
  bool firstLoad;

  QVBoxLayout *verticalLayout;
  QHBoxLayout *buttonLayout;
  QPushButton *btnRefreshTasks;
  QPushButton *btnDelete;
  QPushButton *btnNewTask;
  QTimer *statusTimer;
  QStandardItemModel *droneTasksModel;
  QTableView *droneTasksView;
  QLabel *lblDroneTasks;
};

#endif // DRONETASKSWIDGET_H

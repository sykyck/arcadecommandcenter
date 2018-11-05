#ifndef DRONESWIDGET_H
#define DRONESWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QProcess>
#include <QEvent>
#include "databasemgr.h"
#include "settings.h"
#include "drone.h"
#include "webservice.h"

class DronesWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DronesWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent = 0);
  ~DronesWidget();
  bool isBusy();


protected:
  void showEvent(QShowEvent *);
  bool eventFilter(QObject *obj, QEvent *event);

signals:
  
public slots:

  
private slots:  
  void clearDronesTable();
  void downloadingAlerts();
  void finishedDownloadingDrones(bool success, QVariantMap response);
  void updateDrones();
  void deleteDronesClicked();
  void updateDronesClicked();
  void finishedDeletingDrones(bool success, QVariantMap response);

private:
  enum DroneColumns
  {
    UUID,
    Current_Task_ID,
    Location,
    Public_IP,    
    Local_IP,
    Hostname,
    Uptime,
    Machine_Type,
    Operating_System,
    Check_In_Time
  };

  void insertDrone(Drone drone);
  void populateTable();

  DatabaseMgr *dbMgr;
  Settings *settings;  
  WebService *webService;
  bool busy;
  bool firstLoad;

  QVBoxLayout *verticalLayout;
  QHBoxLayout *buttonLayout;
  QPushButton *btnRefreshDrones;
  QPushButton *btnDelete;
  QPushButton *btnPushAlerts;
  QTimer *statusTimer;
  QStandardItemModel *dronesModel;
  QTableView *dronesView;
  QSortFilterProxyModel *sortFilter;
  QLabel *lblDrones;
};

#endif // DRONESWIDGET_H

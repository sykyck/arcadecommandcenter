#ifndef ALERTSWIDGET_H
#define ALERTSWIDGET_H

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
#include "alert.h"
#include "webservice.h"
#include "alertdetailwidget.h"

class AlertsWidget : public QWidget
{
  Q_OBJECT
public:
  explicit AlertsWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent = 0);
  ~AlertsWidget();
  bool isBusy();


protected:
  void showEvent(QShowEvent *);
  bool eventFilter(QObject *obj, QEvent *event);

signals:
  void updateStatusMessage(QString message);
  
public slots:

  
private slots:  
  void clearAlertsTable();
  void downloadingAlerts();
  void finishedDownloadingAlerts(bool success, QVariantMap response);
  void updateAlerts();
  void deleteAlertsClicked();
  void updateAlertsClicked();
  void finishedDeletingAlerts(bool success, QVariantMap response);
  void alertDoubleclicked(const QModelIndex &index);


private:
  enum AlertColumns
  {
    Alert_ID,
    Date_Time,
    Location,
    Public_IP,
    Local_IP,
    Hostname,
    Uptime,
    Software,
    Version,
    Message,
    Sent
  };

  void insertAlert(Alert alert);
  void populateTable();

  DatabaseMgr *dbMgr;
  Settings *settings;  
  WebService *webService;
  bool busy;
  bool firstLoad;

  QVBoxLayout *verticalLayout;
  QHBoxLayout *buttonLayout;
  QPushButton *btnRefreshAlerts;
  QPushButton *btnDelete;
  QPushButton *btnPushAlerts;
  QTimer *statusTimer;
  QStandardItemModel *alertsModel;
  QTableView *alertsView;
  QSortFilterProxyModel *sortFilter;
  QLabel *lblAlerts;
  AlertDetailWidget *alertDetailWidget;
};

#endif // ALERTSWIDGET_H

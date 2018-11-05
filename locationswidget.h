#ifndef LOCATIONSWIDGET_H
#define LOCATIONSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QStandardItemModel>
#include <QTimer>
#include <QProcess>
#include <QSortFilterProxyModel>
#include "databasemgr.h"
#include "settings.h"
#include "location.h"
#include "webservice.h"

class LocationsWidget : public QWidget
{
  Q_OBJECT
public:
  explicit LocationsWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent = 0);
  ~LocationsWidget();
  bool isBusy();


protected:
  void showEvent(QShowEvent *);

signals:
  
public slots:

  
private slots:  
  void clearTable();
  void downloadingLocations();
  void finishedDownloadingLocations(bool success, QVariantMap response);
  void updateLocations();
  void updateLocationsClicked();
  void addLocationClicked();
  void locationDoubleClicked(const QModelIndex &index);
  void deleteLocationsClicked();
  void finishedDeletingLocations(bool success, QVariantMap response);

private:
  enum LocationsColumns
  {    
    Location_ID,
    Name,
    Public_IP,
    Collections_Mailing_List,
    Service_Mailing_List,
    Bill_Acceptor_Mailing_List,
    Allow_Notifications,
    Show_Collections
  };

  void insertRecord(Location loc);
  void populateTable();

  DatabaseMgr *dbMgr;
  Settings *settings;  
  WebService *webService;
  bool busy;
  bool firstLoad;

  QVBoxLayout *verticalLayout;
  QHBoxLayout *buttonLayout;
  QPushButton *btnRefreshLocations;
  QPushButton *btnDelete;
  QPushButton *btnNewLocation;
  QTimer *statusTimer;
  QStandardItemModel *locationsModel;
  QTableView *locationsView;
  QLabel *lblLocations;
  QSortFilterProxyModel *sortFilter;
};

#endif // LOCATIONSWIDGET_H

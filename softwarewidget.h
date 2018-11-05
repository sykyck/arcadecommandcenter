#ifndef SOFTWAREWIDGET_H
#define SOFTWAREWIDGET_H

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
#include "software.h"
#include "webservice.h"
#include "editsoftwarewidget.h"

class SoftwareWidget : public QWidget
{
  Q_OBJECT
public:
  explicit SoftwareWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent = 0);
  ~SoftwareWidget();
  bool isBusy();


protected:
  void showEvent(QShowEvent *);

signals:
  
public slots:

  
private slots:  
  void clearTable();
  void downloadingSoftware();
  void finishedDownloadingSoftware(bool success, QVariantMap response);
  void updateSoftware();
  void updateSoftwareClicked();
  void addSoftwareClicked();
  void deleteSoftwareClicked();
  void finishedDeletingSoftware(bool success, QVariantMap response);
  void softwareDoubleClicked(const QModelIndex &index);

private:
  enum SoftwareColumns
  {    
    Software_ID,
    Name,
    Version,
    Installer_URL,
    Hash
  };

  void insertRecord(Software s);
  void populateTable();

  DatabaseMgr *dbMgr;
  Settings *settings;  
  WebService *webService;
  bool busy;
  bool firstLoad;

  QVBoxLayout *verticalLayout;
  QHBoxLayout *buttonLayout;
  QPushButton *btnRefreshSoftware;
  QPushButton *btnDelete;
  QPushButton *btnNewSoftware;
  QTimer *statusTimer;
  QStandardItemModel *softwareModel;
  QTableView *softwareView;
  QLabel *lblSoftware;
};

#endif // SOFTWAREWIDGET_H

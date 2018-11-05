#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QTimer>
#include <QDir>
#include <QMap>

#include "qslog/QsLog.h"
#include "formpost/formpost.h"
#include "dronetask.h"
#include "software.h"
#include "location.h"

class WebService : public QObject
{
  Q_OBJECT
public:
  explicit WebService(QString webServiceUrl, QString password, QObject *parent = 0);
  ~WebService();
  void startGetAlerts(qlonglong alertHash);
  void startdeleteAlerts(QStringList alertIdList, bool deleteAll);
  void startGetDrones(qlonglong droneHash);
  void startDeleteDrones(QStringList droneUuidList);
  void startGetDroneTasks(qlonglong taskHash);
  void startAddDroneTask(DroneTask task);
  void startDeleteDroneTasks(QStringList taskIdList);
  void startGetLocations(qlonglong locationHash);
  void startGetSoftware(qlonglong softwareHash);
  void startAddSoftware(Software software);
  void startUpdateSoftware(Software software);
  void startDeleteSoftware(QStringList softwareIdList);
  void startAddLocation(Location location);
  void startUpdateLocation(Location location);
  void startDeleteLocations(QStringList locationIdList);

signals:
  void getAlertsResult(bool success, QVariantMap response);
  void deleteAlertsResult(bool success, QVariantMap response);
  void getDronesResult(bool success, QVariantMap response);
  void deleteDronesResult(bool success, QVariantMap response);
  void getDroneTasksResult(bool success, QVariantMap response);
  void addDroneTaskResult(bool success, QVariantMap response);
  void deleteDroneTasksResult(bool success, QVariantMap response);
  void getLocationsResult(bool success, QVariantMap response);
  void getSoftwareResult(bool success, QVariantMap response);
  void addSoftwareResult(bool success, QVariantMap response);
  void updateSoftwareResult(bool success, QVariantMap response);
  void deleteSoftwareResult(bool success, QVariantMap response);
  void addLocationResult(bool success, QVariantMap response);
  void updateLocationResult(bool success, QVariantMap response);
  void deleteLocationsResult(bool success, QVariantMap response);

private slots:
  void receivedGetAlertsResponse();
  void receivedDeleteAlertsResponse();
  void receivedGetDronesResponse();
  void receivedDeleteDronesResponse();
  void receivedGetDroneTasksResponse();
  void receivedAddDroneTaskResponse();
  void receivedDeleteDroneTasksResponse();
  void receivedGetLocationsResponse();
  void receivedGetSoftwareResponse();
  void receivedAddSoftwareResponse();
  void receivedUpdateSoftwareResponse();
  void receivedDeleteSoftwareResponse();
  void receivedAddLocationResponse();
  void receivedUpdateLocationResponse();
  void receivedDeleteLocationsResponse();
  void resetTimeout(qint64,qint64);
  void abortConnection();

private:
  QTimer *timer;

  QNetworkAccessManager *netMgr;
  QNetworkReply *netReplyGetAlerts;
  QNetworkReply *netReplyDeleteAlerts;
  QNetworkReply *netReplyGetDrones;
  QNetworkReply *netReplyDeleteDrones;
  QNetworkReply *netReplyGetDroneTasks;
  QNetworkReply *netReplyAddDroneTask;
  QNetworkReply *netReplyDeleteDroneTask;
  QNetworkReply *netReplyGetLocations;
  QNetworkReply *netReplyGetSoftware;
  QNetworkReply *netReplyAddSoftware;
  QNetworkReply *netReplyUpdateSoftware;
  QNetworkReply *netReplyDeleteSoftware;
  QNetworkReply *netReplyAddLocation;
  QNetworkReply *netReplyUpdateLocation;
  QNetworkReply *netReplyDeleteLocations;

  QTimer *connectionTimer;
  QUrl webServiceUrl;

  QString password;
  FormPostPlugin *formPost;
};

#endif // WEBSERVICE_H

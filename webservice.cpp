#include "webservice.h"
#include <QStringList>
#include <QFileInfo>
#include "qjson/serializer.h"
#include "qjson/parser.h"

// Maximum number of times we retry to contact server
const int MAX_RETRIES = 3;

// Maximum time in msec to wait for a network connection
const int CONNECTION_TIMEOUT = 15000;

WebService::WebService(QString webServiceUrl, QString password, QObject *parent) : QObject(parent)
{
  /*

  urls["alertLogUrl"] = "http://restlessmindsstudio.com/getAlertLog";
  urls["deleteAlertsUrl"] = "http://restlessmindsstudio.com/deleteAlertLog";
  urls["dronesUrl"] = "http://restlessmindsstudio.com/getDrones";
  urls["deleteDronesUrl"] = "http://restlessmindsstudio.com/deleteDrone";
  urls["droneTasksUrl"] = "http://restlessmindsstudio.com/getDroneTasks";
  urls["addDroneTaskUrl"] = "http://restlessmindsstudio.com/addTask";
  urls["deleteDroneTaskUrl"] = "http://restlessmindsstudio.com/deleteTask";
  urls["locationsUrl"] = "http://restlessmindsstudio.com/getLocations";
  urls["softwareUrl"] = "http://restlessmindsstudio.com/getSoftware";
  urls["addSoftwareUrl"] = "http://restlessmindsstudio.com/addSoftware";
  urls["updateSoftwareUrl"] = "http://restlessmindsstudio.com/updateSoftware";
  urls["deleteSoftwareUrl"] = "http://restlessmindsstudio.com/deleteSoftware";
  */

  this->webServiceUrl = QUrl(webServiceUrl);
  this->password = password;

  formPost = new FormPostPlugin;

  netReplyGetAlerts = 0;
  netReplyDeleteAlerts = 0;
  netReplyGetDrones = 0;
  netReplyDeleteDrones = 0;
  netReplyGetDroneTasks = 0;
  netReplyAddDroneTask = 0;
  netReplyGetLocations = 0;
  netReplyGetSoftware = 0;
  netReplyAddSoftware = 0;
  netReplyUpdateSoftware = 0;
  netReplyDeleteSoftware = 0;
  netReplyAddLocation = 0;
  netReplyUpdateLocation = 0;
  netReplyDeleteLocations = 0;

  netMgr = new QNetworkAccessManager;
  //connect(netMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkActivityFinished(QNetworkReply*)));

  timer = new QTimer;
  timer->setSingleShot(true);
  timer->setInterval(1 + (qrand() % 30000));
  //connect(timer, SIGNAL(timeout()), this, SLOT(videoLookup()));

  connectionTimer = new QTimer;
  connectionTimer->setInterval(CONNECTION_TIMEOUT);
  connectionTimer->setSingleShot(true);
  connect(connectionTimer, SIGNAL(timeout()), this, SLOT(abortConnection()));
}

WebService::~WebService()
{
  timer->deleteLater();
  netMgr->deleteLater();

  if (netReplyGetAlerts)
    netReplyGetAlerts->deleteLater();

  if (netReplyDeleteAlerts)
    netReplyDeleteAlerts->deleteLater();

  if (netReplyGetDrones)
    netReplyGetDrones->deleteLater();

  if (netReplyDeleteDrones)
    netReplyDeleteDrones->deleteLater();

  if (netReplyGetDroneTasks)
    netReplyGetDroneTasks->deleteLater();

  if (netReplyAddDroneTask)
    netReplyAddDroneTask->deleteLater();

  if (netReplyGetLocations)
    netReplyGetLocations->deleteLater();

  if (netReplyGetSoftware)
    netReplyGetSoftware->deleteLater();

  if (netReplyAddSoftware)
    netReplyAddSoftware->deleteLater();

  if (netReplyUpdateSoftware)
    netReplyUpdateSoftware->deleteLater();

  if (netReplyDeleteSoftware)
    netReplyDeleteSoftware->deleteLater();

  if (netReplyAddLocation)
    netReplyAddLocation->deleteLater();

  if (netReplyUpdateLocation)
    netReplyUpdateLocation->deleteLater();

  if (netReplyDeleteLocations)
    netReplyDeleteLocations->deleteLater();

  connectionTimer->deleteLater();

  formPost->deleteLater();
}

void WebService::startGetAlerts(qlonglong alertHash)
{
  QLOG_DEBUG() << QString("Checking for alerts from web service. Current alert hash: %1").arg(alertHash);

  QVariantMap jsonRequest;
  jsonRequest["action"] = "getAlertLog";
  jsonRequest["alert_hash"] = alertHash;
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyGetAlerts = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyGetAlerts, SIGNAL(finished()), this, SLOT(receivedGetAlertsResponse()));
  connect(netReplyGetAlerts, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startdeleteAlerts(QStringList alertIdList, bool deleteAll)
{
  QVariantMap jsonRequest;

  // if this flag is set then instead of using the list of alert IDs, just truncate the table
  if (deleteAll)
  {
    QLOG_DEBUG() << QString("Deleting ALL alerts through web service");
    jsonRequest["delete_all"] = true;
  }
  else
  {
    QLOG_DEBUG() << QString("Deleting alert IDs: %1 through web service:").arg(alertIdList.join(","));
    jsonRequest["alert_id"] = alertIdList;
  }

  jsonRequest["action"] = "deleteAlertLog";
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyDeleteAlerts = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyDeleteAlerts, SIGNAL(finished()), this, SLOT(receivedDeleteAlertsResponse()));
  connect(netReplyDeleteAlerts, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startGetDrones(qlonglong droneHash)
{
  QLOG_DEBUG() << QString("Checking for changes to drones from web service. Current alert hash: %1").arg(droneHash);

  QVariantMap jsonRequest;
  jsonRequest["action"] = "getDrones";
  jsonRequest["drone_hash"] = droneHash;
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyGetDrones = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyGetDrones, SIGNAL(finished()), this, SLOT(receivedGetDronesResponse()));
  connect(netReplyGetDrones, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startDeleteDrones(QStringList droneUuidList)
{
  QVariantMap jsonRequest;

  QLOG_DEBUG() << QString("Deleting drone UUIDs: %1 through web service:").arg(droneUuidList.join(","));
  jsonRequest["action"] = "deleteDrone";
  jsonRequest["uuid"] = droneUuidList;
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyDeleteDrones = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyDeleteDrones, SIGNAL(finished()), this, SLOT(receivedDeleteDronesResponse()));
  connect(netReplyDeleteDrones, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startGetDroneTasks(qlonglong taskHash)
{
  QLOG_DEBUG() << QString("Checking for changes to drone tasks from web service. Current task hash: %1").arg(taskHash);

  QVariantMap jsonRequest;
  jsonRequest["action"] = "getDroneTasks";
  jsonRequest["task_hash"] = taskHash;
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyGetDroneTasks = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyGetDroneTasks, SIGNAL(finished()), this, SLOT(receivedGetDroneTasksResponse()));
  connect(netReplyGetDroneTasks, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startAddDroneTask(DroneTask task)
{
  QLOG_DEBUG() << QString("Sending drone task to web service");

  QVariantMap jsonRequest;
  jsonRequest["action"] = "addTask";
  jsonRequest["passphrase"] = password;
  jsonRequest["task_name"] = task.name;
  jsonRequest["expiration_date"] = task.expiration_date;

  QVariantList ruleList;
  foreach (TaskRule rule, task.rules)
  {
    QVariantMap r;
    r["operand"] = rule.operand;
    r["op"] = rule.op;
    r["value"] = rule.value;

    ruleList.append(r);
  }

  jsonRequest["rules"] = ruleList;

  QJson::Serializer serializer;

  formPost->addField("msg", QString(serializer.serialize(jsonRequest)));
  formPost->addFile("package", task.packageURL, "application/octet-stream");

  netReplyAddDroneTask = formPost->postData(webServiceUrl.toString());

  connect(netReplyAddDroneTask, SIGNAL(finished()), this, SLOT(receivedAddDroneTaskResponse()));
  connect(netReplyAddDroneTask, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));
  connect(netReplyAddDroneTask, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startDeleteDroneTasks(QStringList taskIdList)
{
  QVariantMap jsonRequest;

  QLOG_DEBUG() << QString("Deleting task IDs: %1 through web service").arg(taskIdList.join(","));
  jsonRequest["action"] = "deleteTask";
  jsonRequest["task_id"] = taskIdList;
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyDeleteDroneTask = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyDeleteDroneTask, SIGNAL(finished()), this, SLOT(receivedDeleteDroneTasksResponse()));
  connect(netReplyDeleteDroneTask, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startGetLocations(qlonglong locationHash)
{
  QLOG_DEBUG() << QString("Checking for changes to locations from web service. Current location hash: %1").arg(locationHash);

  QVariantMap jsonRequest;
  jsonRequest["action"] = "getLocations";
  jsonRequest["location_hash"] = locationHash;
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyGetLocations = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyGetLocations, SIGNAL(finished()), this, SLOT(receivedGetLocationsResponse()));
  connect(netReplyGetLocations, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startGetSoftware(qlonglong softwareHash)
{
  QLOG_DEBUG() << QString("Checking for changes to software from web service. Current software hash: %1").arg(softwareHash);

  QVariantMap jsonRequest;
  jsonRequest["action"] = "getSoftware";
  jsonRequest["software_hash"] = softwareHash;
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyGetSoftware = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyGetSoftware, SIGNAL(finished()), this, SLOT(receivedGetSoftwareResponse()));
  connect(netReplyGetSoftware, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startAddSoftware(Software software)
{
  QLOG_DEBUG() << QString("Sending software to web service");

  QVariantMap jsonRequest;
  jsonRequest["action"] = "addSoftware";
  jsonRequest["passphrase"] = password;
  jsonRequest["software_name"] = software.name;
  jsonRequest["version"] = software.version;

  QJson::Serializer serializer;

  formPost->addField("msg", QString(serializer.serialize(jsonRequest)));
  formPost->addFile("package", software.installerURL, "application/octet-stream");

  netReplyAddSoftware = formPost->postData(webServiceUrl.toString());

  connect(netReplyAddSoftware, SIGNAL(finished()), this, SLOT(receivedAddSoftwareResponse()));
  connect(netReplyAddSoftware, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));
  connect(netReplyAddSoftware, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startUpdateSoftware(Software software)
{
  QLOG_DEBUG() << QString("Sending software update to web service");

  QVariantMap jsonRequest;
  jsonRequest["action"] = "updateSoftware";
  jsonRequest["passphrase"] = password;
  jsonRequest["software_id"] = software.softwareID;
  jsonRequest["software_name"] = software.name;
  jsonRequest["version"] = software.version;

  QJson::Serializer serializer;

  formPost->addField("msg", QString(serializer.serialize(jsonRequest)));

  if (!software.installerURL.isEmpty())
  {
    formPost->addFile("package", software.installerURL, "application/octet-stream");
  }

  netReplyUpdateSoftware = formPost->postData(webServiceUrl.toString());

  connect(netReplyUpdateSoftware, SIGNAL(finished()), this, SLOT(receivedUpdateSoftwareResponse()));
  connect(netReplyUpdateSoftware, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));
  connect(netReplyUpdateSoftware, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startDeleteSoftware(QStringList softwareIdList)
{
  QVariantMap jsonRequest;

  QLOG_DEBUG() << QString("Deleting software IDs: %1 through web service:").arg(softwareIdList.join(","));
  jsonRequest["action"] = "deleteSoftware";
  jsonRequest["software_id"] = softwareIdList;
  jsonRequest["passphrase"] = password;

  //qDebug(qPrintable(data));
  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;
  //QByteArray convertedData = serializer.serialize(jsonRequest);

  //QString postRequest = QString("msg=%1").arg(serializer.serialize(jsonRequest).constData());
  //QLOG_DEBUG() << QString("Sending: %1").arg(serializer.serialize(jsonRequest).constData());
  //QByteArray convertedData(qPrintable(postRequest));

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyDeleteSoftware = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyDeleteSoftware, SIGNAL(finished()), this, SLOT(receivedDeleteSoftwareResponse()));
  connect(netReplyDeleteSoftware, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startAddLocation(Location location)
{
  // "name":"","public_ip_address":"","collections_mailing_list":"","service_mailing_list":"","bill_acceptor_mailing_list":"","allow_notifications":"","show_collections":""
  QVariantMap jsonRequest;

  QLOG_DEBUG() << QString("Adding location through web service");

  jsonRequest["action"] = "addLocation";
  jsonRequest["name"] = location.name;
  jsonRequest["public_ip_address"] = location.publicIP;
  jsonRequest["collections_mailing_list"] = location.collectionsMailingList;
  jsonRequest["service_mailing_list"] = location.serviceMailingList;
  jsonRequest["bill_acceptor_mailing_list"] = location.billAcceptorMailingList;
  jsonRequest["allow_notifications"] = location.allowNotifications;
  jsonRequest["show_collections"] = location.showCollections;
  jsonRequest["passphrase"] = password;

  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyAddLocation = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyAddLocation, SIGNAL(finished()), this, SLOT(receivedAddLocationResponse()));
  connect(netReplyAddLocation, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startUpdateLocation(Location location)
{
  // "location_id":"","name":"","public_ip_address":"","collections_mailing_list":"","service_mailing_list":"","bill_acceptor_mailing_list":"","allow_notifications":"","show_collections":""
  QVariantMap jsonRequest;

  QLOG_DEBUG() << QString("Updating location through web service");

  jsonRequest["action"] = "updateLocation";
  jsonRequest["location_id"] = location.locationID;
  jsonRequest["name"] = location.name;
  jsonRequest["public_ip_address"] = location.publicIP;
  jsonRequest["collections_mailing_list"] = location.collectionsMailingList;
  jsonRequest["service_mailing_list"] = location.serviceMailingList;
  jsonRequest["bill_acceptor_mailing_list"] = location.billAcceptorMailingList;
  jsonRequest["allow_notifications"] = location.allowNotifications;
  jsonRequest["show_collections"] = location.showCollections;
  jsonRequest["passphrase"] = password;

  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyUpdateLocation = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyUpdateLocation, SIGNAL(finished()), this, SLOT(receivedUpdateLocationResponse()));
  connect(netReplyUpdateLocation, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::startDeleteLocations(QStringList locationIdList)
{
  QVariantMap jsonRequest;

  QLOG_DEBUG() << QString("Deleting location IDs: %1 through web service:").arg(locationIdList.join(","));
  jsonRequest["action"] = "deleteLocations";
  jsonRequest["location_id"] = locationIdList;
  jsonRequest["passphrase"] = password;

  QNetworkRequest request(webServiceUrl);

  QJson::Serializer serializer;

  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  netReplyDeleteLocations = netMgr->post(request, serializer.serialize(jsonRequest));

  connect(netReplyDeleteLocations, SIGNAL(finished()), this, SLOT(receivedDeleteLocationsResponse()));
  connect(netReplyDeleteLocations, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(resetTimeout(qint64,qint64)));

  // Start timer which will abort the connection when the time expires
  // Connecting the downloadProgress signal to the resetTimeout slot
  // allows resetting the timer everytime the progress changes
  connectionTimer->start();
}

void WebService::receivedGetAlertsResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyGetAlerts->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyGetAlerts->readAll();

    // Expected response is in JSON
    // { "alert_hash" : 0, "data":[{"alert_id":"","alert_time":"","name":"","public_ip_address":"","local_ip_address":"","hostname":"","uptime":"","software_name":"","version":"","log_message":"","sent":"1"},
    //                             {...}]}

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while getting alerts: %1").arg(netReplyGetAlerts->errorString());
  }

  netReplyGetAlerts->deleteLater();
  netReplyGetAlerts = 0;

  emit getAlertsResult(success, jsonResponse);
}

void WebService::receivedDeleteAlertsResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyDeleteAlerts->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyDeleteAlerts->readAll();

    // Expected response is in JSON
    // { "result" : "description" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while deleting alerts: %1").arg(netReplyDeleteAlerts->errorString());
  }

  netReplyDeleteAlerts->deleteLater();
  netReplyDeleteAlerts = 0;

  emit deleteAlertsResult(success, jsonResponse);
}

void WebService::receivedGetDronesResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyGetDrones->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyGetDrones->readAll();

    // Expected response is in JSON
    // { "drone_hash" : 0, "data":[{"uuid":"","current_task_id":"","name":"","public_ip_address":"","local_ip_address":"","hostname":"","uptime":"","machine_type":"","os":"","check_in_time":"","formattedCheckInTime":""},
    //                             {...}]}

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while getting drones: %1").arg(netReplyGetDrones->errorString());
  }

  netReplyGetDrones->deleteLater();
  netReplyGetDrones = 0;

  emit getDronesResult(success, jsonResponse);
}

void WebService::receivedDeleteDronesResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyDeleteDrones->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyDeleteDrones->readAll();

    // Expected response is in JSON
    // { "result" : "description" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while deleting drones: %1").arg(netReplyDeleteDrones->errorString());
  }

  netReplyDeleteDrones->deleteLater();
  netReplyDeleteDrones = 0;

  emit deleteDronesResult(success, jsonResponse);
}

void WebService::receivedGetDroneTasksResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyGetDroneTasks->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyGetDroneTasks->readAll();

    // Expected response is in JSON
    // { "task_hash" : 0, "data":[{"uuid":"","current_task_id":"","name":"","public_ip_address":"","local_ip_address":"","hostname":"","uptime":"","machine_type":"","os":"","check_in_time":"","formattedCheckInTime":""},
    //                             {...}]}

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while getting drone tasks: %1").arg(netReplyGetDroneTasks->errorString());
  }

  netReplyGetDroneTasks->deleteLater();
  netReplyGetDroneTasks = 0;

  emit getDroneTasksResult(success, jsonResponse);
}

void WebService::receivedAddDroneTaskResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyAddDroneTask->error() == QNetworkReply::NoError)
  {
    // Expected response is in JSON
    // { "result" : "" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(formPost->response(), &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(formPost->response().constData());

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(formPost->response().constData());
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while adding drone task: %1").arg(netReplyAddDroneTask->errorString());
  }

  netReplyAddDroneTask->deleteLater();
  netReplyAddDroneTask = 0;

  emit addDroneTaskResult(success, jsonResponse);
}

void WebService::receivedDeleteDroneTasksResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyDeleteDroneTask->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyDeleteDroneTask->readAll();

    // Expected response is in JSON
    // { "result" : "" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while deleting drone tasks: %1").arg(netReplyDeleteDroneTask->errorString());
  }

  netReplyDeleteDroneTask->deleteLater();
  netReplyDeleteDroneTask = 0;

  emit deleteDroneTasksResult(success, jsonResponse);
}

void WebService::receivedGetLocationsResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyGetLocations->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyGetLocations->readAll();

    // Expected response is in JSON
    // { "location_hash" : 0, "data":[{"location_id":"","name":"","public_ip_address":"","collections_mailing_list":"","service_mailing_list":"","bill_acceptor_mailing_list":"","allow_notifications":"","show_collections":""},
    //                             {...}]}

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while getting locations: %1").arg(netReplyGetLocations->errorString());
  }

  netReplyGetLocations->deleteLater();
  netReplyGetLocations = 0;

  emit getLocationsResult(success, jsonResponse);
}

void WebService::receivedGetSoftwareResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyGetSoftware->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyGetSoftware->readAll();

    // Expected response is in JSON
    // { "software_hash" : 0, "data":[{"software_id":"","software_name":"","version":"","installer_url":"","hash":""},
    //                             {...}]}

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while getting software: %1").arg(netReplyGetSoftware->errorString());
  }

  netReplyGetSoftware->deleteLater();
  netReplyGetSoftware = 0;

  emit getSoftwareResult(success, jsonResponse);
}

void WebService::receivedAddSoftwareResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyAddSoftware->error() == QNetworkReply::NoError)
  {
    // Expected response is in JSON
    // { "result" : "" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(formPost->response(), &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(formPost->response().constData());

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(formPost->response().constData());
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while adding software: %1").arg(netReplyAddSoftware->errorString());
  }

  netReplyAddSoftware->deleteLater();
  netReplyAddSoftware = 0;

  emit addSoftwareResult(success, jsonResponse);
}

void WebService::receivedUpdateSoftwareResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyUpdateSoftware->error() == QNetworkReply::NoError)
  {
    // Expected response is in JSON
    // { "result" : "" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(formPost->response(), &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(formPost->response().constData());

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(formPost->response().constData());
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while updating software: %1").arg(netReplyUpdateSoftware->errorString());
  }

  netReplyUpdateSoftware->deleteLater();
  netReplyUpdateSoftware = 0;

  emit updateSoftwareResult(success, jsonResponse);
}

void WebService::receivedDeleteSoftwareResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyDeleteSoftware->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyDeleteSoftware->readAll();

    // Expected response is in JSON
    // { "result" : "" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while deleting software: %1").arg(netReplyDeleteSoftware->errorString());
  }

  netReplyDeleteSoftware->deleteLater();
  netReplyDeleteSoftware = 0;

  emit deleteSoftwareResult(success, jsonResponse);
}

void WebService::receivedAddLocationResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyAddLocation->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyAddLocation->readAll();

    // Expected response is in JSON
    // { "result" : "" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while deleting software: %1").arg(netReplyAddLocation->errorString());
  }

  netReplyAddLocation->deleteLater();
  netReplyAddLocation = 0;

  emit addLocationResult(success, jsonResponse);
}

void WebService::receivedUpdateLocationResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyUpdateLocation->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyUpdateLocation->readAll();

    // Expected response is in JSON
    // { "result" : "" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while deleting software: %1").arg(netReplyUpdateLocation->errorString());
  }

  netReplyUpdateLocation->deleteLater();
  netReplyUpdateLocation = 0;

  emit updateLocationResult(success, jsonResponse);
}

void WebService::receivedDeleteLocationsResponse()
{
  connectionTimer->stop();

  bool success = false;
  QVariantMap jsonResponse;

  // The finished signal is always emitted, even if there was a network error
  if (netReplyDeleteLocations->error() == QNetworkReply::NoError)
  {
    QByteArray json = netReplyDeleteLocations->readAll();

    // Expected response is in JSON
    // { "result" : "" }

    // If the server has a problem with what was sent, the response will be:
    // { "error" : "description" }

    QString response(json.constData());

    bool ok = false;
    QJson::Parser parser;
    QVariant var = parser.parse(json, &ok);

    if (ok)
    {
      QLOG_DEBUG() << QString("Received valid JSON response: %1").arg(response);

      jsonResponse = var.toMap();

      if (jsonResponse.contains("error"))
      {
        QLOG_ERROR() << QString("Server returned an error: %1").arg(jsonResponse["error"].toString());
      }
      else
      {
        success = true;
      }
    }
    else
    {
      QLOG_ERROR() << QString("Did not receive valid JSON response: %1").arg(response);
    }
  }
  else
  {
    QLOG_ERROR() << QString("Network error while deleting software: %1").arg(netReplyDeleteLocations->errorString());
  }

  netReplyDeleteLocations->deleteLater();
  netReplyDeleteLocations = 0;

  emit deleteLocationsResult(success, jsonResponse);
}

void WebService::resetTimeout(qint64, qint64)
{
  // Restart timer
  connectionTimer->start();
}

void WebService::abortConnection()
{
  QLOG_DEBUG() << "Aborting network connection due to timeout";

  if (netReplyGetAlerts)
    netReplyGetAlerts->abort();
  else if (netReplyDeleteAlerts)
    netReplyDeleteAlerts->abort();
  else if (netReplyGetDrones)
    netReplyGetDrones->abort();
  else if (netReplyDeleteDrones)
    netReplyDeleteDrones->abort();
  else if (netReplyGetDroneTasks)
    netReplyGetDroneTasks->abort();
  else if (netReplyGetLocations)
    netReplyGetLocations->abort();
  else if (netReplyGetSoftware)
    netReplyGetSoftware->abort();
  else if (netReplyAddDroneTask)
    netReplyAddDroneTask->abort();
  else if (netReplyAddSoftware)
    netReplyAddSoftware->abort();
  else if (netReplyDeleteSoftware)
    netReplyDeleteSoftware->abort();
  else if (netReplyUpdateSoftware)
    netReplyUpdateSoftware->abort();
  else if (netReplyDeleteDroneTask)
    netReplyDeleteDroneTask->abort();
  else if (netReplyAddLocation)
    netReplyAddLocation->abort();
  else if (netReplyUpdateLocation)
    netReplyUpdateLocation->abort();
  else if (netReplyDeleteLocations)
    netReplyDeleteLocations->abort();
  else
    QLOG_ERROR() << "No network connection appears to be valid, not aborting";  
}

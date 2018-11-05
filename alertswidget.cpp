#include "alertswidget.h"
#include "global.h"
#include "qslog/QsLog.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QKeyEvent>

AlertsWidget::AlertsWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent) : QWidget(parent)
{
  this->dbMgr = dbMgr;
  this->settings = settings;
  this->webService = webService;
  busy = false;
  firstLoad = true;  

  statusTimer = new QTimer;
  statusTimer->setInterval(settings->getValue("refresh_alerts_interval", REFRESH_ALERTS_INTERVAL).toInt());
  connect(statusTimer, SIGNAL(timeout()), this, SLOT(updateAlerts()));

  btnRefreshAlerts = new QPushButton(tr("Refresh"));
  connect(btnRefreshAlerts, SIGNAL(clicked()), this, SLOT(updateAlertsClicked()));

  btnDelete = new QPushButton(tr("Delete Alert(s)"));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteAlertsClicked()));

  btnPushAlerts = new QPushButton(tr("Push Unsent Alerts"));
  //connect(btnPushAlerts, SIGNAL(clicked()), this, SLOT(restartDevicesClicked()));

  connect(webService, SIGNAL(getAlertsResult(bool,QVariantMap)), this, SLOT(finishedDownloadingAlerts(bool,QVariantMap)));
  connect(webService, SIGNAL(deleteAlertsResult(bool,QVariantMap)), this, SLOT(finishedDeletingAlerts(bool,QVariantMap)));

  alertDetailWidget = new AlertDetailWidget;

  // Alert_ID, Date_Time, Location, Public_IP, Local_IP, Hostname, Uptime, Software, Version, Message, Sent
  alertsModel = new QStandardItemModel(0, 11);
  alertsModel->setHorizontalHeaderItem(Alert_ID, new QStandardItem(QString("ID")));
  alertsModel->setHorizontalHeaderItem(Date_Time, new QStandardItem(QString("Date/Time")));
  alertsModel->setHorizontalHeaderItem(Location, new QStandardItem(QString("Location")));
  alertsModel->setHorizontalHeaderItem(Public_IP, new QStandardItem(QString("Public IP")));
  alertsModel->setHorizontalHeaderItem(Local_IP, new QStandardItem(QString("Local IP")));
  alertsModel->setHorizontalHeaderItem(Hostname, new QStandardItem(QString("Hostname")));
  alertsModel->setHorizontalHeaderItem(Uptime, new QStandardItem(QString("Uptime")));
  alertsModel->setHorizontalHeaderItem(Software, new QStandardItem(QString("Software")));
  alertsModel->setHorizontalHeaderItem(Version, new QStandardItem(QString("Version")));
  alertsModel->setHorizontalHeaderItem(Message, new QStandardItem(QString("Message")));
  alertsModel->setHorizontalHeaderItem(Sent, new QStandardItem(QString("Sent")));

  sortFilter = new QSortFilterProxyModel;
  sortFilter->setSourceModel(alertsModel);
  sortFilter->setSortRole(Qt::UserRole + 1);

  lblAlerts = new QLabel(tr("Alerts"));
  alertsView = new QTableView;
  alertsView->setModel(sortFilter);
  alertsView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  alertsView->horizontalHeader()->setStretchLastSection(true);
  alertsView->horizontalHeader()->setStyleSheet("font:bold Arial;");
  alertsView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  alertsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  alertsView->setSelectionBehavior(QAbstractItemView::SelectRows);
  alertsView->setWordWrap(true);
  alertsView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  alertsView->verticalHeader()->hide();  
  alertsView->setAlternatingRowColors(true);
  alertsView->setSortingEnabled(true);
  //alertsView->setColumnHidden(Alert_ID, true);
  alertsView->setColumnHidden(Uptime, true);
  // Make table read-only
  alertsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  alertsView->installEventFilter(this);
  connect(alertsView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(alertDoubleclicked(QModelIndex)));

  buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(btnRefreshAlerts);
  buttonLayout->addWidget(btnDelete);
  buttonLayout->addWidget(btnPushAlerts);
  buttonLayout->addStretch(1);

  verticalLayout = new QVBoxLayout;
  verticalLayout->addWidget(lblAlerts);
  verticalLayout->addWidget(alertsView);
  verticalLayout->addLayout(buttonLayout);

  this->setLayout(verticalLayout);
}

AlertsWidget::~AlertsWidget()
{
  statusTimer->deleteLater();
  alertDetailWidget->deleteLater();
}

bool AlertsWidget::isBusy()
{
  return busy;
}

void AlertsWidget::showEvent(QShowEvent *)
{
  QLOG_DEBUG() << QString("Showing Alerts tab");

  // Load local copies of alerts from database
  // Send checksum of alerts from last download to web service
  // if the checksum has changed on the server then
  //   server responds with all alerts and new checksum
  //   delete local copy of alerts in database
  //   insert downloaded alerts
  //   update checksum
  //   update datagrid
  //   move to first row in datagrid
  // else
  //   server responds that there are no changes

  if (firstLoad)
  {
    QList<Alert> alerts = dbMgr->getAlertLogs();

    foreach (Alert a, alerts)
    {
      insertAlert(a);
    }

    firstLoad = false;

    alertsView->sortByColumn(Date_Time, Qt::DescendingOrder);
    alertsView->resizeColumnsToContents();
  }
}

bool AlertsWidget::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress)
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    if (keyEvent->key() == Qt::Key_Delete)
    {
      deleteAlertsClicked();
      return true;
    }
    else
    {
      // standard event processing
      return QWidget::eventFilter(obj, event);
    }
  }
  else
  {
    // standard event processing
    return QWidget::eventFilter(obj, event);
  }
}

void AlertsWidget::insertAlert(Alert alert)
{
  // Alert_ID, Date_Time, Location, Public_IP, Local_IP, Hostname, Uptime, Software, Version, Message, Sent
  QStandardItem *idField, *datetimeField, *locationField, *publicIpField, *localIpField,
      *hostnameField, *uptimeField, *softwareField, *versionField, *messageField, *sentField;

  idField = new QStandardItem(QString("%1").arg(alert.alertID));
  idField->setData(alert.alertID);

  datetimeField = new QStandardItem(QDateTime::fromTime_t(alert.dateTime).toString("MM/dd/yyyy h:mm:ss ap"));
  datetimeField->setData(alert.dateTime);

  locationField = new QStandardItem(alert.location);
  locationField->setData(alert.location);

  publicIpField = new QStandardItem(alert.publicIP);
  publicIpField->setData(alert.publicIP);

  localIpField = new QStandardItem(alert.localIP);
  localIpField->setData(alert.localIP);

  hostnameField = new QStandardItem(alert.hostname);
  hostnameField->setData(alert.hostname);

  uptimeField = new QStandardItem(alert.uptime);
  uptimeField->setData(alert.uptime);

  softwareField = new QStandardItem(alert.software);
  softwareField->setData(alert.software);

  versionField = new QStandardItem(alert.version);
  versionField->setData(alert.version);

  QString msg = alert.message;
  messageField = new QStandardItem(msg.length() > 50 ? msg.replace("\n", " ").replace("\r", " ").mid(0, 45) + "..." : msg.replace("\n", " ").replace("\r", " "));
  messageField->setData(alert.message);

  sentField = new QStandardItem(QString("%1").arg(alert.sent ? "True" : "False"));
  sentField->setData(alert.sent);

  int row = alertsModel->rowCount();

  alertsModel->setItem(row, Alert_ID, idField);
  alertsModel->setItem(row, Date_Time, datetimeField);
  alertsModel->setItem(row, Location, locationField);
  alertsModel->setItem(row, Public_IP, publicIpField);
  alertsModel->setItem(row, Local_IP, localIpField);
  alertsModel->setItem(row, Hostname, hostnameField);
  alertsModel->setItem(row, Uptime, uptimeField);
  alertsModel->setItem(row, Software, softwareField);
  alertsModel->setItem(row, Version, versionField);
  alertsModel->setItem(row, Message, messageField);
  alertsModel->setItem(row, Sent, sentField);
}

void AlertsWidget::populateTable()
{

}

void AlertsWidget::clearAlertsTable()
{
  if (alertsModel->rowCount() > 0)
  {
    alertsModel->removeRows(0, alertsModel->rowCount());
  }
}

void AlertsWidget::downloadingAlerts()
{
}

void AlertsWidget::finishedDownloadingAlerts(bool success, QVariantMap response)
{
  emit updateStatusMessage(tr("Finished downloading alerts."));

  // If successful it means either the alerts log was downloaded or nothing has changed so we already have the current alerts
  if (success)
  {
    if (response.empty())
      QLOG_DEBUG() << "We already have the current alert log";
    else
    {
      // Replace alerts in datagrid with what we just downloaded
      clearAlertsTable();

      if (response.contains("data") && response.contains("alert_hash"))
      {
        settings->setValue("alert_hash", response["alert_hash"].toLongLong());

        dbMgr->insertAlertLogs(response["data"].toList());

        // alert_id, alert_time, name, public_ip_address, local_ip_address, hostname, uptime, software_name, version, log_message, sent
        foreach (QVariant v, response["data"].toList())
        {
          QVariantMap alert = v.toMap();

          Alert a;
          a.alertID = alert["alert_id"].toInt();
          a.dateTime = alert["alert_time"].toUInt();
          a.location = alert["name"].toString();
          a.publicIP = alert["public_ip_address"].toString();
          a.localIP = alert["local_ip_address"].toString();
          a.hostname = alert["hostname"].toString();
          a.uptime = alert["uptime"].toString();
          a.software = alert["software_name"].toString();
          a.version = alert["version"].toString();
          a.message = alert["log_message"].toString();
          a.sent = alert["sent"].toInt() == 1;

          insertAlert(a);
        }

        alertsView->sortByColumn(alertsView->horizontalHeader()->sortIndicatorSection(), alertsView->horizontalHeader()->sortIndicatorOrder());
        alertsView->resizeColumnsToContents();
        alertsView->scrollToTop();
      }
    }
  }

  this->setEnabled(true);
}

void AlertsWidget::updateAlerts()
{
}

void AlertsWidget::deleteAlertsClicked()
{
  QModelIndexList selectedRows = alertsView->selectionModel()->selectedRows(Alert_ID);

  if (selectedRows.count() == 0)
  {
    QLOG_DEBUG() << "User pressed Delete key without any rows selected in alerts table";
    QMessageBox::warning(this, tr("Delete Alert(s)"), tr("Select one or more rows in the table above."));
  }
  else
  {
    QLOG_DEBUG() << "User selected" << selectedRows.count() << "rows to delete from the alerts table";

    // ask user if he wants to delete video
    // delete entire directory and entry in database if it's not currently in an active moiv change
    if (QMessageBox::question(this, tr("Delete Alert(s)"), tr("Are you sure you want to delete the selected %1 alert(s)?").arg(selectedRows.count()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      this->setEnabled(false);

      QLOG_DEBUG() << "User chose to delete alert(s)";

      QStringList alertIdList;

      // Delete one or more rows from the model and database
      //qDebug() << selectedRows.count() << "rows selected";
      while (selectedRows.count() > 0)
      {
        QModelIndex index = selectedRows.at(0);

        alertIdList.append(index.data(Qt::UserRole + 1).value<QString>());

        sortFilter->removeRow(index.row());

        selectedRows = alertsView->selectionModel()->selectedRows(Alert_ID);
      }

      //QLOG_DEBUG() << alertIdList;

      emit updateStatusMessage(tr("Deleting alert(s)..."));

      webService->startdeleteAlerts(alertIdList, false);
    }
    else
    {
      QLOG_DEBUG() << "User chose not to delete the alerts";
    }
  }
}

void AlertsWidget::updateAlertsClicked()
{
  this->setEnabled(false);
  emit updateStatusMessage(tr("Checking alerts..."));
  webService->startGetAlerts(settings->getValue("alert_hash", ALERT_HASH).toLongLong());
}

void AlertsWidget::finishedDeletingAlerts(bool success, QVariantMap response)
{
  emit updateStatusMessage(tr("Finished deleting alert(s)."));

  // If successful it means the selected alert records were deleted from the database
  if (success)
  {
    if (response.contains("result"))
    {
      QMessageBox::information(this, tr("Delete Alert(s)"), response["result"].toString());
    }
  }
  else
  {
    if (response.contains("error"))
    {
      QMessageBox::warning(this, tr("Delete Alert(s)"), response["error"].toString());
    }
  }

  this->setEnabled(true);
}

void AlertsWidget::alertDoubleclicked(const QModelIndex &index)
{
  QLOG_DEBUG() << QString("User double-clicked alert in datagrid");

  Alert alert;
  alert.alertID = sortFilter->index(index.row(), Alert_ID).data(Qt::UserRole + 1).toInt();
  alert.dateTime = sortFilter->index(index.row(), Date_Time).data(Qt::UserRole + 1).toUInt();
  alert.location = sortFilter->index(index.row(), Location).data().toString();
  alert.publicIP = sortFilter->index(index.row(), Public_IP).data().toString();
  alert.localIP = sortFilter->index(index.row(), Local_IP).data().toString();
  alert.hostname = sortFilter->index(index.row(), Hostname).data().toString();
  alert.uptime = sortFilter->index(index.row(), Uptime).data().toString();
  alert.software = sortFilter->index(index.row(), Software).data().toString();
  alert.version = sortFilter->index(index.row(), Version).data().toString();
  alert.message = sortFilter->index(index.row(), Message).data(Qt::UserRole + 1).toString();
  alert.sent = sortFilter->index(index.row(), Sent).data().toBool();

  alertDetailWidget->populateForm(alert);


  alertDetailWidget->show();

  // In case the window was already open, bring it to the foreground
  alertDetailWidget->raise();
  alertDetailWidget->activateWindow();
}

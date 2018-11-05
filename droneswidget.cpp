#include "droneswidget.h"
#include "global.h"
#include "qslog/QsLog.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QKeyEvent>

DronesWidget::DronesWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent) : QWidget(parent)
{
  this->dbMgr = dbMgr;
  this->settings = settings;
  this->webService = webService;
  busy = false;
  firstLoad = true;

  statusTimer = new QTimer;
  statusTimer->setInterval(settings->getValue("refresh_drones_interval", REFRESH_DRONES_INTERVAL).toInt());
  connect(statusTimer, SIGNAL(timeout()), this, SLOT(updateDrones()));

  btnRefreshDrones = new QPushButton(tr("Refresh"));
  connect(btnRefreshDrones, SIGNAL(clicked()), this, SLOT(updateDronesClicked()));

  btnDelete = new QPushButton(tr("Delete Drone(s)"));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteDronesClicked()));

  connect(webService, SIGNAL(getDronesResult(bool,QVariantMap)), this, SLOT(finishedDownloadingDrones(bool,QVariantMap)));
  connect(webService, SIGNAL(deleteDronesResult(bool,QVariantMap)), this, SLOT(finishedDeletingDrones(bool,QVariantMap)));

  // UUID, Current_Task_ID, Location, Public_IP, Local_IP, Hostname, Uptime, Machine_Type, Operating_System, Check_In_Time
  dronesModel = new QStandardItemModel(0, 10);
  dronesModel->setHorizontalHeaderItem(UUID, new QStandardItem(QString("UUID")));
  dronesModel->setHorizontalHeaderItem(Current_Task_ID, new QStandardItem(QString("Current Task")));
  dronesModel->setHorizontalHeaderItem(Location, new QStandardItem(QString("Location")));
  dronesModel->setHorizontalHeaderItem(Public_IP, new QStandardItem(QString("Public IP")));
  dronesModel->setHorizontalHeaderItem(Local_IP, new QStandardItem(QString("Local IP")));
  dronesModel->setHorizontalHeaderItem(Hostname, new QStandardItem(QString("Hostname")));
  dronesModel->setHorizontalHeaderItem(Uptime, new QStandardItem(QString("Uptime")));
  dronesModel->setHorizontalHeaderItem(Machine_Type, new QStandardItem(QString("Machine Type")));
  dronesModel->setHorizontalHeaderItem(Operating_System, new QStandardItem(QString("OS")));
  dronesModel->setHorizontalHeaderItem(Check_In_Time, new QStandardItem(QString("Check In Time")));

  sortFilter = new QSortFilterProxyModel;
  sortFilter->setSourceModel(dronesModel);
  sortFilter->setSortRole(Qt::UserRole + 1);

  lblDrones = new QLabel(tr("Drones"));
  dronesView = new QTableView;
  dronesView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  dronesView->horizontalHeader()->setStretchLastSection(true);
  dronesView->horizontalHeader()->setStyleSheet("font:bold Arial;");
  dronesView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  dronesView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  dronesView->setSelectionBehavior(QAbstractItemView::SelectRows);
  dronesView->setWordWrap(true);
  dronesView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);  
  dronesView->verticalHeader()->hide();
  dronesView->setModel(sortFilter);
  dronesView->setAlternatingRowColors(true);
  dronesView->setSortingEnabled(true);
  dronesView->setColumnHidden(UUID, true);
  dronesView->setColumnHidden(Uptime, true);
  dronesView->installEventFilter(this);
  // Make table read-only
  dronesView->setEditTriggers(QAbstractItemView::NoEditTriggers);  

  buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(btnRefreshDrones);
  buttonLayout->addWidget(btnDelete);
  buttonLayout->addStretch(1);

  verticalLayout = new QVBoxLayout;
  verticalLayout->addWidget(lblDrones);
  verticalLayout->addWidget(dronesView);
  verticalLayout->addLayout(buttonLayout);

  this->setLayout(verticalLayout);
}

DronesWidget::~DronesWidget()
{
  statusTimer->deleteLater();
}

bool DronesWidget::isBusy()
{
  return busy;
}

void DronesWidget::showEvent(QShowEvent *)
{
  QLOG_DEBUG() << QString("Showing Drones tab");

  // Load local copies of drones from database
  // Send checksum of drones from last download to web service
  // if the checksum has changed on the server then
  //   server responds with all drones and new checksum
  //   delete local copy of drones in database
  //   insert downloaded drones
  //   update checksum
  //   update datagrid
  //   move to first row in datagrid
  // else
  //   server responds that there are no changes

  if (firstLoad)
  {
    QList<Drone> drones = dbMgr->getDrones();

    foreach (Drone d, drones)
    {
      insertDrone(d);
    }

    firstLoad = false;

    dronesView->sortByColumn(Check_In_Time, Qt::AscendingOrder);
    dronesView->resizeColumnsToContents();
  }
}

bool DronesWidget::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress)
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    if (keyEvent->key() == Qt::Key_Delete)
    {
      deleteDronesClicked();
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

void DronesWidget::insertDrone(Drone drone)
{
  // UUID, Current_Task_ID, Location, Public_IP, Local_IP, Hostname, Uptime, Machine_Type, Operating_System, Check_In_Time
  QStandardItem *idField, *currentTaskIdField, *locationField, *publicIpField, *localIpField,
      *hostnameField, *uptimeField, *machineTypeField, *osField, *checkInTimeField;

  idField = new QStandardItem(drone.uuid);
  idField->setData(drone.uuid);

  currentTaskIdField = new QStandardItem(QString("%1").arg(drone.currentTaskID));
  currentTaskIdField->setData(drone.currentTaskID);

  locationField = new QStandardItem(drone.location);
  locationField->setData(drone.location);

  publicIpField = new QStandardItem(drone.publicIP);
  publicIpField->setData(drone.publicIP);

  localIpField = new QStandardItem(drone.localIP);
  localIpField->setData(drone.localIP);

  hostnameField = new QStandardItem(drone.hostname);
  hostnameField->setData(drone.hostname);

  uptimeField = new QStandardItem(drone.uptime);
  uptimeField->setData(drone.uptime);

  machineTypeField = new QStandardItem(drone.machineType);
  machineTypeField->setData(drone.machineType);

  osField = new QStandardItem(drone.os);
  osField->setData(drone.os);

  checkInTimeField = new QStandardItem(QString("%1 (%2)").arg(QDateTime::fromTime_t(drone.checkInTime).toString("MM/dd/yyyy h:mm:ss ap")).arg(Global::secondsToDHMS(QDateTime::currentDateTime().toTime_t() - drone.checkInTime)));
  checkInTimeField->setData(drone.checkInTime);

  int row = dronesModel->rowCount();

  dronesModel->setItem(row, UUID, idField);
  dronesModel->setItem(row, Current_Task_ID, currentTaskIdField);
  dronesModel->setItem(row, Location, locationField);
  dronesModel->setItem(row, Public_IP, publicIpField);
  dronesModel->setItem(row, Local_IP, localIpField);
  dronesModel->setItem(row, Hostname, hostnameField);
  dronesModel->setItem(row, Uptime, uptimeField);
  dronesModel->setItem(row, Machine_Type, machineTypeField);
  dronesModel->setItem(row, Operating_System, osField);
  dronesModel->setItem(row, Check_In_Time, checkInTimeField);
}

void DronesWidget::populateTable()
{

}

void DronesWidget::clearDronesTable()
{
  if (dronesModel->rowCount() > 0)
  {
    dronesModel->removeRows(0, dronesModel->rowCount());
  }
}

void DronesWidget::downloadingAlerts()
{
}

void DronesWidget::finishedDownloadingDrones(bool success, QVariantMap response)
{
  // If successful it means either the drones were downloaded or nothing has changed so we already have the current drones
  if (success)
  {
    if (response.empty())
      QLOG_DEBUG() << "We already have the current drones";
    else
    {
      // Replace alerts in datagrid with what we just downloaded
      clearDronesTable();

      if (response.contains("data") && response.contains("drone_hash"))
      {
        settings->setValue("drone_hash", response["drone_hash"].toLongLong());

        dbMgr->insertDrones(response["data"].toList());

        // uuid, current_task_id, name, public_ip_address, local_ip_address, hostname, uptime, machine_type, operating_system, check_in_time, formattedCheckInTime
        foreach (QVariant v, response["data"].toList())
        {
          QVariantMap drone = v.toMap();

          Drone d;
          d.uuid = drone["uuid"].toString();
          d.currentTaskID = drone["current_task_id"].toInt();
          d.location = drone["name"].toString();
          d.publicIP = drone["public_ip_address"].toString();
          d.localIP = drone["local_ip_address"].toString();
          d.hostname = drone["hostname"].toString();
          d.uptime = drone["uptime"].toString();
          d.machineType = drone["machine_type"].toString();
          d.os = drone["operating_system"].toString();
          d.checkInTime = drone["check_in_time"].toInt();
          d.formattedCheckInTime = drone["formattedCheckInTime"].toString();

          insertDrone(d);
        }

        dronesView->sortByColumn(dronesView->horizontalHeader()->sortIndicatorSection(), dronesView->horizontalHeader()->sortIndicatorOrder());
        dronesView->resizeColumnsToContents();
        dronesView->scrollToTop();
      }
    }
  }

  this->setEnabled(true);
}

void DronesWidget::updateDrones()
{
}

void DronesWidget::deleteDronesClicked()
{
  QModelIndexList selectedRows = dronesView->selectionModel()->selectedRows(UUID);

  if (selectedRows.count() == 0)
  {
    QLOG_DEBUG() << "User pressed Delete key without any rows selected in drones table";
    QMessageBox::warning(this, tr("Delete Drone(s)"), tr("Select one or more rows in the table above."));
  }
  else
  {
    QLOG_DEBUG() << "User selected" << selectedRows.count() << "rows to delete from the drones table";

    if (QMessageBox::question(this, tr("Delete Drone(s)"), tr("Are you sure you want to delete the selected %1 drone(s)?").arg(selectedRows.count()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      this->setEnabled(false);

      QLOG_DEBUG() << "User chose to delete drone(s)";

      QStringList uuidList;

      // Delete one or more rows from the model and database
      //qDebug() << selectedRows.count() << "rows selected";
      while (selectedRows.count() > 0)
      {
        QModelIndex index = selectedRows.at(0);

        uuidList.append(index.data(Qt::UserRole + 1).value<QString>());

        sortFilter->removeRow(index.row());

        selectedRows = dronesView->selectionModel()->selectedRows(UUID);
      }

      webService->startDeleteDrones(uuidList);
    }
    else
    {
      QLOG_DEBUG() << "User chose not to delete the drone(s)";
    }
  }
}

void DronesWidget::updateDronesClicked()
{
  this->setEnabled(false);
  webService->startGetDrones(settings->getValue("drone_hash", DRONE_HASH).toLongLong());
}

void DronesWidget::finishedDeletingDrones(bool success, QVariantMap response)
{
  // If successful it means the drones were deleted
  if (success)
  {
    QMessageBox::information(this, tr("Delete Drone(s)"), response["result"].toString());
  }
  else
  {
    QMessageBox::warning(this, tr("Delete Drone(s)"), response["error"].toString());
  }

  this->setEnabled(true);
}

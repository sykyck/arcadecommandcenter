#include "dronetaskswidget.h"
#include "global.h"
#include "qslog/QsLog.h"
#include <QMessageBox>
#include <QHeaderView>

DroneTasksWidget::DroneTasksWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent) : QWidget(parent)
{
  this->dbMgr = dbMgr;
  this->settings = settings;
  this->webService = webService;
  busy = false;
  firstLoad = true;

  statusTimer = new QTimer;
  statusTimer->setInterval(settings->getValue("refresh_drone_tasks_interval", REFRESH_DRONE_TASKS_INTERVAL).toInt());
  connect(statusTimer, SIGNAL(timeout()), this, SLOT(updateTasks()));

  btnRefreshTasks = new QPushButton(tr("Refresh"));
  connect(btnRefreshTasks, SIGNAL(clicked()), this, SLOT(updateTasksClicked()));

  btnDelete = new QPushButton(tr("Delete Task(s)"));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteTasksClicked()));

  btnNewTask = new QPushButton(tr("Add Task"));
  connect(btnNewTask, SIGNAL(clicked()), this, SLOT(addTaskClicked()));

  connect(webService, SIGNAL(getDroneTasksResult(bool,QVariantMap)), this, SLOT(finishedDownloadingTasks(bool,QVariantMap)));
  connect(webService, SIGNAL(deleteDroneTasksResult(bool,QVariantMap)), this, SLOT(finishedDeletingTasks(bool,QVariantMap)));

  // task_id, task_name, package_url, expiration_date, num_rules, num_drones_finished, num_drones_applicable
  droneTasksModel = new QStandardItemModel(0, 7);
  droneTasksModel->setHorizontalHeaderItem(Task_ID, new QStandardItem(QString("Task ID")));
  droneTasksModel->setHorizontalHeaderItem(Task_Name, new QStandardItem(QString("Name")));
  droneTasksModel->setHorizontalHeaderItem(Package_URL, new QStandardItem(QString("Package URL")));
  droneTasksModel->setHorizontalHeaderItem(Expiration, new QStandardItem(QString("Expiration")));
  droneTasksModel->setHorizontalHeaderItem(Num_Rules, new QStandardItem(QString("# Rules")));
  droneTasksModel->setHorizontalHeaderItem(Num_Drones_Finished, new QStandardItem(QString("# Finished")));
  droneTasksModel->setHorizontalHeaderItem(Num_Drones_Applicable, new QStandardItem(QString("# Applicable")));

  lblDroneTasks = new QLabel(tr("Drone Tasks"));
  droneTasksView = new QTableView;
  droneTasksView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  droneTasksView->horizontalHeader()->setStretchLastSection(true);
  droneTasksView->horizontalHeader()->setStyleSheet("font:bold Arial;");
  droneTasksView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  droneTasksView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  droneTasksView->setSelectionBehavior(QAbstractItemView::SelectRows);
  droneTasksView->setWordWrap(true);
  droneTasksView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  droneTasksView->verticalHeader()->hide();
  droneTasksView->setModel(droneTasksModel);
  droneTasksView->setAlternatingRowColors(true);
  droneTasksView->setColumnHidden(Package_URL, true);

  // Make table read-only
  droneTasksView->setEditTriggers(QAbstractItemView::NoEditTriggers);

  buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(btnRefreshTasks);
  buttonLayout->addWidget(btnNewTask);
  buttonLayout->addWidget(btnDelete);
  buttonLayout->addStretch(1);

  verticalLayout = new QVBoxLayout;
  verticalLayout->addWidget(lblDroneTasks);
  verticalLayout->addWidget(droneTasksView);
  verticalLayout->addLayout(buttonLayout);

  this->setLayout(verticalLayout);
}

DroneTasksWidget::~DroneTasksWidget()
{
  statusTimer->deleteLater();
}

bool DroneTasksWidget::isBusy()
{
  return busy;
}

void DroneTasksWidget::showEvent(QShowEvent *)
{
  QLOG_DEBUG() << QString("Showing Drone Tasks tab");

  // Load local copies of drone tasks from database
  // Send checksum of drone tasks from last download to web service
  // if the checksum has changed on the server then
  //   server responds with all drone tasks and new checksum
  //   delete local copy of drone tasks in database
  //   insert downloaded drone tasks
  //   update checksum
  //   update datagrid
  //   move to first row in datagrid
  // else
  //   server responds that there are no changes

  if (firstLoad)
  {
    QList<DroneTask> droneTasks = dbMgr->getDroneTasks();

    foreach (DroneTask d, droneTasks)
    {
      insertRecord(d);
    }

    firstLoad = false;
  }
}

void DroneTasksWidget::insertRecord(DroneTask task)
{
  // task_id, task_name, package_url, expiration_date, num_rules, num_drones_finished, num_drones_applicable
  QStandardItem *idField, *taskNameField, *packageUrlField, *expirationField, *numRulesField,
      *numDronesApplicableField, *numDronesFinishedField;

  idField = new QStandardItem(QString("%1").arg(task.taskID));
  idField->setData(task.taskID);

  taskNameField = new QStandardItem(task.name);
  taskNameField->setData(task.name);

  packageUrlField = new QStandardItem(task.packageURL);
  packageUrlField->setData(task.packageURL);

  expirationField = new QStandardItem(QString("%1").arg(task.expiration_date == 0 ? "N/A" : QDateTime::fromTime_t(task.expiration_date).toString("MM/dd/yyyy h:mm:ss ap")));
  expirationField->setData(task.expiration_date);

  numRulesField = new QStandardItem(QString("%1").arg(task.numRules));
  numRulesField->setData(task.numRules);

  numDronesFinishedField = new QStandardItem(QString("%1").arg(task.numDronesFinished));
  numDronesFinishedField->setData(task.numDronesFinished);

  numDronesApplicableField = new QStandardItem(QString("%1").arg(task.numDronesApplicable));
  numDronesApplicableField->setData(task.numDronesApplicable);

  int row = droneTasksModel->rowCount();

  droneTasksModel->setItem(row, Task_ID, idField);
  droneTasksModel->setItem(row, Task_Name, taskNameField);
  droneTasksModel->setItem(row, Package_URL, packageUrlField);
  droneTasksModel->setItem(row, Expiration, expirationField);
  droneTasksModel->setItem(row, Num_Rules, numRulesField);
  droneTasksModel->setItem(row, Num_Drones_Finished, numDronesFinishedField);
  droneTasksModel->setItem(row, Num_Drones_Applicable, numDronesApplicableField);
}

void DroneTasksWidget::populateTable()
{

}

void DroneTasksWidget::clearTable()
{
  if (droneTasksModel->rowCount() > 0)
  {
    droneTasksModel->removeRows(0, droneTasksModel->rowCount());
  }
}

void DroneTasksWidget::downloadingTasks()
{
}

void DroneTasksWidget::finishedDownloadingTasks(bool success, QVariantMap response)
{
  // If successful it means either the drones were downloaded or nothing has changed so we already have the current drones
  if (success)
  {
    if (response.empty())
      QLOG_DEBUG() << "We already have the current drone tasks";
    else
    {
      // Replace alerts in datagrid with what we just downloaded
      clearTable();

      if (response.contains("data") && response.contains("task_hash"))
      {
        settings->setValue("task_hash", response["task_hash"].toLongLong());

        dbMgr->insertDroneTasks(response["data"].toList());

        // task_id, task_name, package_url, expiration_date, num_rules, num_done, num_app
        foreach (QVariant v, response["data"].toList())
        {
          QVariantMap task = v.toMap();

          DroneTask t;
          t.taskID = task["task_id"].toInt();
          t.name = task["task_name"].toString();
          t.packageURL = task["package_url"].toString();
          t.expiration_date = task["expiration_date"].toInt();
          t.numRules = task["num_rules"].toInt();
          t.numDronesFinished = task["num_done"].toInt();
          t.numDronesApplicable = task["num_app"].toInt();

          insertRecord(t);
        }
      }
    }
  }

  this->setEnabled(true);
}

void DroneTasksWidget::updateTasks()
{
}

void DroneTasksWidget::updateTasksClicked()
{
  this->setEnabled(false);
  webService->startGetDroneTasks(settings->getValue("task_hash", TASK_HASH).toLongLong());
}

void DroneTasksWidget::addTaskClicked()
{
  EditDroneTaskWidget editDroneTask(dbMgr, settings, webService);
  if (editDroneTask.exec() == QDialog::Accepted)
    updateTasksClicked();
}

void DroneTasksWidget::deleteTasksClicked()
{
  QModelIndexList selectedRows = droneTasksView->selectionModel()->selectedRows(Task_ID);

  if (selectedRows.count() == 0)
  {
    QLOG_DEBUG() << "User pressed Delete key without any rows selected in drone tasks table";
    QMessageBox::warning(this, tr("Delete Task(s)"), tr("Select one or more rows in the table above."));
  }
  else
  {
    QLOG_DEBUG() << "User selected" << selectedRows.count() << "rows to delete from the drone tasks table";

    // ask user if he wants to delete drone tasks
    if (QMessageBox::question(this, tr("Delete Task(s)"), tr("Are you sure you want to delete the selected %1 drone task(s)?").arg(selectedRows.count()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      this->setEnabled(false);

      QLOG_DEBUG() << "User chose to delete drone task(s)";

      QStringList taskIdList;

      // Delete one or more rows from the model and database
      //qDebug() << selectedRows.count() << "rows selected";
      while (selectedRows.count() > 0)
      {
        QModelIndex index = selectedRows.at(0);

        taskIdList.append(index.data(Qt::UserRole + 1).value<QString>());

        droneTasksModel->removeRow(index.row());

        selectedRows = droneTasksView->selectionModel()->selectedRows(Task_ID);
      }

      webService->startDeleteDroneTasks(taskIdList);
    }
    else
    {
      QLOG_DEBUG() << "User chose not to delete the drone task";
    }
  }
}

void DroneTasksWidget::finishedDeletingTasks(bool success, QVariantMap response)
{
  // If successful it means the drone tasks were deleted
  if (success)
  {
    QMessageBox::information(this, tr("Delete Task(s)"), response["result"].toString());
  }
  else
  {
    QMessageBox::warning(this, tr("Delete Task(s)"), response["error"].toString());
  }

  this->setEnabled(true);
}

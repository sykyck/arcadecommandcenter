#include "softwarewidget.h"
#include "global.h"
#include "qslog/QsLog.h"
#include <QMessageBox>
#include <QHeaderView>

SoftwareWidget::SoftwareWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent) : QWidget(parent)
{
  this->dbMgr = dbMgr;
  this->settings = settings;
  this->webService = webService;
  busy = false;
  firstLoad = true;

  statusTimer = new QTimer;
  statusTimer->setInterval(settings->getValue("refresh_software_interval", REFRESH_SOFTWARE_INTERVAL).toInt());
  connect(statusTimer, SIGNAL(timeout()), this, SLOT(updateSoftware()));

  btnRefreshSoftware = new QPushButton(tr("Refresh"));
  connect(btnRefreshSoftware, SIGNAL(clicked()), this, SLOT(updateSoftwareClicked()));

  btnDelete = new QPushButton(tr("Delete Software"));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteSoftwareClicked()));

  btnNewSoftware = new QPushButton(tr("Add Software"));
  connect(btnNewSoftware, SIGNAL(clicked()), this, SLOT(addSoftwareClicked()));

  connect(webService, SIGNAL(getSoftwareResult(bool,QVariantMap)), this, SLOT(finishedDownloadingSoftware(bool,QVariantMap)));
  connect(webService, SIGNAL(deleteSoftwareResult(bool,QVariantMap)), this, SLOT(finishedDeletingSoftware(bool,QVariantMap)));

  // software_id, software_name, version, installer_url, hash
  softwareModel = new QStandardItemModel(0, 5);
  softwareModel->setHorizontalHeaderItem(Software_ID, new QStandardItem(QString("Software ID")));
  softwareModel->setHorizontalHeaderItem(Name, new QStandardItem(QString("Name")));
  softwareModel->setHorizontalHeaderItem(Version, new QStandardItem(QString("Version")));
  softwareModel->setHorizontalHeaderItem(Installer_URL, new QStandardItem(QString("Installer URL")));
  softwareModel->setHorizontalHeaderItem(Hash, new QStandardItem(QString("Hash")));

  lblSoftware = new QLabel(tr("Software"));
  softwareView = new QTableView;
  softwareView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  softwareView->horizontalHeader()->setStretchLastSection(true);
  softwareView->horizontalHeader()->setStyleSheet("font:bold Arial;");
  softwareView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  softwareView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  softwareView->setSelectionBehavior(QAbstractItemView::SelectRows);
  softwareView->setWordWrap(true);
  softwareView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  softwareView->verticalHeader()->hide();
  softwareView->setModel(softwareModel);
  softwareView->setAlternatingRowColors(true);
  softwareView->setColumnHidden(Software_ID, true);
  softwareView->setColumnHidden(Hash, true);
  // Make table read-only
  softwareView->setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(softwareView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(softwareDoubleClicked(QModelIndex)));

  buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(btnRefreshSoftware);
  buttonLayout->addWidget(btnNewSoftware);
  buttonLayout->addWidget(btnDelete);
  buttonLayout->addStretch(1);

  verticalLayout = new QVBoxLayout;
  verticalLayout->addWidget(lblSoftware);
  verticalLayout->addWidget(softwareView);
  verticalLayout->addLayout(buttonLayout);

  this->setLayout(verticalLayout);
}

SoftwareWidget::~SoftwareWidget()
{
  statusTimer->deleteLater();
}

bool SoftwareWidget::isBusy()
{
  return busy;
}

void SoftwareWidget::showEvent(QShowEvent *)
{
  QLOG_DEBUG() << QString("Showing Software tab");

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
    QList<Software> softwareList = dbMgr->getSoftware();

    foreach (Software s, softwareList)
    {
      insertRecord(s);
    }

    firstLoad = false;
  }
}

void SoftwareWidget::insertRecord(Software s)
{
  // software_id, software_name, version, installer_url, hash
  QStandardItem *idField, *nameField, *versionField, *installerUrlField, *hashField;

  idField = new QStandardItem(QString("%1").arg(s.softwareID));
  idField->setData(s.softwareID);

  nameField = new QStandardItem(s.name);
  nameField->setData(s.name);

  versionField = new QStandardItem(s.version);
  versionField->setData(s.version);

  installerUrlField = new QStandardItem(s.installerURL);
  installerUrlField->setData(s.installerURL);

  hashField = new QStandardItem(s.hash);
  hashField->setData(s.hash);

  int row = softwareModel->rowCount();

  softwareModel->setItem(row, Software_ID, idField);
  softwareModel->setItem(row, Name, nameField);
  softwareModel->setItem(row, Version, versionField);
  softwareModel->setItem(row, Installer_URL, installerUrlField);
  softwareModel->setItem(row, Hash, hashField);
}

void SoftwareWidget::populateTable()
{

}

void SoftwareWidget::clearTable()
{
  if (softwareModel->rowCount() > 0)
  {
    softwareModel->removeRows(0, softwareModel->rowCount());
  }
}

void SoftwareWidget::downloadingSoftware()
{
}

void SoftwareWidget::finishedDownloadingSoftware(bool success, QVariantMap response)
{
  // If successful it means either the software were downloaded or nothing has changed so we already have the current software
  if (success)
  {
    if (response.empty())
      QLOG_DEBUG() << "We already have the current software";
    else
    {
      // Replace alerts in datagrid with what we just downloaded
      clearTable();

      if (response.contains("data") && response.contains("software_hash"))
      {
        settings->setValue("software_hash", response["software_hash"].toLongLong());

        dbMgr->insertSoftware(response["data"].toList());

        // software_id, software_name, version, installer_url, hash
        foreach (QVariant v, response["data"].toList())
        {
          QVariantMap software = v.toMap();

          Software s;
          s.softwareID = software["software_id"].toInt();
          s.name = software["software_name"].toString();
          s.version = software["version"].toString();
          s.installerURL = software["installer_url"].toString();
          s.hash = software["hash"].toString();

          insertRecord(s);
        }
      }
    }
  }

  this->setEnabled(true);
}

void SoftwareWidget::updateSoftware()
{
}

void SoftwareWidget::updateSoftwareClicked()
{
  this->setEnabled(false);
  webService->startGetSoftware(settings->getValue("software_hash", SOFTWARE_HASH).toLongLong());
}

void SoftwareWidget::addSoftwareClicked()
{
  EditSoftwareWidget editSoftware(dbMgr, settings, webService);
  if (editSoftware.exec() == QDialog::Accepted)
    updateSoftwareClicked();
}

void SoftwareWidget::deleteSoftwareClicked()
{
  QModelIndexList selectedRows = softwareView->selectionModel()->selectedRows(Software_ID);

  if (selectedRows.count() == 0)
  {
    QLOG_DEBUG() << "User pressed Delete key without any rows selected in software table";
    QMessageBox::warning(this, tr("Delete Software"), tr("Select one or more rows in the table above."));
  }
  else
  {
    QLOG_DEBUG() << "User selected" << selectedRows.count() << "rows to delete from the software table";

    if (QMessageBox::question(this, tr("Delete Software"), tr("Are you sure you want to delete the selected software?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      this->setEnabled(false);

      QLOG_DEBUG() << "User chose to delete software";

      QStringList softwareIdList;

      // Delete one or more rows from the model and database
      //qDebug() << selectedRows.count() << "rows selected";
      while (selectedRows.count() > 0)
      {
        QModelIndex index = selectedRows.at(0);

        softwareIdList.append(index.data(Qt::UserRole + 1).value<QString>());

        softwareModel->removeRow(index.row());

        selectedRows = softwareView->selectionModel()->selectedRows(Software_ID);
      }

      webService->startDeleteSoftware(softwareIdList);
    }
    else
    {
      QLOG_DEBUG() << "User chose not to delete the software";
    }
  }
}

void SoftwareWidget::finishedDeletingSoftware(bool success, QVariantMap response)
{
  // If successful it means the software was deleted
  if (success)
  {
    QMessageBox::information(this, tr("Delete Software"), response["result"].toString());
  }
  else
  {
    QMessageBox::warning(this, tr("Delete Software"), response["error"].toString());
  }

  this->setEnabled(true);
}

void SoftwareWidget::softwareDoubleClicked(const QModelIndex &index)
{
  QStandardItem *idField, *nameField, *versionField, *installerUrlField, *hashField;

  idField = softwareModel->item(index.row(), Software_ID);
  nameField = softwareModel->item(index.row(), Name);
  versionField = softwareModel->item(index.row(), Version);
  installerUrlField = softwareModel->item(index.row(), Installer_URL);
  hashField = softwareModel->item(index.row(), Hash);

  Software software;
  software.softwareID = idField->data().toInt();
  software.name = nameField->data().toString();
  software.version = versionField->data().toString();
  software.installerURL = installerUrlField->data().toString();
  software.hash = hashField->data().toString();

  EditSoftwareWidget editSoftware(dbMgr, settings, webService);
  editSoftware.populateForm(software);

  if (editSoftware.exec() == QDialog::Accepted)
    updateSoftwareClicked();
}

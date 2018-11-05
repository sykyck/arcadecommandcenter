#include "locationswidget.h"
#include "global.h"
#include "qslog/QsLog.h"
#include <QMessageBox>
#include <QHeaderView>
#include "editlocationwidget.h"

LocationsWidget::LocationsWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent) : QWidget(parent)
{
  this->dbMgr = dbMgr;
  this->settings = settings;
  this->webService = webService;
  busy = false;
  firstLoad = true;

  statusTimer = new QTimer;
  statusTimer->setInterval(settings->getValue("refresh_locations_interval", REFRESH_LOCATIONS_INTERVAL).toInt());
  connect(statusTimer, SIGNAL(timeout()), this, SLOT(updateLocations()));

  btnRefreshLocations = new QPushButton(tr("Refresh"));
  connect(btnRefreshLocations, SIGNAL(clicked()), this, SLOT(updateLocationsClicked()));

  btnDelete = new QPushButton(tr("Delete Location(s)"));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(deleteLocationsClicked()));

  btnNewLocation = new QPushButton(tr("Add Location"));
  connect(btnNewLocation, SIGNAL(clicked()), this, SLOT(addLocationClicked()));

  connect(webService, SIGNAL(getLocationsResult(bool,QVariantMap)), this, SLOT(finishedDownloadingLocations(bool,QVariantMap)));
  connect(webService, SIGNAL(deleteLocationsResult(bool,QVariantMap)), this, SLOT(finishedDeletingLocations(bool,QVariantMap)));

  // location_id, name, public_ip_address, collections_mailing_list, service_mailing_list, bill_acceptor_mailing_list, allow_notifications, show_collections
  locationsModel = new QStandardItemModel(0, 8);
  locationsModel->setHorizontalHeaderItem(Location_ID, new QStandardItem(QString("Location ID")));
  locationsModel->setHorizontalHeaderItem(Name, new QStandardItem(QString("Name")));
  locationsModel->setHorizontalHeaderItem(Public_IP, new QStandardItem(QString("Public IP")));
  locationsModel->setHorizontalHeaderItem(Collections_Mailing_List, new QStandardItem(QString("Collection Recipients")));
  locationsModel->setHorizontalHeaderItem(Service_Mailing_List, new QStandardItem(QString("Service Recipients")));
  locationsModel->setHorizontalHeaderItem(Bill_Acceptor_Mailing_List, new QStandardItem(QString("Bill Acceptor Recipients")));
  locationsModel->setHorizontalHeaderItem(Allow_Notifications, new QStandardItem(QString("Allow Notifications")));
  locationsModel->setHorizontalHeaderItem(Show_Collections, new QStandardItem(QString("Show Collections")));

  sortFilter = new QSortFilterProxyModel;
  sortFilter->setSourceModel(locationsModel);
  sortFilter->setSortRole(Qt::UserRole + 1);

  lblLocations = new QLabel(tr("Locations"));
  locationsView = new QTableView;
  locationsView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  locationsView->horizontalHeader()->setStretchLastSection(true);
  locationsView->horizontalHeader()->setStyleSheet("font:bold Arial;");
  locationsView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  locationsView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  locationsView->setSelectionBehavior(QAbstractItemView::SelectRows);
  locationsView->setWordWrap(true);
  locationsView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  locationsView->verticalHeader()->hide();
  locationsView->setModel(sortFilter);
  locationsView->setAlternatingRowColors(true);
  locationsView->setSortingEnabled(true);
  locationsView->setColumnHidden(Location_ID, true);
  // Make table read-only
  locationsView->setEditTriggers(QAbstractItemView::NoEditTriggers);

  connect(locationsView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(locationDoubleClicked(QModelIndex)));

  buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(btnRefreshLocations);  
  buttonLayout->addWidget(btnNewLocation);
  buttonLayout->addWidget(btnDelete);
  buttonLayout->addStretch(1);

  verticalLayout = new QVBoxLayout;
  verticalLayout->addWidget(lblLocations);
  verticalLayout->addWidget(locationsView);
  verticalLayout->addLayout(buttonLayout);

  this->setLayout(verticalLayout);
}

LocationsWidget::~LocationsWidget()
{
  statusTimer->deleteLater();
}

bool LocationsWidget::isBusy()
{
  return busy;
}

void LocationsWidget::showEvent(QShowEvent *)
{
  QLOG_DEBUG() << QString("Showing Locations tab");

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
    QList<Location> locations = dbMgr->getLocations();

    foreach (Location l, locations)
    {
      insertRecord(l);
    }

    locationsView->sortByColumn(Name, Qt::AscendingOrder);
    locationsView->resizeColumnsToContents();

    firstLoad = false;
  }
}

void LocationsWidget::insertRecord(Location loc)
{
  // location_id, name, public_ip_address, collections_mailing_list, service_mailing_list, bill_acceptor_mailing_list, allow_notifications, show_collections
  QStandardItem *idField, *nameField, *publicIpField, *collectionsMailingListField, *serviceMailingListField,
                *billAcceptorMailingListField, *allowNotificationsField, *showCollectionsField;

  idField = new QStandardItem(QString("%1").arg(loc.locationID));
  idField->setData(loc.locationID);

  nameField = new QStandardItem(loc.name);
  nameField->setData(loc.name);

  publicIpField = new QStandardItem(loc.publicIP);
  publicIpField->setData(loc.publicIP);

  collectionsMailingListField = new QStandardItem(loc.collectionsMailingList);
  collectionsMailingListField->setData(loc.collectionsMailingList);

  serviceMailingListField = new QStandardItem(loc.serviceMailingList);
  serviceMailingListField->setData(loc.serviceMailingList);

  billAcceptorMailingListField = new QStandardItem(loc.billAcceptorMailingList);
  billAcceptorMailingListField->setData(loc.billAcceptorMailingList);

  allowNotificationsField = new QStandardItem(loc.allowNotifications ? "True" : "False");
  allowNotificationsField->setData(loc.allowNotifications);

  showCollectionsField = new QStandardItem(loc.showCollections ? "True" : "False");
  showCollectionsField->setData(loc.showCollections);

  int row = locationsModel->rowCount();

  locationsModel->setItem(row, Location_ID, idField);
  locationsModel->setItem(row, Name, nameField);
  locationsModel->setItem(row, Public_IP, publicIpField);
  locationsModel->setItem(row, Collections_Mailing_List, collectionsMailingListField);
  locationsModel->setItem(row, Service_Mailing_List, serviceMailingListField);
  locationsModel->setItem(row, Bill_Acceptor_Mailing_List, billAcceptorMailingListField);
  locationsModel->setItem(row, Allow_Notifications, allowNotificationsField);
  locationsModel->setItem(row, Show_Collections, showCollectionsField);
}

void LocationsWidget::populateTable()
{

}

void LocationsWidget::clearTable()
{
  if (locationsModel->rowCount() > 0)
  {
    locationsModel->removeRows(0, locationsModel->rowCount());
  }
}

void LocationsWidget::downloadingLocations()
{
}

void LocationsWidget::finishedDownloadingLocations(bool success, QVariantMap response)
{
  // If successful it means either the locations were downloaded or nothing has changed so we already have the current locations
  if (success)
  {
    if (response.empty())
      QLOG_DEBUG() << "We already have the current locations";
    else
    {
      // Replace alerts in datagrid with what we just downloaded
      clearTable();

      if (response.contains("data") && response.contains("location_hash"))
      {
        settings->setValue("location_hash", response["location_hash"].toLongLong());

        dbMgr->insertLocations(response["data"].toList());

        // location_id, name, public_ip_address, collections_mailing_list, service_mailing_list, bill_acceptor_mailing_list, allow_notifications, show_collections
        foreach (QVariant v, response["data"].toList())
        {
          QVariantMap location = v.toMap();

          Location l;
          l.locationID = location["location_id"].toInt();
          l.name = location["name"].toString();
          l.publicIP = location["public_ip_address"].toString();
          l.collectionsMailingList = location["collections_mailing_list"].toString();
          l.serviceMailingList = location["service_mailing_list"].toString();
          l.billAcceptorMailingList = location["bill_acceptor_mailing_list"].toString();
          l.allowNotifications = location["allow_notifications"].toInt() == 1;
          l.showCollections = location["show_collections"].toInt() == 1;

          insertRecord(l);
        }

        locationsView->sortByColumn(locationsView->horizontalHeader()->sortIndicatorSection(), locationsView->horizontalHeader()->sortIndicatorOrder());
        locationsView->resizeColumnsToContents();
        locationsView->scrollToTop();
      }
    }
  }

  this->setEnabled(true);
}

void LocationsWidget::updateLocations()
{
  QLOG_DEBUG() << "Time to check for location updates";
  updateLocationsClicked();
}

void LocationsWidget::updateLocationsClicked()
{
  this->setEnabled(false);
  webService->startGetLocations(settings->getValue("location_hash", LOCATION_HASH).toLongLong());
}

void LocationsWidget::addLocationClicked()
{
  EditLocationWidget editLocation(dbMgr, settings, webService);

  if (editLocation.exec() == QDialog::Accepted)
    updateLocationsClicked();
}

void LocationsWidget::locationDoubleClicked(const QModelIndex &index)
{
 // QStandardItem *idField, *nameField, *publicIpField, *collectionsMailingListField, *serviceMailingListField,
  //              *billAcceptorMailingListField, *allowNotificationsField, *showCollectionsField;

  Location location;

  location.locationID = sortFilter->index(index.row(), Location_ID).data().toInt();
  location.name = sortFilter->index(index.row(), Name).data().toString();
  location.publicIP = sortFilter->index(index.row(), Public_IP).data().toString();
  location.collectionsMailingList = sortFilter->index(index.row(), Collections_Mailing_List).data().toString();
  location.serviceMailingList = sortFilter->index(index.row(), Service_Mailing_List).data().toString();
  location.billAcceptorMailingList = sortFilter->index(index.row(), Bill_Acceptor_Mailing_List).data().toString();
  location.allowNotifications = sortFilter->index(index.row(), Allow_Notifications).data().toBool();
  location.showCollections = sortFilter->index(index.row(), Show_Collections).data().toBool();

  EditLocationWidget editLocation(dbMgr, settings, webService);
  editLocation.populateForm(location);

  if (editLocation.exec() == QDialog::Accepted)
    updateLocationsClicked();
}

void LocationsWidget::deleteLocationsClicked()
{
  QModelIndexList selectedRows = locationsView->selectionModel()->selectedRows(Location_ID);

  if (selectedRows.count() == 0)
  {
    QLOG_DEBUG() << "User pressed Delete key without any rows selected in locations table";
    QMessageBox::warning(this, tr("Delete Location(s)"), tr("Select one or more rows in the table above."));
  }
  else
  {
    QLOG_DEBUG() << "User selected" << selectedRows.count() << "rows to delete from the locations table";

    if (QMessageBox::question(this, tr("Delete Location(s)"), tr("Are you sure you want to delete the selected %1 location(s)?").arg(selectedRows.count()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      this->setEnabled(false);

      QLOG_DEBUG() << "User chose to delete location(s)";

      QStringList locationIdList;

      // Delete one or more rows from the model and database
      //qDebug() << selectedRows.count() << "rows selected";
      while (selectedRows.count() > 0)
      {
        QModelIndex index = selectedRows.at(0);

        locationIdList.append(index.data(Qt::UserRole + 1).value<QString>());

        sortFilter->removeRow(index.row());

        selectedRows = locationsView->selectionModel()->selectedRows(Location_ID);
      }

      webService->startDeleteLocations(locationIdList);
    }
    else
    {
      QLOG_DEBUG() << "User chose not to delete the location(s)";
    }
  }
}

void LocationsWidget::finishedDeletingLocations(bool success, QVariantMap response)
{
  //emit updateStatusMessage(tr("Finished deleting location(s)."));

  // If successful it means the selected location records were deleted from the database
  if (success)
  {
    if (response.contains("result"))
    {
      QMessageBox::information(this, tr("Delete Location(s)"), response["result"].toString());
    }
  }
  else
  {
    if (response.contains("error"))
    {
      QMessageBox::warning(this, tr("Delete Location(s)"), response["error"].toString());
    }
  }

  this->setEnabled(true);
}

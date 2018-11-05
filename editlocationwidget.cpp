#include "editlocationwidget.h"
#include "qslog/QsLog.h"
#include "global.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QHostAddress>

EditLocationWidget::EditLocationWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent) :  QDialog(parent)
{
  existingLocation = false;

  this->dbMgr = dbMgr;
  this->settings = settings;
  this->webService = webService;

  connect(this->webService, SIGNAL(addLocationResult(bool,QVariantMap)), this, SLOT(finishedAddingLocation(bool,QVariantMap)));
  connect(this->webService, SIGNAL(updateLocationResult(bool,QVariantMap)), this, SLOT(finishedUpdatingLocation(bool,QVariantMap)));

  lblLocationName = new QLabel(tr("Location Name"));
  lblPublicIpAddress = new QLabel(tr("Public IP Address"));
  lblCollectionRecipients = new QLabel(tr("Collection Recipients"));
  lblServiceRecipients = new QLabel(tr("Service Recipients"));
  lblBillAcceptorRecipients = new QLabel(tr("Bill Acceptor Recipients"));
  lblAllowNotifications = new QLabel(tr("Allow Notifications"));
  lblShowCollections = new QLabel(tr("Show Collection Reports"));

  txtLocationName = new QLineEdit;
  txtLocationName->setMaxLength(50);
  txtPublicIpAddress = new QLineEdit;
  txtPublicIpAddress->setMaxLength(15);
  txtCollectionRecipient = new QLineEdit;
  txtCollectionRecipient->setMaxLength(255);
  txtServiceRecipient = new QLineEdit;
  txtServiceRecipient->setMaxLength(255);
  txtBillAcceptorRecipient = new QLineEdit;
  txtBillAcceptorRecipient->setMaxLength(255);

  lstCollectionRecipients = new QListWidget;
  connect(lstCollectionRecipients, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(collectionRecipientDoubleClicked(QModelIndex)));

  lstServiceRecipients = new QListWidget;
  connect(lstServiceRecipients, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(serviceRecipientDoubleClicked(QModelIndex)));

  lstBillAcceptorRecipients = new QListWidget;
  connect(lstBillAcceptorRecipients, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(billAcceptorRecipientDoubleClicked(QModelIndex)));

  btnAddCollectionRecipient = new QPushButton(tr("Add"));
  btnAddCollectionRecipient->setDefault(false);
  btnAddCollectionRecipient->setAutoDefault(false);

  btnRemoveCollectionRecipient = new QPushButton(tr("Remove"));
  btnRemoveCollectionRecipient->setDefault(false);
  btnRemoveCollectionRecipient->setAutoDefault(false);

  connect(btnAddCollectionRecipient, SIGNAL(clicked()), this, SLOT(addCollectionRecipientClicked()));
  connect(btnRemoveCollectionRecipient, SIGNAL(clicked()), this, SLOT(removeCollectionRecipientClicked()));

  btnAddServiceRecipient = new QPushButton(tr("Add"));
  btnAddServiceRecipient->setDefault(false);
  btnAddServiceRecipient->setAutoDefault(false);

  btnRemoveServiceRecipient = new QPushButton(tr("Remove"));
  btnRemoveServiceRecipient->setDefault(false);
  btnRemoveServiceRecipient->setAutoDefault(false);

  connect(btnAddServiceRecipient, SIGNAL(clicked()), this, SLOT(addServiceRecipientClicked()));
  connect(btnRemoveServiceRecipient, SIGNAL(clicked()), this, SLOT(removeServiceRecipientClicked()));

  btnAddBillAcceptorRecipient = new QPushButton(tr("Add"));
  btnAddBillAcceptorRecipient->setDefault(false);
  btnAddBillAcceptorRecipient->setAutoDefault(false);

  btnRemoveBillAcceptorRecipient = new QPushButton(tr("Remove"));
  btnRemoveBillAcceptorRecipient->setDefault(false);
  btnRemoveBillAcceptorRecipient->setAutoDefault(false);

  connect(btnAddBillAcceptorRecipient, SIGNAL(clicked()), this, SLOT(addBillAcceptorRecipientClicked()));
  connect(btnRemoveBillAcceptorRecipient, SIGNAL(clicked()), this, SLOT(removeBillAcceptorRecipientClicked()));

  chkAllowNotifications = new QCheckBox;
  chkAllowNotifications->setChecked(false);
  chkShowCollections = new QCheckBox;
  chkShowCollections->setChecked(false);

  btnSave = new QPushButton(tr("Save"));
  btnCancel = new QPushButton(tr("Cancel"));
  connect(btnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));


  buttonLayout = new QHBoxLayout;
  collectionRecipientButtonLayout = new QHBoxLayout;
  serviceRecipientButtonLayout = new QHBoxLayout;
  billAcceptorButtonLayout = new QHBoxLayout;

  collectionRecipientButtonLayout->addWidget(btnAddCollectionRecipient);
  collectionRecipientButtonLayout->addWidget(btnRemoveCollectionRecipient);
  serviceRecipientButtonLayout->addWidget(btnAddServiceRecipient);
  serviceRecipientButtonLayout->addWidget(btnRemoveServiceRecipient);
  billAcceptorButtonLayout->addWidget(btnAddBillAcceptorRecipient);
  billAcceptorButtonLayout->addWidget(btnRemoveBillAcceptorRecipient);

  gridLayout = new QGridLayout;
  gridLayout->addWidget(lblLocationName, 0, 0);
  gridLayout->addWidget(txtLocationName, 0, 1, 1, 2);

  gridLayout->addWidget(lblPublicIpAddress, 1, 0);
  gridLayout->addWidget(txtPublicIpAddress, 1, 1, 1, 2);

  gridLayout->addWidget(lblCollectionRecipients, 2, 0);
  gridLayout->addWidget(txtCollectionRecipient, 2, 1, 1, 2);
  gridLayout->addLayout(collectionRecipientButtonLayout, 3, 1, 1, 2);
  gridLayout->addWidget(lstCollectionRecipients, 4, 1, 1, 2);

  gridLayout->addWidget(lblServiceRecipients, 5, 0);
  gridLayout->addWidget(txtServiceRecipient, 5, 1, 1, 2);
  gridLayout->addLayout(serviceRecipientButtonLayout, 6, 1, 1, 2);
  gridLayout->addWidget(lstServiceRecipients, 7, 1, 1, 2);

  gridLayout->addWidget(lblBillAcceptorRecipients, 8, 0);
  gridLayout->addWidget(txtBillAcceptorRecipient, 8, 1, 1, 2);
  gridLayout->addLayout(billAcceptorButtonLayout, 9, 1, 1, 2);
  gridLayout->addWidget(lstBillAcceptorRecipients, 10, 1, 1, 2);

  gridLayout->addWidget(lblAllowNotifications, 11, 0);
  gridLayout->addWidget(chkAllowNotifications, 11, 1, 1, 2);

  gridLayout->addWidget(lblShowCollections, 12, 0);
  gridLayout->addWidget(chkShowCollections, 12, 1, 1, 2);

  buttonLayout->addWidget(btnSave);
  buttonLayout->addWidget(btnCancel);
  //gridLayout->setRowStretch(13, 3);
  gridLayout->addLayout(buttonLayout, 13, 0, 1, 4);

  this->setLayout(gridLayout);

  this->setWindowTitle(tr("Add Location"));
}

EditLocationWidget::~EditLocationWidget()
{
  //gridLayout->deleteLater();
}

void EditLocationWidget::populateForm(Location location)
{
  this->setWindowTitle(tr("Edit Location"));

  this->location = location;
  existingLocation = true;

  txtLocationName->setText(location.name);
  txtPublicIpAddress->setText(location.publicIP);

  lstCollectionRecipients->addItems(location.collectionsMailingList.split(",", QString::SkipEmptyParts));
  lstServiceRecipients->addItems(location.serviceMailingList.split(",", QString::SkipEmptyParts));
  lstBillAcceptorRecipients->addItems(location.billAcceptorMailingList.split(",", QString::SkipEmptyParts));

  chkAllowNotifications->setChecked(location.allowNotifications);
  chkShowCollections->setChecked(location.showCollections);
}

Location EditLocationWidget::getLocation()
{
  return location;
}

bool EditLocationWidget::locationChanged()
{
  return true;
}

void EditLocationWidget::showEvent(QShowEvent *)
{
  txtLocationName->setFocus();
  txtLocationName->selectAll();

  // Disable maximize,minimize and resizing of window
  this->setFixedSize(this->sizeHint().width(), 650);
}

void EditLocationWidget::accept()
{
  QString errors = isValidInput();

  if (errors.isEmpty())
  {
    if (existingLocation)
    {
      bool recipientsChanged = false;

      QStringList originalList = location.collectionsMailingList.split(",", QString::SkipEmptyParts);
      QStringList newList = listWidgetToStringList(lstCollectionRecipients);

      // All email addresses are converted to lowercase when entered with the form
      // so all addresses are expected to be lowercase when sorting
      qSort(originalList);
      qSort(newList);

      if (originalList != newList)
        recipientsChanged = true;

      if (!recipientsChanged)
      {
        originalList = location.serviceMailingList.split(",", QString::SkipEmptyParts);
        newList = listWidgetToStringList(lstServiceRecipients);

        qSort(originalList);
        qSort(newList);

        if (originalList != newList)
          recipientsChanged = true;
      }

      if (!recipientsChanged)
      {
        originalList = location.billAcceptorMailingList.split(",", QString::SkipEmptyParts);
        newList = listWidgetToStringList(lstBillAcceptorRecipients);

        qSort(originalList);
        qSort(newList);

        if (originalList != newList)
          recipientsChanged = true;
      }

      // When editing existing location we need to see if anything was changed
      // if so then send the data, otherwise do not contact web service
      if (location.name != txtLocationName->text() ||
          location.publicIP != txtPublicIpAddress->text() ||
          location.allowNotifications != chkAllowNotifications->isChecked() ||
          location.showCollections != chkShowCollections->isChecked() ||
          recipientsChanged)
      {
        location.name = txtLocationName->text();
        location.publicIP = txtPublicIpAddress->text();
        location.collectionsMailingList = listWidgetToStringList(lstCollectionRecipients).join(",");
        location.serviceMailingList = listWidgetToStringList(lstServiceRecipients).join(",");
        location.billAcceptorMailingList = listWidgetToStringList(lstBillAcceptorRecipients).join(",");
        location.allowNotifications = chkAllowNotifications->isChecked();
        location.showCollections = chkShowCollections->isChecked();

        webService->startUpdateLocation(location);
      }
      else
      {
        QLOG_DEBUG() << "User did not make any changes to location";
        QDialog::accept();
      }
    }
    else
    {
      // Adding new location

      location.locationID = 0;
      location.name = txtLocationName->text();
      location.publicIP = txtPublicIpAddress->text();
      location.collectionsMailingList = listWidgetToStringList(lstCollectionRecipients).join(",");
      location.serviceMailingList = listWidgetToStringList(lstServiceRecipients).join(",");
      location.billAcceptorMailingList = listWidgetToStringList(lstBillAcceptorRecipients).join(",");
      location.allowNotifications = chkAllowNotifications->isChecked();
      location.showCollections = chkShowCollections->isChecked();

      webService->startAddLocation(location);
    }
  }
  else
  {
    QLOG_DEBUG() << QString("User tried to submit incomplete location. Error: %1").arg(errors);
    QMessageBox::warning(this, tr("Save"), tr("Correct the following problems:\n%1").arg(errors));
  }
}

void EditLocationWidget::reject()
{
  QDialog::reject();
}

void EditLocationWidget::finishedAddingLocation(bool success, QVariantMap response)
{
  // If successful it means the location was added
  if (success)
  {
    if (response.contains("result"))
    {
      QMessageBox::information(this, tr("Add Location"), response["result"].toString());

      QDialog::accept();
    }
  }
  else
  {
    if (response.contains("error"))
    {
      QMessageBox::warning(this, tr("Add Location"), response["error"].toString());
    }
  }
}

void EditLocationWidget::finishedUpdatingLocation(bool success, QVariantMap response)
{
  // If successful it means the location was added
  if (success)
  {
    if (response.contains("result"))
    {
      QMessageBox::information(this, tr("Update Location"), response["result"].toString());

      QDialog::accept();
    }
  }
  else
  {
    if (response.contains("error"))
    {
      QMessageBox::warning(this, tr("Update Location"), response["error"].toString());
    }
  }
}

void EditLocationWidget::addCollectionRecipientClicked()
{
  QString errorMessage = "";
  txtCollectionRecipient->setText(txtCollectionRecipient->text().trimmed());

  if (Global::isValidEmail(txtCollectionRecipient->text()))
  {
    // If the email address doesn't exist in list already
    if (lstCollectionRecipients->findItems(txtCollectionRecipient->text(), Qt::MatchFixedString).count() > 0)
    {
      errorMessage = "The email address is already in the list.";
    }
    else
    {
      lstCollectionRecipients->addItem(txtCollectionRecipient->text().toLower());
      txtCollectionRecipient->clear();
    }
  }
  else
    errorMessage = "Enter a valid email address.";

  if (errorMessage.length() > 0)
    QMessageBox::warning(this, "Add Collection Recipient", errorMessage);

  txtCollectionRecipient->setFocus();
  txtCollectionRecipient->selectAll();
}

void EditLocationWidget::removeCollectionRecipientClicked()
{
  if (lstCollectionRecipients->currentRow() >= 0)
  {
    QListWidgetItem *item = lstCollectionRecipients->takeItem(lstCollectionRecipients->currentRow());

    delete item;
  }
  else
  {
    QMessageBox::information(this, "Remove Collection Recipient", "Select an email address to remove from the list.");
  }
}

void EditLocationWidget::addServiceRecipientClicked()
{
  QString errorMessage = "";
  txtServiceRecipient->setText(txtServiceRecipient->text().trimmed());

  if (Global::isValidEmail(txtServiceRecipient->text()))
  {
    // If the email address doesn't exist in list already
    if (lstServiceRecipients->findItems(txtServiceRecipient->text(), Qt::MatchFixedString).count() > 0)
    {
      errorMessage = "The email address is already in the list.";
    }
    else
    {
      lstServiceRecipients->addItem(txtServiceRecipient->text().toLower());
      txtServiceRecipient->clear();
    }
  }
  else
    errorMessage = "Enter a valid email address.";

  if (errorMessage.length() > 0)
    QMessageBox::warning(this, "Add Service Recipient", errorMessage);

  txtServiceRecipient->setFocus();
  txtServiceRecipient->selectAll();
}

void EditLocationWidget::removeServiceRecipientClicked()
{
  if (lstServiceRecipients->currentRow() >= 0)
  {
    QListWidgetItem *item = lstServiceRecipients->takeItem(lstServiceRecipients->currentRow());

    delete item;
  }
  else
  {
    QMessageBox::information(this, "Remove Service Recipient", "Select an email address to remove from the list.");
  }
}

void EditLocationWidget::addBillAcceptorRecipientClicked()
{
  QString errorMessage = "";
  txtBillAcceptorRecipient->setText(txtBillAcceptorRecipient->text().trimmed());

  if (Global::isValidEmail(txtBillAcceptorRecipient->text()))
  {
    // If the email address doesn't exist in list already
    if (lstBillAcceptorRecipients->findItems(txtBillAcceptorRecipient->text(), Qt::MatchFixedString).count() > 0)
    {
      errorMessage = "The email address is already in the list.";
    }
    else
    {
      lstBillAcceptorRecipients->addItem(txtBillAcceptorRecipient->text().toLower());
      txtBillAcceptorRecipient->clear();
    }
  }
  else
    errorMessage = "Enter a valid email address.";

  if (errorMessage.length() > 0)
    QMessageBox::warning(this, "Add Bill Acceptor Recipient", errorMessage);

  txtBillAcceptorRecipient->setFocus();
  txtBillAcceptorRecipient->selectAll();
}

void EditLocationWidget::removeBillAcceptorRecipientClicked()
{
  if (lstBillAcceptorRecipients->currentRow() >= 0)
  {
    QListWidgetItem *item = lstBillAcceptorRecipients->takeItem(lstBillAcceptorRecipients->currentRow());

    delete item;
  }
  else
  {
    QMessageBox::information(this, "Remove Bill Acceptor Recipient", "Select an email address to remove from the list.");
  }
}

void EditLocationWidget::collectionRecipientDoubleClicked(const QModelIndex &index)
{
  QListWidgetItem *item = lstCollectionRecipients->item(index.row());
  txtCollectionRecipient->setText(item->text());
}

void EditLocationWidget::serviceRecipientDoubleClicked(const QModelIndex &index)
{
  QListWidgetItem *item = lstServiceRecipients->item(index.row());
  txtServiceRecipient->setText(item->text());
}

void EditLocationWidget::billAcceptorRecipientDoubleClicked(const QModelIndex &index)
{
  QListWidgetItem *item = lstBillAcceptorRecipients->item(index.row());
  txtBillAcceptorRecipient->setText(item->text());
}

QString EditLocationWidget::isValidInput()
{
  QStringList errorList;

  txtLocationName->setText(txtLocationName->text().trimmed());
  txtPublicIpAddress->setText(txtPublicIpAddress->text().trimmed());

  if (txtLocationName->text().isEmpty())
    errorList.append("- Location Name cannot be empty.");
  else if (Global::containsExtendedChars(txtLocationName->text()))
    errorList.append("- Location Name cannot contain extended characters. If you cannot see any of these characters, try to clear the field and type it again.");
  else if (txtLocationName->text().contains("\""))
    errorList.append("- Location Name cannot contain the double-quote (\") character.");

  if (txtPublicIpAddress->text().isEmpty())
    errorList.append("- Public IP Address cannot be empty.");
  else
  {
    QHostAddress testIP;
    if (!testIP.setAddress(txtPublicIpAddress->text()))
      errorList.append("- Public IP Address must be a valid IPv4 address.");
  }

  // The various email recipients are all optional

  // Don't allow user to leave a valid email address in any of the recipient entry fields so there's
  // no confusion
  if (!txtCollectionRecipient->text().isEmpty() && Global::isValidEmail(txtCollectionRecipient->text()))
  {
    errorList.append("- If you intended for the email address in the Collection Recipient field to be used then click the Add button below the field to add it to the list. Otherwise clear the field.");
  }

  if (!txtServiceRecipient->text().isEmpty() && Global::isValidEmail(txtServiceRecipient->text()))
  {
    errorList.append("- If you intended for the email address in the Service Recipient field to be used then click the Add button below the field to add it to the list. Otherwise clear the field.");
  }

  if (!txtBillAcceptorRecipient->text().isEmpty() && Global::isValidEmail(txtBillAcceptorRecipient->text()))
  {
    errorList.append("- If you intended for the email address in the Bill Acceptor Recipient field to be used then click the Add button below the field to add it to the list. Otherwise clear the field.");
  }

  return errorList.join("\n");
}

QStringList EditLocationWidget::listWidgetToStringList(QListWidget *listWidget)
{
  QStringList items;

  for (int i = 0; i < listWidget->count(); ++i)
    items.append(listWidget->item(i)->text());

  return items;
}

bool EditLocationWidget::dataChanged()
{
  bool changed = false;

  return changed;
}

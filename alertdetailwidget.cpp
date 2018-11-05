#include "alertdetailwidget.h"
#include <QCalendarWidget>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QHeaderView>

AlertDetailWidget::AlertDetailWidget(QWidget *parent) : QDialog(parent)
{  
  txtAlertID = new QLineEdit;
  txtAlertID->setReadOnly(true);

  txtDateTime = new QLineEdit;
  txtDateTime->setReadOnly(true);

  txtLocation = new QLineEdit;
  txtLocation->setReadOnly(true);

  txtPublicIP = new QLineEdit;
  txtPublicIP->setReadOnly(true);

  txtLocalIP = new QLineEdit;
  txtLocalIP->setReadOnly(true);

  txtHostname = new QLineEdit;
  txtHostname->setReadOnly(true);

  txtUptime = new QLineEdit;
  txtUptime->setReadOnly(true);

  txtSoftware = new QLineEdit;
  txtSoftware->setReadOnly(true);

  txtVersion = new QLineEdit;
  txtVersion->setReadOnly(true);

  txtMessage = new QTextEdit;
  txtMessage->setReadOnly(true);

  txtSent = new QLineEdit;
  txtSent->setReadOnly(true);

  btnClose = new QPushButton(tr("Close"));
  connect(btnClose, SIGNAL(clicked()), this, SLOT(accept()));

  //btnCancel = new QPushButton(tr("Cancel"));
  //connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

  buttonLayout = new QDialogButtonBox;
  buttonLayout->addButton(btnClose, QDialogButtonBox::AcceptRole);
 // buttonLayout->addButton(btnCancel, QDialogButtonBox::RejectRole);

  formLayout = new QFormLayout;
  // This size policy for the fields will make the cmbMovieSets stretch the full width of window
  //formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  formLayout->addRow(tr("Alert ID"), txtAlertID);
  formLayout->addRow(tr("Date/Time"), txtDateTime);
  formLayout->addRow(tr("Location"), txtLocation);
  formLayout->addRow(tr("Public IP"), txtPublicIP);
  formLayout->addRow(tr("Local IP"), txtLocalIP);
  formLayout->addRow(tr("Hostname"), txtHostname);
  formLayout->addRow(tr("Uptime"), txtUptime);
  formLayout->addRow(tr("Software"), txtSoftware);
  formLayout->addRow(tr("Version"), txtVersion);
  formLayout->addRow(tr("Message"), txtMessage);
  formLayout->addRow(tr("Alert Sent"), txtSent);


  verticalLayout = new QVBoxLayout;
  verticalLayout->addLayout(formLayout);
  verticalLayout->addWidget(buttonLayout);

  this->setWindowTitle(tr("Alert Details"));
  this->setLayout(verticalLayout);  

  // Disable maximize,minimize and resizing of window
  this->setFixedSize(this->sizeHint());

  // Using a fixed width
  this->setFixedWidth(600);
}

AlertDetailWidget::~AlertDetailWidget()
{
  // gridLayout->deleteLater();
}

void AlertDetailWidget::populateForm(Alert alert)
{
  txtAlertID->setText(QString("%1").arg(alert.alertID));
  txtDateTime->setText(QDateTime::fromTime_t(alert.dateTime).toString("MM/dd/yyyy h:mm:ss ap"));
  txtLocation->setText(alert.location);
  txtPublicIP->setText(alert.publicIP);
  txtLocalIP->setText(alert.localIP);
  txtHostname->setText(alert.hostname);
  txtUptime->setText(alert.uptime);
  txtSoftware->setText(alert.software);
  txtVersion->setText(alert.version);
  txtMessage->setText(alert.message);
  txtSent->setText(alert.sent ? "True" : "False");
}

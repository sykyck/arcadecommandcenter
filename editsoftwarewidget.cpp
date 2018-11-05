#include "editsoftwarewidget.h"
#include "qslog/QsLog.h"
#include "global.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>

EditSoftwareWidget::EditSoftwareWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent) :  QDialog(parent)
{  
  // Disable maximize,minimize and resizing of window
  this->setFixedSize(500, 250);

  existingSoftware = false;

  this->dbMgr = dbMgr;
  this->settings = settings;
  this->webService = webService;

  connect(this->webService, SIGNAL(addSoftwareResult(bool,QVariantMap)), this, SLOT(finishedAddingSoftware(bool,QVariantMap)));
  connect(this->webService, SIGNAL(updateSoftwareResult(bool,QVariantMap)), this, SLOT(finishedUpdatingSoftware(bool,QVariantMap)));

  lblSoftwareName = new QLabel(tr("Software Name"));
  lblVersion = new QLabel(tr("Version"));
  lblPackage = new QLabel(tr("Installer"));
  lblCurrentPackage = new QLabel(tr("Current Installer"));
  lblCurrentPackageURL = new QLabel;
  lblCurrentPackageURL->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
  lblCurrentPackageURL->setOpenExternalLinks(true);

  txtSoftwareName = new QLineEdit;
  txtSoftwareName->setMaxLength(254);
  txtVersion = new QLineEdit;
  txtVersion->setMaxLength(24);
  txtPackage = new QLineEdit;
  txtPackage->setMaxLength(254);

  btnSave = new QPushButton(tr("Save"));
  btnCancel = new QPushButton(tr("Cancel"));
  connect(btnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

  btnChoosePackage = new QToolButton;
  btnChoosePackage->setToolButtonStyle(Qt::ToolButtonTextOnly);
  btnChoosePackage->setText("...");
  connect(btnChoosePackage, SIGNAL(clicked()), this, SLOT(choosePackage()));

  buttonLayout = new QHBoxLayout;  

  gridLayout = new QGridLayout;
  gridLayout->addWidget(lblSoftwareName, 0, 0);
  gridLayout->addWidget(txtSoftwareName, 0, 1, 1, 2);

  gridLayout->addWidget(lblVersion, 1, 0);
  gridLayout->addWidget(txtVersion, 1, 1, 1, 2);

  gridLayout->addWidget(lblPackage, 2, 0);
  gridLayout->addWidget(txtPackage, 2, 1, 1, 2);
  gridLayout->addWidget(btnChoosePackage, 2, 3);

  buttonLayout->addWidget(btnSave);
  buttonLayout->addWidget(btnCancel);
  gridLayout->setRowStretch(4, 3);
  gridLayout->addLayout(buttonLayout, 5, 0, 1, 4);

  this->setLayout(gridLayout);

  this->setWindowTitle(tr("Add Software"));
}

EditSoftwareWidget::~EditSoftwareWidget()
{
  //gridLayout->deleteLater();
}

void EditSoftwareWidget::populateForm(Software software)
{
  this->setWindowTitle(tr("Edit Software"));

  this->software = software;
  existingSoftware = true;

  txtSoftwareName->setText(software.name);
  txtVersion->setText(software.version);
  lblCurrentPackageURL->setText(QString("<a href=\"%1\">%2</a>").arg(software.installerURL).arg(software.installerURL));

  if (!lblCurrentPackage->parent())
  {
    gridLayout->addWidget(lblCurrentPackage, 3, 0);
    gridLayout->addWidget(lblCurrentPackageURL, 3, 1, 1, 3);
  }
}

Software EditSoftwareWidget::getSoftware()
{
  return software;
}

bool EditSoftwareWidget::softwareChanged()
{
  return true;
}

void EditSoftwareWidget::showEvent(QShowEvent *)
{
  txtSoftwareName->setFocus();
  txtSoftwareName->selectAll();
}

void EditSoftwareWidget::accept()
{
  QString errors = isValidInput();

  if (errors.isEmpty())
  {
    if (existingSoftware)
    {
      // When editing existing software we need to see if anything was changed
      // if so then send the data, otherwise do not contact web service
      if (software.name != txtSoftwareName->text() ||
          software.version != txtVersion->text() ||
          !txtPackage->text().isEmpty())
      {
        software.name = txtSoftwareName->text();
        software.version = txtVersion->text();
        software.installerURL = txtPackage->text();

        webService->startUpdateSoftware(software);
      }
      else
      {
        QLOG_DEBUG() << "User did not make any changes to software";
        QDialog::accept();
      }
    }
    else
    {
      software.name = txtSoftwareName->text();
      software.version = txtVersion->text();
      software.installerURL = txtPackage->text();

      webService->startAddSoftware(software);
    }
  }
  else
  {
    QLOG_DEBUG() << QString("User tried to submit incomplete software. Error: %1").arg(errors);
    QMessageBox::warning(this, tr("Save"), tr("Correct the following problems:\n%1").arg(errors));
  }
}

void EditSoftwareWidget::reject()
{
  QDialog::reject();
}

void EditSoftwareWidget::choosePackage()
{
 // QSettings settings("IA", SOFTWARE_NAME);
  //QString videoCoverPath = settings.value("video_cover_path", QDir::homePath()).toString();

  QString fileName = QFileDialog::getOpenFileName(this, "Select Installer", "", "BIN (*.bin *.exe)");

  if (!fileName.isEmpty())
  {
    txtPackage->setText(fileName);

    // Update the video cover path so we return to the same directory next time
   // QFileInfo file(fileName);
   // settings.setValue("video_cover_path", file.absolutePath());
  }
}

void EditSoftwareWidget::finishedAddingSoftware(bool success, QVariantMap response)
{
  // If successful it means the software was added
  if (success)
  {
    if (response.contains("result"))
    {
      QMessageBox::information(this, tr("Add Software"), response["result"].toString());

      QDialog::accept();
    }
  }
  else
  {
    if (response.contains("error"))
    {
      QMessageBox::warning(this, tr("Add Software"), response["error"].toString());
    }
  }
}

void EditSoftwareWidget::finishedUpdatingSoftware(bool success, QVariantMap response)
{
  // If successful it means the software was added
  if (success)
  {
    if (response.contains("result"))
    {
      QMessageBox::information(this, tr("Update Software"), response["result"].toString());

      QDialog::accept();
    }
  }
  else
  {
    if (response.contains("error"))
    {
      QMessageBox::warning(this, tr("Update Software"), response["error"].toString());
    }
  }
}

QString EditSoftwareWidget::isValidInput()
{
  QStringList errorList;

  txtSoftwareName->setText(txtSoftwareName->text().trimmed());
  txtVersion->setText(txtVersion->text().trimmed());
  txtPackage->setText(txtPackage->text().trimmed());

  if (txtSoftwareName->text().isEmpty())
    errorList.append("- Software Name cannot be empty.");
  else if (Global::containsExtendedChars(txtSoftwareName->text()))
    errorList.append("- Software Name cannot contain extended characters. If you cannot see any of these characters, try to clear the field and type it again.");
  else if (txtSoftwareName->text().contains("\""))
    errorList.append("- Software Name cannot contain the double-quote (\") character.");

  if (txtVersion->text().isEmpty())
    errorList.append("- Version cannot be empty.");
  else if (Global::containsExtendedChars(txtVersion->text()))
    errorList.append("- Version cannot contain extended characters. If you cannot see any of these characters, try to clear the field and type it again.");
  else if (txtVersion->text().contains("\""))
    errorList.append("- Version cannot contain the double-quote (\") character.");

  // If editing existing software then the package field is optional because the user might just be changing the software name
  if (txtPackage->text().isEmpty())
  {
    if (!existingSoftware)
      errorList.append("- Package cannot be empty.");
  }
  else if (Global::containsExtendedChars(txtPackage->text()))
    errorList.append("- Package cannot contain extended characters. If you cannot see any of these characters, try to clear the field and type it again.");
  else if (txtPackage->text().contains("\""))
    errorList.append("- Package cannot contain the double-quote (\") character.");
  else if (!QFile::exists(txtPackage->text()))
    errorList.append("- Package file does not exist.");

  return errorList.join("\n");
}

bool EditSoftwareWidget::dataChanged()
{
  bool changed = false;

  return changed;
}

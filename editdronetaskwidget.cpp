#include "editdronetaskwidget.h"
#include "qslog/QsLog.h"
#include "global.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QDebug>

EditDroneTaskWidget::EditDroneTaskWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent) :  QDialog(parent)
{  
  // Disable maximize,minimize and resizing of window
  this->setFixedSize(550, 400);

  this->dbMgr = dbMgr;
  this->settings = settings;
  this->webService = webService;
  connect(this->webService, SIGNAL(addDroneTaskResult(bool,QVariantMap)), this, SLOT(finishedAddingDroneTask(bool,QVariantMap)));

  lblTaskName = new QLabel(tr("Task Name"));
  lblPackage = new QLabel(tr("Package"));
  lblExpiration = new QLabel(tr("Expires"));
  lblRuleBuilder = new QLabel(tr("Rule Builder"));

  txtTaskName = new QLineEdit;
  txtTaskName->setMaxLength(254);
  txtPackage = new QLineEdit;
  txtPackage->setMaxLength(254);

  QDateTime defaultDate =  QDateTime::currentDateTime().addDays(1);

  // Set the seconds of the current time to zero
  defaultDate.setTime(QTime(defaultDate.time().hour(), defaultDate.time().minute()));

  dtExpiration = new QDateTimeEdit(defaultDate);
  dtExpiration->setDateTimeRange(QDateTime::currentDateTime(), QDateTime::currentDateTime().addYears(10));
  dtExpiration->setDisplayFormat("M/d/yyyy h:mm AP"); // TODO: Show time zone of computer
  dtExpiration->setCalendarPopup(true);
  dtExpiration->calendarWidget()->setGridVisible(true);
  dtExpiration->calendarWidget()->setFirstDayOfWeek(Qt::Monday);

  chkEnableExpiration = new QCheckBox;
  connect(chkEnableExpiration, SIGNAL(clicked(bool)), dtExpiration, SLOT(setEnabled(bool)));

  cmbOperands = new QComboBox;
  cmbOperands->setMaxVisibleItems(15);
  cmbOperands->setInsertPolicy(QComboBox::NoInsert);
  cmbOperands->addItem("");
  cmbOperands->addItem("Drone", "uuid");
  cmbOperands->addItem("Public IP", "public_ip_address");
  cmbOperands->addItem("Local IP", "local_ip_address");
  cmbOperands->addItem("Hostname", "hostname");
  cmbOperands->addItem("Machine Type", "machine_type");
  cmbOperands->addItem("Operating System", "os");
  connect(cmbOperands, SIGNAL(currentIndexChanged(int)), this, SLOT(operandChanged()));

  cmbOperators = new QComboBox;
  cmbOperators->setMaxVisibleItems(15);
  cmbOperators->addItem("");
  cmbOperators->addItem("=", "=");
  cmbOperators->addItem("!=", "!=");
  cmbOperators->addItem("Begins With", "begins_with");
  cmbOperators->addItem("Contains", "contains");
  cmbOperators->addItem("Ends With", "ends_with");

  cmbValues = new QComboBox;
  cmbValues->setMaxVisibleItems(15);
  cmbValues->setEditable(true);
  cmbValues->setInsertPolicy(QComboBox::InsertAlphabetically);

  ruleValues = dbMgr->getRuleValues();

  btnAddRule = new QPushButton(tr("Add Rule"));
  btnRemoveRule = new QPushButton(tr("Remove Rule"));
  lstRules = new QListWidget;

  connect(btnAddRule, SIGNAL(clicked()), this, SLOT(addRuleClicked()));
  connect(btnRemoveRule, SIGNAL(clicked()), this, SLOT(removeRuleClicked()));

  btnSave = new QPushButton(tr("Save"));
  btnCancel = new QPushButton(tr("Cancel"));
  connect(btnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

  btnChoosePackage = new QToolButton;
  btnChoosePackage->setToolButtonStyle(Qt::ToolButtonTextOnly);
  btnChoosePackage->setText("...");
  connect(btnChoosePackage, SIGNAL(clicked()), this, SLOT(choosePackage()));

  buttonLayout = new QHBoxLayout;  
  expirationLayout = new QHBoxLayout;

  gridLayout = new QGridLayout;
  gridLayout->addWidget(lblTaskName, 0, 0);
  gridLayout->addWidget(txtTaskName, 0, 1, 1, 3);

  gridLayout->addWidget(lblPackage, 1, 0);
  gridLayout->addWidget(txtPackage, 1, 1, 1, 3);
  gridLayout->addWidget(btnChoosePackage, 1, 4);

  expirationLayout->addWidget(chkEnableExpiration, 1);
  expirationLayout->addWidget(dtExpiration, 4);
  expirationLayout->addStretch(10);

  gridLayout->addWidget(lblExpiration, 2, 0);
  gridLayout->addLayout(expirationLayout, 2, 1, 1, 3);

  gridLayout->addWidget(lblRuleBuilder, 3, 0, 1, 4);

  gridLayout->addWidget(cmbOperands, 4, 0);
  gridLayout->addWidget(cmbOperators, 4, 1);
  gridLayout->addWidget(cmbValues, 4, 2, 1, 2);
  gridLayout->addWidget(btnAddRule, 4, 4);

  gridLayout->addWidget(lstRules, 5, 0, 3, 5);

  gridLayout->addWidget(btnRemoveRule, 8, 4);

 // buttonLayout->addStretch(3);
  buttonLayout->addWidget(btnSave);
  buttonLayout->addWidget(btnCancel);
 // buttonLayout->addStretch(3);
  gridLayout->addLayout(buttonLayout, 10, 0, 1, 5);

  this->setLayout(gridLayout);

  dtExpiration->setEnabled(chkEnableExpiration->isChecked());

  this->setWindowTitle(tr("Drone Task"));
}

EditDroneTaskWidget::~EditDroneTaskWidget()
{
  //gridLayout->deleteLater();
}

void EditDroneTaskWidget::finishedAddingDroneTask(bool success, QVariantMap response)
{
  // If successful it means the software was added
  if (success)
  {
    if (response.contains("result"))
    {
      QMessageBox::information(this, tr("Add Task"), response["result"].toString());

      QDialog::accept();
    }
  }
  else
  {
    if (response.contains("error"))
    {
      QMessageBox::warning(this, tr("Add Task"), response["error"].toString());
    }
  }
}

void EditDroneTaskWidget::populateForm(DroneTask droneTask)
{
  this->droneTask = droneTask;

  //QLOG_DEBUG() << QString("movie = %1, frontCoverFile = %2, backCoverFile = %3").arg(movie.toString()).arg(frontCoverFile).arg(backCoverFile);
/*
  metadataChanged = false;

  this->videoPath = droneTask;
  this->movie = movie;
  this->frontCoverFile = frontCoverFile;
  this->backCoverFile = backCoverFile;

  txtTaskName->setText(movie.UPC());
  txtPackage->setText(movie.Title());

  cmbOperands->setCurrentIndex(0);
  if (!movie.Producer().isEmpty())
  {
    int index = cmbOperands->findText(movie.Producer());
    if (index > -1)
      cmbOperands->setCurrentIndex(index);
    else
    {
      // Item not in the list so add it and select it
      cmbOperands->insertItem(cmbOperands->count(), movie.Producer());
      cmbOperands->setCurrentIndex(cmbOperands->count() - 1);
    }
  }

  operator->setCurrentIndex(0);
  if (!movie.Category().isEmpty())
  {
    int index = operator->findText(movie.Category());
    if (index > -1)
      operator->setCurrentIndex(index);
  }

  cmbValues->setCurrentIndex(0);
  if (!movie.Subcategory().isEmpty())
  {
    int index = cmbValues->findText(movie.Subcategory());
    if (index > -1)
      cmbValues->setCurrentIndex(index);
    else
    {
      // Item not in the list so add it and select it
      cmbValues->insertItem(cmbValues->count(), movie.Subcategory());
      cmbValues->setCurrentIndex(cmbValues->count() - 1);
    }
  }

  if (!frontCoverFile.isEmpty())
    txtFrontCover->setText(frontCoverFile);
  else
    txtFrontCover->clear();

  if (!backCoverFile.isEmpty())
    txtBackCover->setText(backCoverFile);
  else
    txtBackCover->clear();*/
}

DroneTask EditDroneTaskWidget::getDroneTask()
{
  return droneTask;
}

bool EditDroneTaskWidget::droneTaskChanged()
{
  return true;
}

void EditDroneTaskWidget::showEvent(QShowEvent *)
{
  txtTaskName->setFocus();
  txtTaskName->selectAll();
}

void EditDroneTaskWidget::accept()
{
  QString errors = isValidInput();

  if (errors.isEmpty())
  {
    droneTask.name = txtTaskName->text();
    droneTask.packageURL = txtPackage->text();
    droneTask.expiration_date = (chkEnableExpiration->isChecked() ? dtExpiration->dateTime().toTime_t() : 0);

    droneTask.rules.clear();
    for (int i = 0; i < lstRules->count(); i++)
    {
      QListWidgetItem *ruleEntry = lstRules->item(i);

      QVariant v = ruleEntry->data(Qt::UserRole);

      TaskRule rule;
      rule.operand = v.toMap()["operand"].toString();
      rule.op = v.toMap()["operator"].toString();
      rule.value = v.toMap()["value"].toString();

      droneTask.rules.append(rule);
    }

    // Send drone task data with package file to web service
    // If successful then close window
    // otherwise show error and allow user to try again
    webService->startAddDroneTask(droneTask);

    //QDialog::accept();
  }
  else
  {
    QLOG_DEBUG() << QString("User tried to submit an incomplete drone task. Error: %1").arg(errors);
    QMessageBox::warning(this, tr("Save"), tr("Correct the following problems:\n%1").arg(errors));
  }
}

void EditDroneTaskWidget::reject()
{
  QDialog::reject();
}

void EditDroneTaskWidget::choosePackage()
{
 // QSettings settings("IA", SOFTWARE_NAME);
  //QString videoCoverPath = settings.value("video_cover_path", QDir::homePath()).toString();

  QString fileName = QFileDialog::getOpenFileName(this, "Select Package", "", "BIN (*.bin)");

  if (!fileName.isEmpty())
  {
    txtPackage->setText(fileName);

    // Update the video cover path so we return to the same directory next time
   // QFileInfo file(fileName);
   // settings.setValue("video_cover_path", file.absolutePath());
  }
}

void EditDroneTaskWidget::operandChanged()
{
  cmbValues->clear();

  if (cmbOperands->currentIndex() > 0)
  {
    QVariant selectedItem = cmbOperands->itemData(cmbOperands->currentIndex());

    QString longestString = "";
    foreach (QVariant v, ruleValues)
    {
      if (v.toMap()["type"].toString() == selectedItem.toString())
      {
        // The key is the more user-friendly version which is shown in the UI
        cmbValues->addItem(v.toMap()["key"].toString(), v.toMap()["value"].toString());

        if (v.toMap()["key"].toString().length() > longestString.length())
          longestString = v.toMap()["key"].toString();
      }
    }

    // Resize popup list so it fits the widest item. Qt doesn't seem to do this on its own
    cmbValues->view()->setMinimumWidth(cmbValues->fontMetrics().boundingRect(longestString).width() + 15);
  }
}

void EditDroneTaskWidget::addRuleClicked()
{
  QLOG_DEBUG() << "User clicked add rule button";

  // Verify an operand, operator and value were specified
  if (cmbOperands->currentIndex() > 0 && cmbOperators->currentIndex() > 0 && cmbValues->currentText().length() > 0)
  {   
    QString operand = cmbOperands->currentText();
    QString op = cmbOperators->currentText();
    QString value = cmbValues->currentText();

    QListWidgetItem *item = new QListWidgetItem;
    item->setText(QString("%1    %2    \"%3\"").arg(operand).arg(op).arg(value));

    QVariantMap rule;
    rule["operand"] = cmbOperands->itemData(cmbOperands->currentIndex());
    rule["operator"] = cmbOperators->itemData(cmbOperators->currentIndex());
    rule["value"] = cmbValues->itemData(cmbValues->currentIndex());

    // Verify the rule doesn't already exist in the list
    bool duplicateRule = false;
    for (int i = 0; i < lstRules->count(); i++)
    {
      QListWidgetItem *ruleEntry = lstRules->item(i);

      QVariant v = ruleEntry->data(Qt::UserRole);

      if (v.toMap()["operand"] == rule["operand"] &&
          v.toMap()["operator"] == rule["operator"] &&
          v.toMap()["value"] == rule["value"])
      {
        duplicateRule = true;
        break;
      }
    }

    if (duplicateRule)
    {
      QLOG_DEBUG() << "User tried to add a rule that already exists in the list";
      QMessageBox::warning(this, tr("Add Rule"), tr("This rule already exists in the list. Rules must be unique."));
      cmbOperands->setFocus();
    }
    else
    {
      QLOG_DEBUG() << "Adding rule to list";

      item->setData(Qt::UserRole, rule);
      lstRules->addItem(item);

      // Reset drop down menus
      cmbOperands->setCurrentIndex(0);
      cmbOperators->setCurrentIndex(0);

      cmbOperands->setFocus();
    }
  }
  else
    QMessageBox::warning(this, tr("Add Rule"), tr("To add a rule, select an operand, operator and value."));

}

void EditDroneTaskWidget::removeRuleClicked()
{
  if (lstRules->currentRow() >= 0)
  {
    QLOG_DEBUG() << "Removing rule from list";

    QListWidgetItem *item = lstRules->takeItem(lstRules->currentRow());

    delete item;
  }
  else
  {
    QLOG_DEBUG() << QString("User tried to remove a rule without selecting anything");
    QMessageBox::warning(this, tr("Remove Rule"), tr("Select a rule from the list to remove it."), QMessageBox::Ok);
  }

  cmbOperands->setFocus();
}

QString EditDroneTaskWidget::isValidInput()
{
  QStringList errorList;

  txtTaskName->setText(txtTaskName->text().trimmed());
  txtPackage->setText(txtPackage->text().trimmed());

  if (txtTaskName->text().isEmpty())
    errorList.append("- Task Name cannot be empty.");
  else if (Global::containsExtendedChars(txtTaskName->text()))
    errorList.append("- Task Name cannot contain extended characters. If you cannot see any of these characters, try to clear the field and type it again.");
  else if (txtTaskName->text().contains("\""))
    errorList.append("- Task Name cannot contain the double-quote (\") character.");

  if (txtPackage->text().isEmpty())
    errorList.append("- Package cannot be empty.");
  else if (Global::containsExtendedChars(txtPackage->text()))
    errorList.append("- Package cannot contain extended characters. If you cannot see any of these characters, try to clear the field and type it again.");
  else if (txtPackage->text().contains("\""))
    errorList.append("- Package cannot contain the double-quote (\") character.");
  else if (!QFile::exists(txtPackage->text()))
    errorList.append("- Package file does not exist.");

  return errorList.join("\n");
}

bool EditDroneTaskWidget::dataChanged()
{
  bool changed = false;
/*
  if (movie.Title() != txtPackage->text().trimmed())
  {
    changed = true;
  }

  if (movie.Producer() != cmbOperands->currentText().trimmed())
  {
    changed = true;
  }

  if (movie.Category() != operator->currentText())
  {
    changed = true;
  }

  if (movie.Subcategory() != cmbValues->currentText().trimmed())
  {
    changed = true;
  }*/

  return changed;
}

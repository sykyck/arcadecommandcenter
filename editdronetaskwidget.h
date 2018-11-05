#ifndef EDITDRONETASKWIDGET_H
#define EDITDRONETASKWIDGET_H

#include <QDialog>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QStringList>
#include <QCalendarWidget>
#include <QDateTimeEdit>
#include <QKeyEvent>
#include <QListWidget>
#include "databasemgr.h"
#include "settings.h"
#include "webservice.h"
#include "dronetask.h"

class EditDroneTaskWidget : public QDialog
{
  Q_OBJECT
public:
  explicit EditDroneTaskWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent = 0);
  ~EditDroneTaskWidget();
  void populateForm(DroneTask droneTask);
  DroneTask getDroneTask();
  bool droneTaskChanged();

protected:
  //void resizeEvent(QResizeEvent *e);
  void showEvent(QShowEvent *);
  void accept();
  void reject();

signals:
  
public slots:
  void choosePackage();

private slots:
  void operandChanged();
  void addRuleClicked();
  void removeRuleClicked();
  void finishedAddingDroneTask(bool success, QVariantMap response);

private:
  QString isValidInput();

  bool dataChanged();

  QLabel *lblTaskName;
  QLabel *lblPackage;
  QLabel *lblExpiration;
  QLabel *lblRuleBuilder;

  QLineEdit *txtTaskName;
  QLineEdit *txtPackage;
  QCheckBox *chkEnableExpiration;
  QDateTimeEdit *dtExpiration;
  QComboBox *cmbOperands;
  QComboBox *cmbOperators;
  QComboBox *cmbValues;
  QListWidget *lstRules;

  QPushButton *btnSave;
  QPushButton *btnCancel;
  QToolButton *btnChoosePackage;
  QPushButton *btnAddRule;
  QPushButton *btnRemoveRule;

  QGridLayout *gridLayout;
  QHBoxLayout *buttonLayout;
  QHBoxLayout *expirationLayout;
  QHBoxLayout *backCoverButtonLayout;

  DatabaseMgr *dbMgr;
  Settings *settings;
  WebService *webService;
  DroneTask droneTask;

  QVariantList ruleValues;
};

#endif // EDITDRONETASKWIDGET_H

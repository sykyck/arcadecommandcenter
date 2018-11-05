#ifndef EDITSOFTWAREWIDGET_H
#define EDITSOFTWAREWIDGET_H

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

class EditSoftwareWidget : public QDialog
{
  Q_OBJECT
public:
  explicit EditSoftwareWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent = 0);
  ~EditSoftwareWidget();
  void populateForm(Software software);
  Software getSoftware();
  bool softwareChanged();

protected:
  //void resizeEvent(QResizeEvent *e);
  void showEvent(QShowEvent *);
  void accept();
  void reject();

signals:
  
public slots:
  void choosePackage();

private slots:
  void finishedAddingSoftware(bool success, QVariantMap response);
  void finishedUpdatingSoftware(bool success, QVariantMap response);

private:
  QString isValidInput();

  bool dataChanged();

  bool existingSoftware;

  QLabel *lblSoftwareName;
  QLabel *lblVersion;
  QLabel *lblPackage;
  QLabel *lblCurrentPackage;
  QLabel *lblCurrentPackageURL;

  QLineEdit *txtSoftwareName;
  QLineEdit *txtVersion;
  QLineEdit *txtPackage;

  QPushButton *btnSave;
  QPushButton *btnCancel;
  QToolButton *btnChoosePackage;

  QGridLayout *gridLayout;
  QHBoxLayout *buttonLayout;

  DatabaseMgr *dbMgr;
  Settings *settings;
  WebService *webService;
  Software software;
};

#endif // EDITSOFTWAREWIDGET_H

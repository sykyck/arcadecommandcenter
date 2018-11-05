#ifndef EDITLOCATIONWIDGET_H
#define EDITLOCATIONWIDGET_H

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
#include <QListWidget>
#include <QCalendarWidget>
#include <QDateTimeEdit>
#include <QKeyEvent>
#include <QListWidget>
#include "databasemgr.h"
#include "settings.h"
#include "webservice.h"
#include "dronetask.h"

class EditLocationWidget : public QDialog
{
  Q_OBJECT
public:
  explicit EditLocationWidget(DatabaseMgr *dbMgr, Settings *settings, WebService *webService, QWidget *parent = 0);
  ~EditLocationWidget();
  void populateForm(Location location);
  Location getLocation();
  bool locationChanged();

protected:
  //void resizeEvent(QResizeEvent *e);
  void showEvent(QShowEvent *);
  void accept();
  void reject();

signals:
  
public slots:

private slots:
  void finishedAddingLocation(bool success, QVariantMap response);
  void finishedUpdatingLocation(bool success, QVariantMap response);
  void addCollectionRecipientClicked();
  void removeCollectionRecipientClicked();
  void addServiceRecipientClicked();
  void removeServiceRecipientClicked();
  void addBillAcceptorRecipientClicked();
  void removeBillAcceptorRecipientClicked();
  void collectionRecipientDoubleClicked(const QModelIndex &index);
  void serviceRecipientDoubleClicked(const QModelIndex &index);
  void billAcceptorRecipientDoubleClicked(const QModelIndex &index);

private:
  QString isValidInput();
  QStringList listWidgetToStringList(QListWidget *listWidget);

  bool dataChanged();

  bool existingLocation;

  QLabel *lblLocationName;
  QLabel *lblPublicIpAddress;
  QLabel *lblCollectionRecipients;
  QLabel *lblServiceRecipients;
  QLabel *lblBillAcceptorRecipients;
  QLabel *lblAllowNotifications;
  QLabel *lblShowCollections;

  QLineEdit *txtLocationName;
  QLineEdit *txtPublicIpAddress;
  QListWidget *lstCollectionRecipients;
  QLineEdit *txtCollectionRecipient;
  QListWidget *lstServiceRecipients;
  QLineEdit *txtServiceRecipient;
  QListWidget *lstBillAcceptorRecipients;
  QLineEdit *txtBillAcceptorRecipient;

  QPushButton *btnAddCollectionRecipient;
  QPushButton *btnRemoveCollectionRecipient;
  QPushButton *btnAddServiceRecipient;
  QPushButton *btnRemoveServiceRecipient;
  QPushButton *btnAddBillAcceptorRecipient;
  QPushButton *btnRemoveBillAcceptorRecipient;
  QPushButton *btnSave;
  QPushButton *btnCancel;

  QCheckBox *chkAllowNotifications;
  QCheckBox *chkShowCollections;

  QGridLayout *gridLayout;
  QHBoxLayout *collectionRecipientButtonLayout;
  QHBoxLayout *serviceRecipientButtonLayout;
  QHBoxLayout *billAcceptorButtonLayout;
  QHBoxLayout *buttonLayout;

  DatabaseMgr *dbMgr;
  Settings *settings;
  WebService *webService;

  Location location;
};

#endif // EDITLOCATIONWIDGET_H

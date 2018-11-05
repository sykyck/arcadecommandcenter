#ifndef ALERTDETAILWIDGET_H
#define ALERTDETAILWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>
#include "alert.h"

class AlertDetailWidget : public QDialog
{
  Q_OBJECT
public:
  explicit AlertDetailWidget(QWidget *parent = 0);
  ~AlertDetailWidget();
  void populateForm(Alert alert);

signals:

private:  
  QLineEdit *txtAlertID;
  QLineEdit *txtDateTime;
  QLineEdit *txtLocation;
  QLineEdit *txtPublicIP;
  QLineEdit *txtLocalIP;
  QLineEdit *txtHostname;
  QLineEdit *txtUptime;
  QLineEdit *txtSoftware;
  QLineEdit *txtVersion;
  QTextEdit *txtMessage;
  QLineEdit *txtSent;

  QPushButton *btnClose;
  //QPushButton *btnCancel;
  QDialogButtonBox *buttonLayout;
  QFormLayout *formLayout;
  QVBoxLayout *verticalLayout;
};

#endif // ALERTDETAILWIDGET_H

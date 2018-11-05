#ifndef ALERT_H
#define ALERT_H

#include <QString>

struct Alert
{
  int alertID;
  quint32 dateTime;
  QString location;
  QString publicIP;
  QString localIP;
  QString hostname;
  QString uptime;
  QString software;
  QString version;
  QString message;
  bool sent;
};

#endif // ALERT_H

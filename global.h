#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QStringList>

const QString SOFTWARE_NAME = "Arcade Command Center";
const QString SOFTWARE_VERSION = "1.0.1"; // TODO: Always check version number before releasing!!!
const QString WEB_SERVICE_URL = "https://ws.usarcades.com/ws";

class Global
{
public:
  Global();

  static QString getIpAddress();
  static QString getHostname();
  static QString getUptime();
  static QString secondsToHMS(int duration);
  static QString secondsToDHMS(quint32 duration);
  static bool isValidEmail(QString emailAddress);
  static bool containsExtendedChars(const QString &s);
};

#endif // GLOBAL_H

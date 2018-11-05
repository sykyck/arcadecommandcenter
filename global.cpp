#include "global.h"
#include <QProcess>

Global::Global()
{
}

QString Global::getIpAddress()
{
  QString prog = "/bin/sh";
  QStringList arguments;
  arguments << "-c" << "ifconfig eth0 | grep 'inet ' | awk '{print $2}' | sed 's/addr://'";
  QProcess process;
  process.start(prog, arguments);

  QString address;
  if (!process.waitForFinished(10000))
    address = "ERROR";
  else
    address = process.readAll();

  return address;
}

QString Global::getHostname()
{
  QProcess process;
  process.start("hostname");

  QString hostname;
  if (!process.waitForFinished(10000))
    hostname = "ERROR";
  else
    hostname = process.readAll();

  return hostname.trimmed();
}

QString Global::getUptime()
{
  //QString prog = "/bin/sh";
  //QStringList arguments;
  //arguments << "-c" << "uptime";
  QProcess process;
  process.start("uptime");

  QString uptime;
  if (!process.waitForFinished(10000))
    uptime = "ERROR";
  else
    uptime = process.readAll();

  return uptime;
}

QString Global::secondsToHMS(int duration)
{
  QString res;

  int seconds = (int)(duration % 60);
  duration /= 60;
  int min = (int)(duration % 60);
  duration /= 60;

  int hours = 0;
  if (duration >= 24)
    hours = duration;
  else
    hours = (int)(duration % 24);

  return res.sprintf("%02d:%02d:%02d", hours, min, seconds);
}

// Convert a duration of time (in seconds) to days, hours, minutes seconds
QString Global::secondsToDHMS(quint32 duration)
{
  QString res;

  int seconds = (int)(duration % 60);
  duration /= 60;
  int minutes = (int)(duration % 60);
  duration /= 60;
  int hours = (int)(duration % 24);
  int days = (int)(duration / 24);

  if ((hours == 0) && (days == 0))
      return res.sprintf("%d min, %d sec", minutes, seconds);

  if (days == 0)
      return res.sprintf("%d hr, %d min, %d sec", hours, minutes, seconds);

  return res.sprintf("%d days, %d hr, %d min, %d sec", days, hours, minutes, seconds);
}

bool Global::isValidEmail(QString emailAddress)
{
  QRegExp mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
  mailREX.setCaseSensitivity(Qt::CaseInsensitive);
  mailREX.setPatternSyntax(QRegExp::RegExp);

  return mailREX.exactMatch(emailAddress);
}

bool Global::containsExtendedChars(const QString &s)
{
  bool found = false;

  foreach (QChar c, s)
  {
    if (int(c.toAscii()) < 32 || int(c.toAscii()) > 126)
    {
      found = true;
      break;
    }
  }

  return found;
}

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include "settings.h"
#include "encdec.h"
#include "qjson/serializer.h"
#include "qjson/parser.h"

Settings::Settings(QString settings, QObject *parent) : QObject(parent)
{
  loadSettings(settings);
}

QVariant Settings::getValue(QString keyName, QVariant defaultValue)
{
  if (_settings.contains(keyName))
  {
    return _settings[keyName];
  }
  else
  {
    if (defaultValue.isValid())
    {
      setValue(keyName, defaultValue);
      return defaultValue;
    }
    else
    {
      return QVariant();
    }
  }
}

void Settings::setValue(QString keyName, QVariant value)
{
  _settings[keyName] = value;
}

void Settings::loadSettings(QString settings)
{
  bool ok = false;
  QJson::Parser parser;
  QVariant var = parser.parse(settings.toAscii(), &ok);

  if (ok)
  {
    _settings = var.toMap();
  }
}

QString Settings::getSettings()
{
  QJson::Serializer serializer;
  QByteArray jsonData = serializer.serialize(_settings);

  return QString(jsonData.constData());
}


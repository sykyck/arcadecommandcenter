#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>
#include <QVariantMap>

const int SOFTWARE_UPDATE_INTERVAL = 3600000;       // 60 minutes
const int REFRESH_ALERTS_INTERVAL = 15 * 60 * 1000; // 15 minutes
const int REFRESH_DRONES_INTERVAL = 15 * 60 * 1000; // 15 minutes
const int REFRESH_DRONE_TASKS_INTERVAL = 15 * 60 * 1000; // 15 minutes
const int REFRESH_LOCATIONS_INTERVAL = 15 * 60 * 1000; // 15 minutes
const int REFRESH_SOFTWARE_INTERVAL = 15 * 60 * 1000; // 15 minutes

const qlonglong ALERT_HASH = 0; // 0 means we've never downloaded the data therefore we wouldn't have a hash
const qlonglong DRONE_HASH = 0;
const qlonglong TASK_HASH = 0;
const qlonglong LOCATION_HASH = 0;
const qlonglong SOFTWARE_HASH = 0;

class Settings : public QObject
{
  Q_OBJECT
public:

  explicit Settings(QString settings, QObject *parent = 0);
  QVariant getValue(QString keyName, QVariant defaultValue = QVariant());
  void setValue(QString keyName, QVariant value);
  void loadSettings(QString settings);
  QString getSettings();

private:
  QVariantMap _settings;
};

#endif // SETTINGS_H

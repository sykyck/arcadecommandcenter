#ifndef DATABASEMGR_H
#define DATABASEMGR_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QDate>
#include <QVariant>

#include "alert.h"
#include "drone.h"
#include "dronetask.h"
#include "location.h"
#include "software.h"

const QString SETTINGS_TABLE = "CREATE TABLE settings (key_name TEXT PRIMARY KEY NOT NULL UNIQUE, data TEXT)";
const QString ALERT_LOG_TABLE = "CREATE TABLE alert_log (alert_id INTEGER PRIMARY KEY NOT NULL UNIQUE, alert_time TEXT, name TEXT, public_ip_address TEXT, local_ip_address TEXT, hostname TEXT, uptime TEXT, software_name TEXT, version TEXT, log_message TEXT, sent INTEGER)";
const QString DRONES_TABLE = "CREATE TABLE drones (uuid TEXT PRIMARY KEY NOT NULL UNIQUE, current_task_id INTEGER, name TEXT, public_ip_address TEXT, local_ip_address TEXT, hostname TEXT, uptime TEXT, machine_type TEXT, os TEXT, check_in_time INTEGER)";
const QString DRONE_TASKS_TABLE = "CREATE TABLE drone_tasks (task_id INTEGER PRIMARY KEY NOT NULL UNIQUE, task_name TEXT, package_url TEXT, expiration_date INTEGER, num_rules INTEGER, num_drones_finished INTEGER, num_drones_applicable INTEGER)";
const QString LOCATIONS_TABLE = "CREATE TABLE locations (location_id INTEGER PRIMARY KEY NOT NULL UNIQUE, name TEXT, public_ip_address TEXT, collections_mailing_list TEXT, service_mailing_list TEXT, bill_acceptor_mailing_list TEXT, allow_notifications INTEGER, show_collections INTEGER)";
const QString SOFTWARE_TABLE = "CREATE TABLE software (software_id INTEGER PRIMARY KEY NOT NULL UNIQUE, software_name TEXT, version TEXT, installer_url TEXT, hash TEXT)";

class DatabaseMgr : public QObject
{
  Q_OBJECT
public:
  explicit DatabaseMgr(QString dataPath = QString(), QObject *parent = 0);

  // Open/create the database
  bool openDB(QString dbFile);

  bool verifyDb();

  // Close database
  void closeDB();

  QString getValue(QString keyName, QVariant defaultValue, bool *ok);
  bool setValue(QString keyName, QString value);

  bool insertAlertLogs(QVariantList records);
  QList<Alert> getAlertLogs();

  bool insertDrones(QVariantList records);
  QList<Drone> getDrones();

  bool insertDroneTasks(QVariantList records);
  QList<DroneTask> getDroneTasks();

  bool insertLocations(QVariantList records);
  QList<Location> getLocations();

  bool insertSoftware(QVariantList records);
  QList<Software> getSoftware();

  QVariantList getRuleValues();

signals:
  
public slots:

private:
  QSqlDatabase db;
  QString dataPath;
};

#endif // DATABASEMGR_H

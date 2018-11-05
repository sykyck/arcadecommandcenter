#include <QVariant>
#include <QDateTime>
#include <QStringList>
#include <QSqlDriver>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "databasemgr.h"
#include "qslog/QsLog.h"
#include "settings.h"

DatabaseMgr::DatabaseMgr(QString dataPath, QObject *parent) : QObject(parent)
{
  this->dataPath = dataPath;
}

bool DatabaseMgr::openDB(QString dbFile)
{
  // Find QSLite driver
  db = QSqlDatabase::addDatabase("QSQLITE", "main");

  db.setDatabaseName(dbFile);

  if (db.open())
    return true;
  else
    return false;
}

bool DatabaseMgr::verifyDb()
{
  bool ok = true;

  QSqlQuery query(db);

  query.exec("SELECT name FROM sqlite_master WHERE name = 'settings'");
  if (!query.first())
  {
    QLOG_DEBUG() << "Creating settings table";

    if (!query.exec(SETTINGS_TABLE))
      ok = false;
  }

  query.exec("SELECT name FROM sqlite_master WHERE name = 'alert_log'");
  if (!query.first())
  {
    QLOG_DEBUG() << "Creating alert_log table";

    if (!query.exec(ALERT_LOG_TABLE))
      ok = false;
  }

  query.exec("SELECT name FROM sqlite_master WHERE name = 'drones'");
  if (!query.first())
  {
    QLOG_DEBUG() << "Creating drones table";

    if (!query.exec(DRONES_TABLE))
      ok = false;
  }

  query.exec("SELECT name FROM sqlite_master WHERE name = 'drone_tasks'");
  if (!query.first())
  {
    QLOG_DEBUG() << "Creating drone_tasks table";

    if (!query.exec(DRONE_TASKS_TABLE))
      ok = false;
  }

  query.exec("SELECT name FROM sqlite_master WHERE name = 'locations'");
  if (!query.first())
  {
    QLOG_DEBUG() << "Creating locations table";

    if (!query.exec(LOCATIONS_TABLE))
      ok = false;
  }

  query.exec("SELECT name FROM sqlite_master WHERE name = 'software'");
  if (!query.first())
  {
    QLOG_DEBUG() << "Creating software table";

    if (!query.exec(SOFTWARE_TABLE))
      ok = false;
  }

  return ok;
}

void DatabaseMgr::closeDB()
{
  db.close();
}

QString DatabaseMgr::getValue(QString keyName, QVariant defaultValue, bool *ok)
{
  QSqlQuery query(db);
  QString value;

  *ok = false;

  query.prepare("SELECT data FROM settings WHERE key_name = :key_name");
  query.bindValue(":key_name", QVariant(keyName));

  if (query.exec())
  {
    if (query.isActive() && query.first())
    {
      value = query.value(0).toString();
      *ok = true;
    }
    else
    {
      //QLOG_DEBUG() << QString("'%1' not found, now adding").arg(keyName);

      // Looks like the key doesn't exist, if defaultValue is set then
      // try adding the record
      if (!defaultValue.isNull())
      {
        query.prepare("INSERT INTO settings (key_name, data) VALUES (:key_name, :data)");
        query.bindValue(":key_name", keyName);
        query.bindValue(":data", defaultValue);

        if (query.exec())
        {
          value = defaultValue.toString();
          *ok = true;
        }
      }
    }
  }

  return value;
}

bool DatabaseMgr::setValue(QString keyName, QString value)
{
  QSqlQuery query(db);

  query.prepare("UPDATE settings SET data = :data WHERE key_name = :key_name");
  query.bindValue(":data", QVariant(value));
  query.bindValue(":key_name", QVariant(keyName));

  if (query.exec())
  {
    //QLOG_DEBUG() << QString("Updated '%1' in settings table").arg(keyName);
    return true;
  }
  else
  {
    QLOG_ERROR() << QString("Could not update '%1' in settings table").arg(keyName);
    return false;
  }
}

bool DatabaseMgr::insertAlertLogs(QVariantList records)
{
  bool ok = true;

  QSqlQuery query(db);

  // Clear table because we assume that all alerts were downloaded
  if (!query.exec("DELETE FROM alert_log"))
    QLOG_ERROR() << "Could not clear alert_log table. Error:" << query.lastError().text();

  if (!query.exec("VACUUM"))
    QLOG_ERROR() << "Could not compact database. Error:" << query.lastError().text();

  query.prepare("INSERT INTO alert_log (alert_id, alert_time, name, public_ip_address, local_ip_address, hostname, uptime, software_name, version, log_message, sent) VALUES (:alert_id, :alert_time, :name, :public_ip_address, :local_ip_address, :hostname, :uptime, :software_name, :version, :log_message, :sent)");

  foreach (QVariant v, records)
  {
    QVariantMap rec = v.toMap();

    query.bindValue(":alert_id", rec["alert_id"].toInt());
    query.bindValue(":alert_time", rec["alert_time"].toUInt());
    query.bindValue(":name", rec["name"].toString());
    query.bindValue(":public_ip_address", rec["public_ip_address"].toString());
    query.bindValue(":local_ip_address", rec["local_ip_address"].toString());
    query.bindValue(":hostname", rec["hostname"].toString());
    query.bindValue(":uptime", rec["uptime"].toString());
    query.bindValue(":software_name", rec["software_name"].toString());
    query.bindValue(":version", rec["version"].toString());
    query.bindValue(":log_message", rec["log_message"].toString());
    query.bindValue(":sent", rec["sent"].toInt());

    if (!query.exec())
    {
      QLOG_ERROR() << "Could not insert record into alert_log table. Error:" << query.lastError().text();
      ok = false;
    }
  }

  return ok;
}

QList<Alert> DatabaseMgr::getAlertLogs()
{
  QList<Alert> records;

  QSqlQuery query(db);

  if (query.exec("SELECT alert_id, alert_time, name, public_ip_address, local_ip_address, hostname, uptime, software_name, version, log_message, sent FROM alert_log"))
  {
    while (query.next())
    {
      Alert rec;
      rec.alertID = query.value(0).toInt();
      rec.dateTime = query.value(1).toUInt();
      rec.location = query.value(2).toString();
      rec.publicIP = query.value(3).toString();
      rec.localIP = query.value(4).toString();
      rec.hostname = query.value(5).toString();
      rec.uptime = query.value(6).toString();
      rec.software = query.value(7).toString();
      rec.version = query.value(8).toString();
      rec.message = query.value(9).toString();
      rec.sent = query.value(10).toInt() == 1;

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query alert_log table. Error:" << query.lastError().text();
  }

  return records;
}

bool DatabaseMgr::insertDrones(QVariantList records)
{
  bool ok = true;

  QSqlQuery query(db);

  // Clear table because we assume that all drones were downloaded
  if (!query.exec("DELETE FROM drones"))
    QLOG_ERROR() << "Could not clear drones table. Error:" << query.lastError().text();

  if (!query.exec("VACUUM"))
    QLOG_ERROR() << "Could not compact database. Error:" << query.lastError().text();

  query.prepare("INSERT INTO drones (uuid, current_task_id, name, public_ip_address, local_ip_address, hostname, uptime, machine_type, os, check_in_time) VALUES (:uuid, :current_task_id, :name, :public_ip_address, :local_ip_address, :hostname, :uptime, :machine_type, :os, :check_in_time)");

  foreach (QVariant v, records)
  {
    QVariantMap rec = v.toMap();

    query.bindValue(":uuid", rec["uuid"].toString());
    query.bindValue(":current_task_id", rec["current_task_id"].toInt());
    query.bindValue(":name", rec["name"].toString());
    query.bindValue(":public_ip_address", rec["public_ip_address"].toString());
    query.bindValue(":local_ip_address", rec["local_ip_address"].toString());
    query.bindValue(":hostname", rec["hostname"].toString());
    query.bindValue(":uptime", rec["uptime"].toString());
    query.bindValue(":machine_type", rec["machine_type"].toString());
    query.bindValue(":os", rec["operating_system"].toString());
    query.bindValue(":check_in_time", rec["check_in_time"].toUInt());

    if (!query.exec())
    {
      QLOG_ERROR() << "Could not insert record into drone table. Error:" << query.lastError().text();
      ok = false;
    }
  }

  return ok;
}

QList<Drone> DatabaseMgr::getDrones()
{
  QList<Drone> records;

  QSqlQuery query(db);

  if (query.exec("SELECT uuid, current_task_id, name, public_ip_address, local_ip_address, hostname, uptime, machine_type, os, check_in_time FROM drones"))
  {
    while (query.next())
    {
      Drone rec;
      rec.uuid = query.value(0).toString();
      rec.currentTaskID = query.value(1).toInt();
      rec.location = query.value(2).toString();
      rec.publicIP = query.value(3).toString();
      rec.localIP = query.value(4).toString();
      rec.hostname = query.value(5).toString();
      rec.uptime = query.value(6).toString();
      rec.machineType = query.value(7).toString();
      rec.os = query.value(8).toString();
      rec.checkInTime = query.value(9).toUInt();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query drones table. Error:" << query.lastError().text();
  }

  return records;
}

bool DatabaseMgr::insertDroneTasks(QVariantList records)
{
  bool ok = true;

  QSqlQuery query(db);

  // Clear table because we assume that all drone tasks were downloaded
  if (!query.exec("DELETE FROM drone_tasks"))
    QLOG_ERROR() << "Could not clear drone_tasks table. Error:" << query.lastError().text();

  if (!query.exec("VACUUM"))
    QLOG_ERROR() << "Could not compact database. Error:" << query.lastError().text();

  query.prepare("INSERT INTO drone_tasks (task_id, task_name, package_url, expiration_date, num_rules, num_drones_finished, num_drones_applicable) VALUES (:task_id, :task_name, :package_url, :expiration_date, :num_rules, :num_drones_finished, :num_drones_applicable)");

  foreach (QVariant v, records)
  {
    QVariantMap rec = v.toMap();

    query.bindValue(":task_id", rec["task_id"].toInt());
    query.bindValue(":task_name", rec["task_name"].toString());
    query.bindValue(":package_url", rec["package_url"].toString());
    query.bindValue(":expiration_date", rec["expiration_date"].toUInt());
    query.bindValue(":num_rules", rec["num_rules"].toInt());
    query.bindValue(":num_drones_finished", rec["num_done"].toInt());
    query.bindValue(":num_drones_applicable", rec["num_app"].toInt());

    if (!query.exec())
    {
      QLOG_ERROR() << "Could not insert record into drone_tasks table. Error:" << query.lastError().text();
      ok = false;
    }
  }

  return ok;
}

QList<DroneTask> DatabaseMgr::getDroneTasks()
{
  QList<DroneTask> records;

  QSqlQuery query(db);

  if (query.exec("SELECT task_id, task_name, package_url, expiration_date, num_rules, num_drones_finished, num_drones_applicable FROM drone_tasks"))
  {
    while (query.next())
    {
      DroneTask rec;
      rec.taskID = query.value(0).toInt();
      rec.name = query.value(1).toString();
      rec.packageURL = query.value(2).toString();
      rec.expiration_date = query.value(3).toUInt();
      rec.numRules = query.value(4).toInt();
      rec.numDronesFinished = query.value(5).toInt();
      rec.numDronesApplicable = query.value(6).toInt();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query drone_tasks table. Error:" << query.lastError().text();
  }

  return records;
}

bool DatabaseMgr::insertLocations(QVariantList records)
{
  bool ok = true;

  QSqlQuery query(db);

  // Clear table because we assume that all locations were downloaded
  if (!query.exec("DELETE FROM locations"))
    QLOG_ERROR() << "Could not clear locations table. Error:" << query.lastError().text();

  if (!query.exec("VACUUM"))
    QLOG_ERROR() << "Could not compact database. Error:" << query.lastError().text();

  query.prepare("INSERT INTO locations (location_id, name, public_ip_address, collections_mailing_list, service_mailing_list, bill_acceptor_mailing_list, allow_notifications, show_collections) VALUES (:location_id, :name, :public_ip_address, :collections_mailing_list, :service_mailing_list, :bill_acceptor_mailing_list, :allow_notifications, :show_collections)");

  foreach (QVariant v, records)
  {
    QVariantMap rec = v.toMap();

    query.bindValue(":location_id", rec["location_id"].toInt());
    query.bindValue(":name", rec["name"].toString());
    query.bindValue(":public_ip_address", rec["public_ip_address"].toString());
    query.bindValue(":collections_mailing_list", rec["collections_mailing_list"].toString());
    query.bindValue(":service_mailing_list", rec["service_mailing_list"].toString());
    query.bindValue(":bill_acceptor_mailing_list", rec["bill_acceptor_mailing_list"].toString());
    query.bindValue(":allow_notifications", rec["allow_notifications"].toInt());
    query.bindValue(":show_collections", rec["show_collections"].toInt());

    if (!query.exec())
    {
      QLOG_ERROR() << "Could not insert record into locations table. Error:" << query.lastError().text();
      ok = false;
    }
  }

  return ok;
}

QList<Location> DatabaseMgr::getLocations()
{
  QList<Location> records;

  QSqlQuery query(db);

  if (query.exec("SELECT location_id, name, public_ip_address, collections_mailing_list, service_mailing_list, bill_acceptor_mailing_list, allow_notifications, show_collections FROM locations"))
  {
    while (query.next())
    {
      Location rec;
      rec.locationID = query.value(0).toInt();
      rec.name = query.value(1).toString();
      rec.publicIP = query.value(2).toString();
      rec.collectionsMailingList = query.value(3).toString();
      rec.serviceMailingList = query.value(4).toString();
      rec.billAcceptorMailingList = query.value(5).toString();
      rec.allowNotifications = query.value(6).toInt() == 1;
      rec.showCollections = query.value(7).toInt() == 1;

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query locations table. Error:" << query.lastError().text();
  }

  return records;
}

bool DatabaseMgr::insertSoftware(QVariantList records)
{
  bool ok = true;

  QSqlQuery query(db);

  // Clear table because we assume that all software were downloaded
  if (!query.exec("DELETE FROM software"))
    QLOG_ERROR() << "Could not clear software table. Error:" << query.lastError().text();

  if (!query.exec("VACUUM"))
    QLOG_ERROR() << "Could not compact database. Error:" << query.lastError().text();

  query.prepare("INSERT INTO software (software_id, software_name, version, installer_url, hash) VALUES (:software_id, :software_name, :version, :installer_url, :hash)");

  foreach (QVariant v, records)
  {
    QVariantMap rec = v.toMap();

    query.bindValue(":software_id", rec["software_id"].toInt());
    query.bindValue(":software_name", rec["software_name"].toString());
    query.bindValue(":version", rec["version"].toString());
    query.bindValue(":installer_url", rec["installer_url"].toString());
    query.bindValue(":hash", rec["hash"].toString());

    if (!query.exec())
    {
      QLOG_ERROR() << "Could not insert record into software table. Error:" << query.lastError().text();
      ok = false;
    }
  }

  return ok;
}

QList<Software> DatabaseMgr::getSoftware()
{
  QList<Software> records;

  QSqlQuery query(db);

  if (query.exec("SELECT software_id, software_name, version, installer_url, hash FROM software"))
  {
    while (query.next())
    {
      Software rec;
      rec.softwareID = query.value(0).toInt();
      rec.name = query.value(1).toString();
      rec.version = query.value(2).toString();
      rec.installerURL = query.value(3).toString();
      rec.hash = query.value(4).toString();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query software table. Error:" << query.lastError().text();
  }

  return records;
}

QVariantList DatabaseMgr::getRuleValues()
{
  QVariantList records;

  QSqlQuery query(db);

  if (query.exec("SELECT DISTINCT hostname FROM drones ORDER BY hostname"))
  {
    while (query.next())
    {
      QVariantMap rec;
      rec["type"] = "hostname";
      rec["key"] = query.value(0).toString();
      rec["value"] = query.value(0).toString();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query drones table. Error:" << query.lastError().text();
  }

  if (query.exec("SELECT DISTINCT name, public_ip_address FROM locations UNION SELECT DISTINCT name, public_ip_address FROM drones ORDER BY name"))
  {
    while (query.next())
    {
      QVariantMap rec;
      rec["type"] = "public_ip_address";
      rec["key"] = query.value(0).toString() + " / " + query.value(1).toString();
      rec["value"] = query.value(1).toString();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query drones table. Error:" << query.lastError().text();
  }

  if (query.exec("SELECT DISTINCT local_ip_address FROM drones ORDER BY local_ip_address"))
  {
    while (query.next())
    {
      QVariantMap rec;
      rec["type"] = "local_ip_address";
      rec["key"] = query.value(0).toString();
      rec["value"] = query.value(0).toString();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query drones table. Error:" << query.lastError().text();
  }

  if (query.exec("SELECT DISTINCT machine_type FROM drones ORDER BY machine_type"))
  {
    while (query.next())
    {
      QVariantMap rec;
      rec["type"] = "machine_type";
      rec["key"] = query.value(0).toString();
      rec["value"] = query.value(0).toString();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query drones table. Error:" << query.lastError().text();
  }

  if (query.exec("SELECT DISTINCT os FROM drones ORDER BY os"))
  {
    while (query.next())
    {
      QVariantMap rec;
      rec["type"] = "os";
      rec["key"] = query.value(0).toString();
      rec["value"] = query.value(0).toString();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query drones table. Error:" << query.lastError().text();
  }

  if (query.exec("SELECT uuid, name, local_ip_address, hostname FROM drones ORDER BY name, local_ip_address, hostname"))
  {
    while (query.next())
    {
      QVariantMap rec;
      rec["type"] = "uuid";
      rec["key"] = query.value(1).toString() + " / " + query.value(2).toString() + " / " + query.value(3).toString();
      rec["value"] = query.value(0).toString();

      records.append(rec);
    }
  }
  else
  {
    QLOG_ERROR() << "Could not query drones table. Error:" << query.lastError().text();
  }

  return records;
}



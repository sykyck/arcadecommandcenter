#ifndef DRONETASK_H
#define DRONETASK_H

#include <QString>
#include <QList>
#include "taskrule.h"

struct DroneTask
{  
  int taskID;
  QString name;
  QString packageURL;
  //QString expirationDate;
  quint32 expiration_date;
  QList<TaskRule> rules;
  int numRules; // Only used when getting list of drone tasks from server so we don't download a huge list of rules
  int numDronesFinished;
  int numDronesApplicable;
};

#endif // DRONETASK_H

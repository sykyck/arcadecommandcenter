#ifndef DRONE_H
#define DRONE_H

#include <QString>

struct Drone
{
  QString uuid;
  int currentTaskID;
  QString location;
  QString publicIP;
  QString localIP;
  QString hostname;
  QString uptime;
  QString machineType;
  QString os;
  quint32 checkInTime;
  QString formattedCheckInTime;
};

#endif // DRONE_H

#ifndef SOFTWARE_H
#define SOFTWARE_H

#include <QString>

struct Software
{
  int softwareID;
  QString name;
  QString version;
  QString installerURL;
  QString hash;  
};

#endif // SOFTWARE_H

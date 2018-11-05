#ifndef LOCATION_H
#define LOCATION_H

#include <QString>

struct Location
{
  int locationID;
  QString name;
  QString publicIP;
  QString collectionsMailingList;
  QString serviceMailingList;
  QString billAcceptorMailingList;
  bool allowNotifications;
  bool showCollections;

  Location()
  {
    locationID = 0;
    allowNotifications = false;
    showCollections = false;
  }
};

#endif // LOCATION_H

#include "mainwindow.h"
#include "qslog/QsLog.h"
#include "global.h"
#include <QDir>
#include <QSettings>
#include <QCoreApplication>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QDebug>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  // Get the path our executable is located in, this is used
  // for building paths to other resources
  QDir appDir(QCoreApplication::applicationDirPath());

  QLOG_DEBUG() << QString("%1 v%2 starting...").arg(SOFTWARE_NAME).arg(SOFTWARE_VERSION);

  dbMgr = new DatabaseMgr;
  dbMgr->openDB(appDir.absoluteFilePath("arcade-cmd-center.sqlite"));
  dbMgr->verifyDb();

  // Load JSON-formatted settings from database
  bool ok = false;
  QString tempSettings = dbMgr->getValue("all_settings", "", &ok);

  // The Settings class expects a JSON-formatted string containing all the settings
  // If the string is empty then there are no settings in the beginning
  settings = new Settings(tempSettings);

  // BUG: The task packager needs to encrypt the files, generate HMAC-SHA1 of that (uses a secret key with SHA1) and append the hash to the end of the file
  // Then when the client downloads the package, it removes the n bytes from the end of the file, generates the HMAC-SHA1 using the same secrety key
  // and verifies the hash is equal to what was taken off the end of the file. By doing this we verify the file integrity and authenticity and can
  // be sure it will decrypt. This is something Steve Gibson on Security Now (episode from 3/17/15) talked about. This will also negate the need
  // for storing the hash of the file in the database since it is now part of the file and is one less piece of information sent back to the client  

  webService = new WebService(WEB_SERVICE_URL, "@2R91C3tDS4Z6t*96@^ScBIrv@*P#Hj2");

  this->setWindowTitle(SOFTWARE_NAME + " v" + SOFTWARE_VERSION);

  createActions();
  createMenus();

  manualUpdateCheck = false;

  alertsWidget = new AlertsWidget(dbMgr, settings, webService);
  connect(alertsWidget, SIGNAL(updateStatusMessage(QString)), this, SLOT(updateStatus(QString)));
  dronesWidget = new DronesWidget(dbMgr, settings, webService);
  droneTasksWidget = new DroneTasksWidget(dbMgr, settings, webService);
  locationsWidget = new LocationsWidget(dbMgr, settings, webService);
  softwareWidget = new SoftwareWidget(dbMgr, settings, webService);

  tabs = new QTabWidget;
  tabs->addTab(alertsWidget, tr("Alerts"));
  tabs->addTab(dronesWidget, tr("Drones"));
  tabs->addTab(droneTasksWidget, tr("Drone Tasks"));
  tabs->addTab(softwareWidget, tr("Software"));
  tabs->addTab(locationsWidget, tr("Locations"));

  this->setCentralWidget(tabs);

  // Make the status bar at the bottom of the window visible
  this->statusBar()->show();

//  softwareUpdater = new Updater(WEB_SERVICE_URL);
//  checkForUpdateTimer = new QTimer;
//  checkForUpdateTimer->setInterval(settings->getValue("software_update_interval", SOFTWARE_UPDATE_INTERVAL).toInt());
//  connect(checkForUpdateTimer, SIGNAL(timeout()), this, SLOT(checkForUpdate()));
//  connect(softwareUpdater, SIGNAL(finishedChecking(QString)), this, SLOT(checkForUpdateFinished(QString)));
//  checkForUpdateTimer->start();

  // Check for update after event loop goes idle
//  QTimer::singleShot(0, this, SLOT(checkForUpdate()));
}

MainWindow::~MainWindow()
{
  fileMenu->deleteLater();
  toolsMenu->deleteLater();
  helpMenu->deleteLater();
  optionsAction->deleteLater();
  exitAction->deleteLater();
//  softwareUpdateAction->deleteLater();
  //documentationAction->deleteLater();
  aboutAction->deleteLater();
  tabs->deleteLater();
  dbMgr->deleteLater();
  settings->deleteLater();
}

bool MainWindow::isBusy()
{
  // TODO: Implement
  return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (isBusy())
  {
    QMessageBox::warning(this, tr("Busy"), tr("You cannot exit the program until it finishes the current operation. Please wait for it to finish."));
    event->ignore();
  }
  else
  {
    // Save settings on exit
    dbMgr->setValue("all_settings", settings->getSettings());
  }
}

void MainWindow::showEvent(QShowEvent *)
{
  //this->setWindowState(Qt::WindowMaximized);
  QRect screenGeometry = QApplication::desktop()->screenGeometry();

  this->setGeometry(0, 0, 1600, 768);
  int x = (screenGeometry.width() - this->width()) / 2;
  int y = (screenGeometry.height() - this->height()) / 2;
  this->move(x, y);
}

void MainWindow::changeEvent(QEvent *e)
{
  /*
  if (e->type() == QEvent::WindowStateChange)
  {
    // If no window state then it means the maximize button was clicked
    // Normally this will toggle between a maximized window and a smaller window (no state)
    // so override this behavior by returning it to maximized so the user can't reduce the size
    if (this->windowState() == Qt::WindowNoState)
      this->setWindowState(Qt::WindowMaximized);
  }*/
}

void MainWindow::exitProgram()
{
  this->close();
}

void MainWindow::showOptions()
{
  QLOG_DEBUG() << QString("Showing Settings window");
}

void MainWindow::checkForUpdate()
{
  // Make sure we're not busy before checking for update
  if (!isBusy())
  {
    QLOG_DEBUG() << "Checking for software update...";
    updateStatus(tr("Checking for software update..."));
    softwareUpdater->checkForUpdate(SOFTWARE_NAME, SOFTWARE_VERSION);
  }
  else
  {
    QLOG_DEBUG() << "Skipping software update check because we're busy";

    if (manualUpdateCheck)
      QMessageBox::warning(this, tr("Busy"), tr("Cannot check for software update while another operation is in progress. Please wait for it to finish."));

    manualUpdateCheck = false;
  }
}

void MainWindow::manuallyCheckForUpdate()
{
  manualUpdateCheck = true;
  checkForUpdate();
}

void MainWindow::checkForUpdateFinished(QString response)
{
  if (manualUpdateCheck)
  {
    QMessageBox::information(this, SOFTWARE_NAME, response);
    manualUpdateCheck = false;
  }
}

void MainWindow::showAbout()
{
  // In case system date is wrong, just use 2015
  int currentYear = QDate::currentDate().year();

  if (currentYear < 2015)
    currentYear = 2015;

  QMessageBox::about(this, tr("About"), tr("%1 v%2\n\nCopyright 2013-%3 US Arcades, Inc.\nAll Rights Reserved\n\nThe %4 software is used to monitor the arcades, execute remote commands and release software updates. Please see documentation and videos for more information.")
                     .arg(SOFTWARE_NAME)
                     .arg(SOFTWARE_VERSION)
                     .arg(currentYear)
                     .arg(SOFTWARE_NAME));
}

void MainWindow::updateStatus(QString message)
{
  this->statusBar()->showMessage(message, 2000);
}

void MainWindow::createActions()
{
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(QKeySequence::Quit);
  exitAction->setStatusTip(tr("Close the application"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(exitProgram()));

  optionsAction = new QAction(tr("S&ettings..."), this);
  optionsAction->setShortcut(Qt::ALT + Qt::Key_E);
  optionsAction->setStatusTip(tr("Settings"));
  //optionsAction->setEnabled(false);
  connect(optionsAction, SIGNAL(triggered()), this, SLOT(showOptions()));

//  softwareUpdateAction = new QAction(tr("Check for &updates..."), this);
//  softwareUpdateAction->setShortcut(Qt::ALT + Qt::Key_U);
//  softwareUpdateAction->setStatusTip(tr("Check for updates to this software"));
//  connect(softwareUpdateAction, SIGNAL(triggered()), this, SLOT(manuallyCheckForUpdate()));

 // documentationAction = new QAction(tr("&Documentation and Videos"), this);
 // documentationAction->setShortcut(Qt::ALT + Qt::Key_D);
 // documentationAction->setStatusTip(tr("View documentation and videos on using the %1 software.").arg(SOFTWARE_NAME));
 // connect(documentationAction, SIGNAL(triggered()), this, SLOT(showDocumentation()));

  aboutAction = new QAction(tr("&About %1...").arg(SOFTWARE_NAME), this);
  aboutAction->setShortcut(Qt::ALT + Qt::Key_A);
  aboutAction->setStatusTip(tr("Information about the %1 software.").arg(SOFTWARE_NAME));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(exitAction);

  toolsMenu = menuBar()->addMenu(tr("&Tools"));
  toolsMenu->addAction(optionsAction);
//  toolsMenu->addAction(softwareUpdateAction);
  //toolsMenu->addAction(restartDevicesAction);

  helpMenu = menuBar()->addMenu(tr("&Help"));
  //helpMenu->addAction(documentationAction);
  //helpMenu->addSeparator();
  helpMenu->addAction(aboutAction);
}

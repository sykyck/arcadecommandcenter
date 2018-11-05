#include "qtsingleapplication/QtSingleApplication"
//#include <QFileInfo>
#include <QDir>
#include "mainwindow.h"
#include <QApplication>
#include "qslog/QsLog.h"
#include "qslog/QsLogDest.h"

int main(int argc, char *argv[])
{
  QtSingleApplication app(argc, argv);

  if (app.isRunning())
  {
    qDebug("Another instance of '%s' is already running, now exiting.", qPrintable(QFileInfo(QtSingleApplication::applicationFilePath()).fileName()));

    return 0;
  }
  else
  {
    using namespace QsLogging;

    // Init the logging mechanism
    Logger &logger = Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);
    const QString logPath(QDir(app.applicationDirPath()).filePath("arcade-cmd-center.log"));

    // Add file and console destinations
    DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(logPath, EnableLogRotation, MaxSizeBytes(1024 * 1024 * 2), MaxOldLogCount(20)));
    DestinationPtr debugDestination(DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);

    // Change style to CleanLooks which is the default for Gnome
    // this style shows buttons and comboboxes highlighted when they have focus
    app.setStyle("cleanlooks");

    MainWindow win;
    win.show();

    return app.exec();
  }
}

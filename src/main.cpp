#include <QFileInfo>
#include <QList>
#include <QtWidgets/QApplication>
#include <QImageReader>
#include "AbstractTheme.h"
#include "FusionDarkTheme.h"
#include "FusionLightTheme.h"
#include "DarkStyleTheme.h"
#include "DarkOrangeTheme.h"
#include "GenericTheme.h"

#include "MainWindow.h"
#include "AniEditorWindow.h"

#include "DarkStyle.h"
 

void myMessageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        abort();

    default:
        txt = msg;
        break;

    }
    QFile outFile("log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << Qt::endl;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QSettings settings("settings.ini", QSettings::IniFormat);
   // qInstallMessageHandler(myMessageHandler);
    QApplication::setStyle("fusion");
    QImageReader::setAllocationLimit(1024 * 4);
     

    QList<TilesEditor::AbstractTheme*> themes = {
        new TilesEditor::FusionLightTheme(),
        new TilesEditor::FusionDarkTheme(),
        new TilesEditor::DarkStyleTheme(),
        new TilesEditor::DarkOrangeTheme(),
        new TilesEditor::GenericTheme("Aqua", ":/Aqua.qss", false),
        new TilesEditor::GenericTheme("Elegant Dark", ":/ElegantDark.qss", true),
        new TilesEditor::GenericTheme("Material Dark", ":/MaterialDark.qss", true),
        new TilesEditor::GenericTheme("Light Style", ":/qdarkstyle/light/lightstyle.qss", false)
    };

    auto currentTheme = settings.value("theme").toString();
     
    auto launchAniEditor = QFileInfo(QCoreApplication::applicationFilePath()).baseName().toLower() == "ganieditor";

    QStringList launchFiles;
    QStringList searchDirectories;

    for (int i = 1; i < argc; ++i)
    {
        if (QString(argv[i]) == "-g")
            launchAniEditor = true;

        else if (QString(argv[i]) == "-s" && i + 1 < argc) {
            searchDirectories.append(argv[++i]);
        }
        else launchFiles.append(argv[i]);

    }

    if (launchAniEditor)
    {
        TilesEditor::AniEditorWindow w(app, settings, nullptr, nullptr);
        w.addThemesButton();
        
        for (auto theme : themes)
        {
            w.addTheme(theme);
            if (theme->text() == currentTheme)
            {
                theme->trigger();
            }
        }

        for (auto& searchPath : searchDirectories)
            w.addSearchDir(searchPath);

        if (launchFiles.size())
        {
            for (auto& fullPath : launchFiles)
            {
                w.openGaniFilename(fullPath);
            }
        }

        w.show();


        return app.exec();
    }

    TilesEditor::MainWindow w(app, settings);
    for (auto theme : themes)
    {
        w.addTheme(theme);
        if (theme->text() == currentTheme)
        {
            theme->trigger();
        }
    }

    for (auto& searchPath : searchDirectories)
        w.addSearchDir(searchPath);

    if (launchFiles.size())
    {
        for (auto& fullPath: launchFiles)
        {
            w.openLevelFilename(fullPath);
        }
    }
    w.show();


    
    return app.exec();
}

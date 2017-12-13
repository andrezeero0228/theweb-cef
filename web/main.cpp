#include "mainwindow.h"
#include <QApplication>
#include <QTimer>
#include "cef/app.h"
#include <include/cef_app.h>
#include <QFontDatabase>
#include "cef/client.h"
#include <QDir>

Client* cefClient;
CefBrowserSettings::struct_type browserSettings;
QTimer CefEventLoopTimer;

int main(int argc, char *argv[])
{
    CefMainArgs main_args(argc, argv);

    if (CefExecuteProcess(main_args, NULL, NULL) >= 0) {
        //CEF handles this process
        //Return here
        return 0;
    }

    CefSettings cSettings;
    CefString(&cSettings.cache_path) = QDir::homePath().append("/.theweb/cache").toStdString();

    CefRefPtr<App> app(new App);
    CefInitialize(main_args, cSettings, app.get(), NULL);

    QApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theWeb");

    a.setQuitOnLastWindowClosed(true);

    cefClient = new Client();

    /*MainWindow w;
    w.show();*/

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(NULL);
    browserSettings.background_color = 0xFFFFFFFF;
    CefString(&browserSettings.sans_serif_font_family) = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family().toStdString();
    CefString(&browserSettings.fixed_font_family) = QFontDatabase::systemFont(QFontDatabase::FixedFont).family().toStdString();
    CefBrowserHost::CreateBrowser(windowInfo, cefClient, "http://www.google.com/", browserSettings, CefRefPtr<CefRequestContext>());

    CefEventLoopTimer.setInterval(0);
    QObject::connect(&CefEventLoopTimer, &QTimer::timeout, [=] {
        CefDoMessageLoopWork();
    });
    CefEventLoopTimer.start();

    int exec = a.exec();

    CefCookieManager::GetGlobalManager(NULL).get()->FlushStore(NULL);
    CefShutdown();

    return exec;
}

void QuitApp(int exitCode = 0) {
    CefEventLoopTimer.stop();
    QApplication::exit(exitCode);
}

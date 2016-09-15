#include "mainwindow.h"
#include "ui_mainwindow.h"

extern CefHandler* handler;
extern SignalBroker* signalBroker;
extern void QuitApp(int exitCode = 0);
extern QTimer cefEventLoopTimer;

MainWindow::MainWindow(Browser newBrowser, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->errorFrame->setVisible(false);
    ui->warningFrame->setVisible(false);
    ui->menubar->setVisible(false);
    ui->JsDialogFrame->setVisible(false);
    ui->AuthFrame->setVisible(false);

    ui->toolBar->addWidget(ui->spaceSearch);

    QMenu* menu = new QMenu();
    menu->addAction(ui->actionNew_Window);
    menu->addSeparator();
    menu->addAction(ui->actionSettings);
    menu->addAction(ui->actionAbout_theWeb);
    menu->addSeparator();
    menu->addAction(ui->actionExit);

    QToolButton* menuButton = new QToolButton();
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setIcon(QIcon(":/icons/icon"));
    menuButton->setMenu(menu);
    ui->toolBar->insertWidget(ui->actionGo_Back, menuButton);

    QMenu* powerMenu = new QMenu();
    powerMenu->addSection("Framerate limiting");
    powerMenu->addAction(ui->actionDon_t_Limit);
    powerMenu->addAction(ui->actionLimit_to_60_fps);
    powerMenu->addAction(ui->actionLimit_to_30_fps);
    powerMenu->addAction(ui->actionLimit_to_15_fps);
    powerMenu->addAction(ui->actionLimit_to_1_fps);

    QToolButton* powerButton = new QToolButton();
    powerButton->setPopupMode(QToolButton::InstantPopup);
    powerButton->setIcon(QIcon::fromTheme("battery"));
    powerButton->setMenu(powerMenu);
    ui->toolBar->addWidget(powerButton);

    connect(signalBroker, SIGNAL(RenderProcessTerminated(Browser,CefRequestHandler::TerminationStatus)), this, SLOT(RenderProcessTerminated(Browser,CefRequestHandler::TerminationStatus)));
    connect(signalBroker, SIGNAL(TitleChanged(Browser,CefString)), this, SLOT(TitleChanged(Browser,CefString)));
    connect(signalBroker, SIGNAL(AddressChange(Browser,CefRefPtr<CefFrame>,CefString)), this, SLOT(AddressChange(Browser,CefRefPtr<CefFrame>,CefString)));
    connect(signalBroker, SIGNAL(FullscreenModeChange(Browser,bool)), this, SLOT(FullscreenModeChange(Browser,bool)));
    connect(signalBroker, SIGNAL(JSDialog(CefRefPtr<CefBrowser>,CefString,CefHandler::JSDialogType,CefString,CefString,CefRefPtr<CefJSDialogCallback>,bool&)), this, SLOT(JSDialog(CefRefPtr<CefBrowser>,CefString,CefHandler::JSDialogType,CefString,CefString,CefRefPtr<CefJSDialogCallback>,bool&)));
    connect(signalBroker, SIGNAL(LoadingStateChange(Browser,bool,bool,bool)), this, SLOT(LoadingStateChange(Browser,bool,bool,bool)));
    connect(signalBroker, SIGNAL(LoadError(Browser,CefRefPtr<CefFrame>,CefHandler::ErrorCode,CefString,CefString)), this, SLOT(LoadError(Browser,CefRefPtr<CefFrame>,CefHandler::ErrorCode,CefString,CefString)));
    connect(signalBroker, SIGNAL(BeforeClose(Browser)), this, SLOT(BeforeClose(Browser)));
    connect(signalBroker, SIGNAL(AuthCredentials(Browser,CefRefPtr<CefFrame>,bool,CefString,int,CefString,CefString,CefRefPtr<CefAuthCallback>)), this, SLOT(AuthCredentials(Browser,CefRefPtr<CefFrame>,bool,CefString,int,CefString,CefString,CefRefPtr<CefAuthCallback>)));
    connect(signalBroker, SIGNAL(BeforeUnloadDialog(Browser,CefString,bool,CefRefPtr<CefJSDialogCallback>)), this, SLOT(BeforeUnloadDialog(Browser,CefString,bool,CefRefPtr<CefJSDialogCallback>)));
    connect(signalBroker, SIGNAL(BeforePopup(Browser,CefRefPtr<CefFrame>,CefString,CefString,CefLifeSpanHandler::WindowOpenDisposition,bool,CefPopupFeatures,CefWindowInfo*,CefBrowserSettings,bool*)), this, SLOT(BeforePopup(Browser,CefRefPtr<CefFrame>,CefString,CefString,CefLifeSpanHandler::WindowOpenDisposition,bool,CefPopupFeatures,CefWindowInfo*,CefBrowserSettings,bool*)));

    CefWindowInfo windowInfo;
    //windowInfo.SetAsChild(0, CefRect(0, 0, 100, 100));

    CefBrowserSettings settings;

    if (newBrowser.get() == 0) {
        browser = CefBrowserHost::CreateBrowserSync(windowInfo, CefRefPtr<CefHandler>(handler), "http://www.google.com/", settings, CefRefPtr<CefRequestContext>());
    } else {
        browser = newBrowser;
    }

    QWindow* window = QWindow::fromWinId(browser.get()->GetHost()->GetWindowHandle());
    browserWidget = QWidget::createWindowContainer(window);

    ((QBoxLayout*) ui->centralwidget->layout())->insertWidget(ui->centralwidget->layout()->indexOf(ui->loadingProgressBar), browserWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (!handler->canClose(browser)) {
        browser.get()->GetHost().get()->CloseBrowser(false);
        event->ignore();
    }

    if (event->isAccepted()) {
        browserWidget->close();
    }
}

void MainWindow::on_actionExit_triggered()
{
    QuitApp();
}

void MainWindow::on_actionGo_Back_triggered()
{
    browser.get()->GoBack();
}

void MainWindow::on_actionGo_Forward_triggered()
{
    browser.get()->GoForward();
}

void MainWindow::on_reloadErrorButton_clicked()
{
    ui->actionReload->trigger();
}

void MainWindow::on_actionReload_triggered()
{
    browser.get()->ReloadIgnoreCache();
}

bool MainWindow::IsCorrectBrowser(Browser browser) {
    if (browser.get() != NULL && this->browser.get() != NULL) {
        return browser.get()->GetIdentifier() == this->browser.get()->GetIdentifier();
    } else {
        return false;
    }
}

void MainWindow::RenderProcessTerminated(Browser browser, CefRequestHandler::TerminationStatus status) {
    if (IsCorrectBrowser(browser)) {
        ui->errorTitle->setText("Well, this is strange.");
        switch (status) {
        case TS_PROCESS_CRASHED:
            ui->errorText->setText("The window crashed.");
            break;
        case TS_PROCESS_WAS_KILLED:
            ui->errorText->setText("The window was closed by the system.");
            break;
        case TS_ABNORMAL_TERMINATION:
            ui->errorText->setText("The window closed abnormally.");
        }

        ui->errorFrame->setVisible(true);
    }
}

void MainWindow::TitleChanged(Browser browser, const CefString& title) {
    if (IsCorrectBrowser(browser)) {
        this->setWindowTitle(QString::fromStdString(title.ToString()).append(" - theWeb"));
    }
}

void MainWindow::on_spaceSearch_returnPressed()
{
    browser.get()->GetMainFrame().get()->LoadURL(ui->spaceSearch->text().toStdString());
}

void MainWindow::AddressChange(Browser browser, CefRefPtr<CefFrame> frame, const CefString &url) {
    if (IsCorrectBrowser(browser) && frame.get()->IsMain()) {
        ui->spaceSearch->setCurrentUrl(QUrl(QString::fromStdString(url.ToString())));
    }
}

void MainWindow::FullscreenModeChange(Browser browser, bool fullscreen) {
    if (IsCorrectBrowser(browser)) {
        static bool wasMaximized = false;
        if (fullscreen) {
            wasMaximized = this->isMaximized();
            ui->toolBar->setVisible(false);
            this->showFullScreen();

            currentWarning = MainWindow::fullscreen;
            QUrl currentUrl(QString::fromStdString(browser.get()->GetMainFrame().get()->GetURL().ToString()));
            ui->warningLabel->setText(currentUrl.host() + " is now full screen.");

            ui->warningOk->setVisible(true);
            ui->warningCancel->setVisible(true);
            ui->warningOk->setText("OK");
            ui->warningCancel->setText("Exit Full Screen");
            ui->warningFrame->setVisible(true);
        } else {
            ui->toolBar->setVisible(true);
            if (wasMaximized) {
                this->showMaximized();
            } else {
                this->showNormal();
            }

            if (currentWarning == MainWindow::fullscreen) {
                ui->warningFrame->setVisible(false);
                currentWarning = none;
            }
        }
    }
}

void MainWindow::on_warningOk_clicked()
{
    switch (currentWarning) {
    case fullscreen:
        ; //do nothing
    }
    ui->warningFrame->setVisible(false);

    currentWarning = none;
}

void MainWindow::on_warningCancel_clicked()
{
    switch (currentWarning) {
    case fullscreen:
        browser.get()->GetMainFrame().get()->ExecuteJavaScript("document.webkitExitFullscreen()", "", 0);
    }
    ui->warningFrame->setVisible(false);

    currentWarning = none;
}

void MainWindow::on_actionNew_Window_triggered()
{
    MainWindow* newWin = new MainWindow();
    newWin->show();
}

void MainWindow::JSDialog(CefRefPtr<CefBrowser> browser, const CefString &origin_url, CefHandler::JSDialogType dialog_type, const CefString &message_text, const CefString &default_prompt_text, CefRefPtr<CefJSDialogCallback> callback, bool &suppress_message) {
    if (IsCorrectBrowser(browser)) {
        browserWidget->setVisible(false);
        ui->JsDialogFrame->setVisible(true);
        ui->JsDialogOk->setText("OK");
        ui->JsDialogCancel->setText("Cancel");
        ui->JsDialogText->setText(QString::fromStdString(message_text.ToString()));

        switch (dialog_type) {
        case JSDIALOGTYPE_ALERT:
            ui->JsDialogCancel->setVisible(false);
            ui->JsDialogPrompt->setVisible(false);
            break;
        case JSDIALOGTYPE_CONFIRM:
            ui->JsDialogCancel->setVisible(true);
            ui->JsDialogPrompt->setVisible(false);
            break;
        case JSDIALOGTYPE_PROMPT:
            ui->JsDialogCancel->setVisible(true);
            ui->JsDialogPrompt->setText(QString::fromStdString(default_prompt_text));
            ui->JsDialogPrompt->setVisible(true);

            ui->JsDialogPrompt->setFocus();
            break;
        }
        this->JsDialogCallback = callback;
    }
}

void MainWindow::LoadingStateChange(Browser browser, bool isLoading, bool canGoBack, bool canGoForward) {
    if (IsCorrectBrowser(browser)) {
        ui->actionGo_Back->setEnabled(canGoBack);
        ui->actionGo_Forward->setEnabled(canGoForward);

        if (isLoading) {
            ui->errorFrame->setVisible(false);
            browserWidget->setVisible(true);
            ui->loadingProgressBar->setVisible(true);
        } else {
            ui->loadingProgressBar->setVisible(false);
        }
    }
}

void MainWindow::LoadError(Browser browser, CefRefPtr<CefFrame> frame, CefHandler::ErrorCode errorCode, const CefString &errorText, const CefString &failedUrl) {
    if (IsCorrectBrowser(browser)) {
        if (errorCode != ERR_ABORTED && frame.get()->IsMain()) {
            switch (qrand() % 7) {
            case 0:
                ui->errorTitle->setText("This isn't supposed to happen...");
                break;
            case 1:
                ui->errorTitle->setText("Can't connect to webpage");
                break;
            case 2:
                ui->errorTitle->setText("Well...");
                break;
            case 3:
                ui->errorTitle->setText("Well, that didn't work.");
                break;
            case 4:
                ui->errorTitle->setText("Let's try that again, shall we?");
                break;
            case 5:
                ui->errorTitle->setText("Whoa!");
                break;
            case 6:
                ui->errorTitle->setText("Oops!");
                break;
            }


            switch (errorCode) {
            case ERR_NAME_NOT_RESOLVED:
                ui->errorText->setText("Couldn't find server");
                break;
            case ERR_TIMED_OUT:
                ui->errorText->setText("Server took too long to respond");
                break;
            case ERR_ACCESS_DENIED:
                ui->errorText->setText("Access Denied");
                break;
            case ERR_CACHE_MISS:
                ui->errorTitle->setText("Confirm Form Resubmission");
                ui->errorText->setText("Clicking \"Reload\" will send the form again. If you have done an action through this webpage, such as a purchase, it will be repeated.");
                break;
            case ERR_CONNECTION_RESET:
                ui->errorText->setText("The connection was reset");
                break;
            case ERR_CONNECTION_CLOSED:
            case ERR_EMPTY_RESPONSE:
                ui->errorText->setText("The server didn't send anything");
                break;
            case ERR_INTERNET_DISCONNECTED:
                ui->errorText->setText("Disconnected from the internet");
                break;
            case ERR_TOO_MANY_REDIRECTS:
                ui->errorText->setText("A redirect loop was detected");
                break;
            case ERR_UNSAFE_REDIRECT:
                ui->errorText->setText("Redirect not allowed");
                break;
            case ERR_UNSAFE_PORT:
                ui->errorText->setText("Disallowed Port");
                break;
            case ERR_INVALID_RESPONSE:
                ui->errorText->setText("Invalid Response");
                break;
            case ERR_UNKNOWN_URL_SCHEME:
                ui->errorText->setText("Unknown URL Scheme");
                break;
            case ERR_INVALID_URL:
                ui->errorText->setText("Invalid URL");
                break;
            default:
                ui->errorText->setText(QString::fromStdString(errorText.ToString()));
            }
            ui->errorFrame->setVisible(true);
            browserWidget->setVisible(false);
        }
    }
}

void MainWindow::BeforeClose(Browser browser) {
    if (IsCorrectBrowser(browser)) {
        this->close();
    }
}

void MainWindow::on_JsDialogOk_clicked()
{
    ui->JsDialogFrame->setVisible(false);
    browserWidget->setVisible(true);
    JsDialogCallback.get()->Continue(true, ui->JsDialogPrompt->text().toStdString());
}

void MainWindow::on_JsDialogCancel_clicked()
{
    ui->JsDialogFrame->setVisible(false);
    browserWidget->setVisible(true);
    JsDialogCallback.get()->Continue(false, "");
}

void MainWindow::BeforeUnloadDialog(Browser browser, const CefString &message_text, bool is_reload, CefRefPtr<CefJSDialogCallback> callback) {
    if (IsCorrectBrowser(browser)) {
        JsDialogCallback = callback;
        if (is_reload) {
            ui->JsDialogOk->setText("Reload anyway");
            ui->JsDialogCancel->setText("Don't Reload");
        } else {
            ui->JsDialogOk->setText("Leave anyway");
            ui->JsDialogCancel->setText("Don't leave");
        }
        ui->JsDialogText->setText(QString::fromStdString(message_text.ToString()));
        ui->JsDialogOk->setVisible(true);
        ui->JsDialogCancel->setVisible(true);
        ui->JsDialogPrompt->setVisible(false);
        browserWidget->setVisible(false);
        ui->JsDialogFrame->setVisible(true);
    }
}

void MainWindow::on_JsDialogPrompt_returnPressed()
{
    ui->JsDialogOk->click();
}

void MainWindow::AuthCredentials(Browser browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString &host, int port, const CefString &realm, const CefString &scheme, CefRefPtr<CefAuthCallback> callback) {
    if (IsCorrectBrowser(browser)) {
        AuthCallback = callback;
        browserWidget->setVisible(false);

        ui->AuthHost->setText("Log in to " + QString::fromStdString(host.ToString()));
        ui->AuthRealm->setText("Server Realm: " + QString::fromStdString(realm.ToString()));
        ui->AuthUsername->setText("");
        ui->AuthPassword->setText("");
        if (QUrl(QString::fromStdString(frame.get()->GetURL().ToString())).scheme() == "http" && scheme == "basic") {
            ui->AuthBASIC->setVisible(true);
        } else {
            ui->AuthBASIC->setVisible(false);
        }
        ui->AuthFrame->setVisible(true);

        ui->AuthUsername->setFocus();
    }
}

void MainWindow::on_AuthOk_clicked()
{
    ui->AuthFrame->setVisible(false);
    browserWidget->setVisible(true);
    AuthCallback.get()->Continue(ui->AuthUsername->text().toStdString(), ui->AuthPassword->text().toStdString());
}

void MainWindow::on_AuthCancel_clicked()
{
    ui->AuthFrame->setVisible(false);
    browserWidget->setVisible(true);
    AuthCallback.get()->Cancel();
}

void MainWindow::on_AuthUsername_returnPressed()
{
    ui->AuthOk->click();
}

void MainWindow::on_AuthPassword_returnPressed()
{
    ui->AuthOk->click();
}

void MainWindow::BeforePopup(Browser browser, CefRefPtr<CefFrame> frame, const CefString &target_url, const CefString &target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures &popupFeatures, CefWindowInfo *windowInfo, CefBrowserSettings settings, bool *no_javascript_access) {
    if (IsCorrectBrowser(browser)) {
        Browser newBrowser = browser.get()->GetHost().get()->CreateBrowserSync(*windowInfo, CefRefPtr<CefHandler>(handler), target_url.ToString(), CefBrowserSettings(), CefRefPtr<CefRequestContext>());
        MainWindow* newWin = new MainWindow(newBrowser);
        newWin->show();
    }
}

void MainWindow::on_actionLimit_to_60_fps_triggered()
{
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(true);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 60);
}

void MainWindow::on_actionLimit_to_30_fps_triggered()
{
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(true);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 30);
}

void MainWindow::on_actionLimit_to_15_fps_triggered()
{
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(true);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(1000 / 15);
}

void MainWindow::on_actionLimit_to_1_fps_triggered()
{
    ui->actionDon_t_Limit->setChecked(false);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(true);
    cefEventLoopTimer.setInterval(1000);
}

void MainWindow::on_actionDon_t_Limit_triggered()
{
    ui->actionDon_t_Limit->setChecked(true);
    ui->actionLimit_to_60_fps->setChecked(false);
    ui->actionLimit_to_30_fps->setChecked(false);
    ui->actionLimit_to_15_fps->setChecked(false);
    ui->actionLimit_to_1_fps->setChecked(false);
    cefEventLoopTimer.setInterval(0);
}

void MainWindow::on_actionAbout_theWeb_triggered()
{
    browser.get()->GetMainFrame().get()->LoadURL("theweb://theweb");
}

void MainWindow::on_actionSettings_triggered()
{
    browser.get()->GetMainFrame().get()->LoadURL("theweb://settings");
}

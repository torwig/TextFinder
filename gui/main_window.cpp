#include "main_window.h"
#include "ui_main_window.h"
#include "base/config.h"

#include <QMessageBox>
#include <QScreen>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      mMainWindow(new Ui::MainWindow),
      mTaskExecuter(new TaskExecuter(this))
{
    mMainWindow->setupUi(this);
    // Actions
    connect(mMainWindow->actionExit, &QAction::triggered,
            this, &MainWindow::onExitClicked);
    connect(mMainWindow->actionStart, &QAction::triggered,
            this, &MainWindow::startSearchTask);
    connect(mMainWindow->actionStop, &QAction::triggered,
            this, &MainWindow::stopSearchTask);
    // Buttons
    connect(mMainWindow->startPushButton, &QPushButton::clicked,
            this, &MainWindow::startSearchTask);
    connect(mMainWindow->stopPushButton, &QPushButton::clicked,
            this, &MainWindow::stopSearchTask);
    // SpinBoxes
    mMainWindow->numberOfThreadsSpinBox->setMinimum(
                Config::instance().kMinNumberOfThreads);
    mMainWindow->numberOfThreadsSpinBox->setMaximum(
                Config::instance().kMaxNumberOfThreads);
    mMainWindow->maxURLNumberSpinBox->setMinimum(
                Config::instance().kMinNumberOfUrls);
    mMainWindow->maxURLNumberSpinBox->setMaximum(
                Config::instance().kMaxNumberOfUrls);

    // TaskExecuter
    connect(this, &MainWindow::startSearch,
            mTaskExecuter, &TaskExecuter::startSearchTask);
    connect(this, &MainWindow::stopSearch,
            mTaskExecuter, &TaskExecuter::stopSearchTask);

    connect(mTaskExecuter, &TaskExecuter::downloadingUrl,
            this, &MainWindow::onDownloadingUrl);
    connect(mTaskExecuter, &TaskExecuter::urlDownloaded,
            this, &MainWindow::onUrlDownloaded);
    connect(mTaskExecuter, &TaskExecuter::errorOccuredAtUrl,
            this, &MainWindow::onErrorOccured);
    connect(mTaskExecuter, &TaskExecuter::scanningUrl,
            this, &MainWindow::onScanningUrl);
    connect(mTaskExecuter, &TaskExecuter::urlLimitWasReached,
            this, &MainWindow::onUrlWasReached);
    connect(mTaskExecuter, &TaskExecuter::pagesProcessing,
            this, &MainWindow::onPagesProcessing);
    connect(mTaskExecuter, &TaskExecuter::logMessage,
            this, &MainWindow::onLogMessage);
    connect(mTaskExecuter, &TaskExecuter::searchCompleted,
            this, &MainWindow::onSearchCompleted);
    connect(mTaskExecuter, &TaskExecuter::textFoundAtUrl,
            this, &MainWindow::onTextFoundAtUrl);
    connect(mTaskExecuter, &TaskExecuter::textNotFoundAtUrl,
            this, &MainWindow::onTextNotFoundAtUrl);

    connect(this, &MainWindow::prepareForExit,
            mTaskExecuter, &TaskExecuter::stopAllWorkers);
    connect(mTaskExecuter, &TaskExecuter::allThreadsStopped,
            this, &MainWindow::onAllThreadsStopped);
}

MainWindow::~MainWindow()
{
    delete mMainWindow;
}

void MainWindow::onDownloadingUrl(const QString& url)
{
    logMessage(QString("Downloading %1...").arg(url));
    setSearchStatus(QString("Downloading..."));
}

void MainWindow::onUrlDownloaded(const QString& url)
{
    logMessage(QString("Downloaded %1").arg(url));
}

void MainWindow::onScanningUrl(const QString& url)
{
    logMessage(QString("Scanning %1...").arg(url));
    setSearchStatus(QString("Scanning..."));
}

void MainWindow::onErrorOccured(const QString& url, const QString& errorString)
{
    logMessage(QString("Error at %1: %2").arg(url).arg(errorString));
}

void MainWindow::onTextFoundAtUrl(const QString& url)
{
    setSearchStatus(QString("Found!"));
    logMessage(QString("\"%1\" was found at: %2")
               .arg(mCurrentTask.searchString).arg(url));
}

void MainWindow::onTextNotFoundAtUrl(const QString& url)
{
    logMessage(QString("Not found at %1").arg(url));
}

void MainWindow::onTextNotFound()
{
    setSearchStatus(QString("Not found."));
    QMessageBox::information(this, QString("Search completed"),
                             QString("Unable to find text \"%1\"")
                             .arg(mCurrentTask.searchString));
}

void MainWindow::onUrlWasReached()
{
    logMessage(QString("URL limit has been reached."));
}

void MainWindow::onPagesProcessing(int n)
{
    logMessage(QString("In progress: %1 page(s) .").arg(n));
}

void MainWindow::onSearchCompleted(bool success)
{
    if (!mSearchInProgress) {
        return;
    }
    mSearchInProgress = false;
    updateUi();
    if (success) {
        if (!mWasFound) {
            mWasFound = true;
            QMessageBox::information(this, QString("Search completed"),
                                     QString("Text \"%1\" was found!")
                                     .arg(mCurrentTask.searchString));
        }
    }
    else {
        onTextNotFound();
    }
}

void MainWindow::onAllThreadsStopped()
{
    qApp->quit();
}

void MainWindow::onLogMessage(const QString& msg)
{
    logMessage(msg);
}

void MainWindow::showEvent(QShowEvent* ev)
{
    QScreen* screen = QApplication::primaryScreen();
    QRect geom = screen->geometry();
    move( ( geom.width() - width() ) / 2,
          ( geom.height() - height() ) / 2);
    QMainWindow::showEvent(ev);
}

void MainWindow::closeEvent(QCloseEvent* ev)
{
    emit prepareForExit();
    ev->ignore();
}

void MainWindow::startSearchTask()
{
    if (mSearchInProgress) {
        return;
    }
    mWasFound = false;
    mMainWindow->textEdit->clear();
    SearchTask searchTask;
    Config::instance().setNumberOfThreads(
                mMainWindow->numberOfThreadsSpinBox->value());
    Config::instance().setNumberOfUrls(
                mMainWindow->maxURLNumberSpinBox->value());
    searchTask.searchString = mMainWindow->searchForLineEdit->text();
    searchTask.startUrl = mMainWindow->startUrlLineEdit->text();
    if (searchTaskValid(searchTask)) {
        mSearchInProgress = true;
        mCurrentTask = searchTask;
        setSearchStatus(QString("Search started..."));
        logMessage(QString("Searching for \"%1\"...").arg(mCurrentTask.searchString));
        emit startSearch(mCurrentTask);
        updateUi();
    }
}

void MainWindow::stopSearchTask()
{
    logMessage(QString("Stopped."));
    setSearchStatus(QString("Stopped."));
    mSearchInProgress = false;
    emit stopSearch();
    updateUi();
}

void MainWindow::onExitClicked()
{
    emit prepareForExit();
}

bool MainWindow::searchTaskValid(const SearchTask& task)
{
    if (task.searchString.isEmpty()) {
        QMessageBox::critical(this, QString("Error"),
                              QString("Empty search string!"),
                              QMessageBox::Ok);
        return false;
    }
    if (task.startUrl.isEmpty()) {
        QMessageBox::critical(this, QString("Error"),
                              QString("Empty start URL!"),
                              QMessageBox::Ok);
        return false;
    }
    return true;
}

void MainWindow::logMessage(const QString& msg)
{
    if (mSearchInProgress) {
        mMainWindow->textEdit->append(msg);
    }
}

void MainWindow::updateUi()
{
    mMainWindow->startPushButton->setDisabled(mSearchInProgress);
    mMainWindow->actionStart->setDisabled(mSearchInProgress);
    mMainWindow->stopPushButton->setDisabled(!mSearchInProgress);
    mMainWindow->actionStop->setDisabled(!mSearchInProgress);
}

void MainWindow::setSearchStatus(const QString& status)
{
    mMainWindow->searchStatusLineEdit->setText(status);
}

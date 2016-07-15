#include "task_executer.h"
#include "base/config.h"
#include "base/text_finder.h"

#include <QDebug>
#include <QPair>

TaskExecuter::TaskExecuter(QObject* parent)
    : QObject(parent)
{

}

TaskExecuter::~TaskExecuter()
{

}

/**
 * @brief TaskExecuter::startSearchTask starts text search
 * @param task should be validated before
 */
void TaskExecuter::startSearchTask(SearchTask task)
{
    if (mSearchInProgress) {
        qDebug() << QString("Please, stop previous search and then start new one");
        return;
    }
    qDebug() << QString("Starting search for %1 from %2...")
                .arg(task.searchString).arg(task.startUrl);
    mCurrentTask = task;
    mSearchInProgress = true;
    if (Config::instance().getNumberOfThreads() > mWorkers.size()) {
        increaseNumberOfThreads();
    }
    TaskInfo taskInfo;
    taskInfo.searchUrl = task.startUrl;
    mTaskQueue.push_back(taskInfo);
    continueDownload();
}

void TaskExecuter::stopSearchTask()
{
    if (mSearchInProgress) {
        qDebug() << QString("Stopping search task...");
        mSearchInProgress = false;
        cleanup();
    }
}

void TaskExecuter::stopAllWorkers()
{
    qDebug() << QString("Deleting all workers...");
    stopSearchTask();
    for (auto it = mWorkers.begin(); it != mWorkers.end(); ++it) {
        delete it.key();
    }
    emit allThreadsStopped();
}

void TaskExecuter::increaseNumberOfThreads()
{
    qDebug() << QString("Increasing number of threads...");
    for (int i = mWorkers.size() + 1;
         i <= Config::instance().getNumberOfThreads(); ++i ) {
        PageDownloader* textFinder = new PageDownloader();
        QThread* thread = new QThread();
        connectWorkerSignals(textFinder);
        connect(textFinder, &PageDownloader::destroyed,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        textFinder->moveToThread(thread);
        thread->start();
        WorkerInfo workerInfo;
        workerInfo.thread = thread;
        mWorkers.insert(textFinder, workerInfo);
    }
}

void TaskExecuter::continueDownload()
{
    qDebug() << QString("Continue download pages...");
    while ( !allUrlsProcessed() ) {
        if (visitedUrlNumber() >= Config::instance().getNumberOfUrls()) {
            qDebug() << QString("Unable to start download new URL: limit.");
            return;
        }
        bool taskLaunched {false};
        int busyWorkers {0};
        for (auto it = mWorkers.begin(); it != mWorkers.end(); ++it) {
            ++busyWorkers;
            if (busyWorkers > Config::instance().getNumberOfThreads()) {
                qDebug() << QString("Using max number of workers.");
                return;
            }
            if (!it.value().hasWork) {
                QString url = getNextUrlForDownload();
                qDebug() << QString("Next URL: %1").arg(url);
                it.value().hasWork = true;
                it.value().searchUrl = url;
                mUrlsInProcess.insert(url);
                mVisitedUrls.insert(url);
                QMetaObject::invokeMethod(it.key(), "download",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, url));
                taskLaunched = true;
                qDebug() << QString("New task launched for %1").arg(url);
                emit pagesProcessing(mVisitedUrls.size());
                break;
            }
            else {
                qDebug() << QString("Worker #%1 is working.").arg(busyWorkers);
            }
        }
        if (!taskLaunched) {
            // All workers busy
            qDebug() << QString("No available workers.");
            break;
        }
    }
}

void TaskExecuter::connectWorkerSignals(PageDownloader* pageDownloader)
{
    connect(pageDownloader, &PageDownloader::downloadingUrl,
            this, &TaskExecuter::downloadingUrl);
    connect(pageDownloader, &PageDownloader::errorOccuredAtUrl,
            this, &TaskExecuter::onErrorOccuredAtUrl);
    connect(pageDownloader, &PageDownloader::downloadCompleted,
            this, &TaskExecuter::onDownloadCompletedAtUrl);
    connect(pageDownloader, &PageDownloader::logMessage,
            this, &TaskExecuter::logMessage);
}

void TaskExecuter::cleanup()
{
    qDebug() << QString("Cleanup.");
    mTextWasFound = false;
    for (auto it = mWorkers.begin(); it != mWorkers.end(); ++it) {
        it.value().hasWork = false;
        it.value().searchUrl.clear();
    }
    mUrlsInProcess.clear();
    mVisitedUrls.clear();
    mTaskQueue.clear();
    mNexTaskInQueue = 0;
}

bool TaskExecuter::allUrlsProcessed()
{
    return mNexTaskInQueue >= mTaskQueue.size();
}

int TaskExecuter::visitedUrlNumber()
{
    return mVisitedUrls.size();
}

bool TaskExecuter::urlLimitReached()
{
    return visitedUrlNumber() >= Config::instance().getNumberOfUrls();
}

QString TaskExecuter::getNextUrlForDownload()
{
    qDebug() << QString("Getting next URL for download...");
    while (!allUrlsProcessed()
           && mVisitedUrls.contains(mTaskQueue[mNexTaskInQueue].searchUrl)) {
        ++mNexTaskInQueue;
    }
    if (allUrlsProcessed()) {
        qDebug() << QString("All URLs from queue were processed.");
        return QString();
    }
    else {
        QString url = mTaskQueue[mNexTaskInQueue].searchUrl;
        ++mNexTaskInQueue;
        return url;
    }
}

void TaskExecuter::addUrlsToQueue(QStringList urls)
{
    qDebug() << QString("Adding child URLs to queue...");
    foreach (QString url, urls) {
        TaskInfo taskInfo;
        taskInfo.searchUrl = url;
        mTaskQueue.push_back(taskInfo);
    }
}

void TaskExecuter::onDownloadCompletedAtUrl(const QByteArray& data)
{
    PageDownloader* textFinder = qobject_cast<PageDownloader*>(sender());
    WorkerInfo workerInfo = mWorkers[textFinder];
    QString downloadedUrl = workerInfo.searchUrl;
    emit urlDownloaded(downloadedUrl);
    qDebug() << QString("Downloaded %1").arg(downloadedUrl);
    workerInfo.hasWork = false;
    workerInfo.searchUrl.clear();
    for (int i = 0; i < mTaskQueue.size(); ++i) {
        if (mTaskQueue[i].searchUrl == downloadedUrl) {
            qDebug() << QString("Setting content for %1").arg(downloadedUrl);
            mTaskQueue[i].content = data;
            mTaskQueue[i].finished = true;
            break;
        }
    }
    mWorkers[textFinder] = workerInfo;
    if (!mUrlsInProcess.contains(downloadedUrl)) {
        return;
    }
    mUrlsInProcess.remove(downloadedUrl);
    if (allUrlsProcessed() && mUrlsInProcess.empty()) {
        onAllUrlsDownloaded();
    }
    else if (!allUrlsProcessed() && mUrlsInProcess.empty()
             && urlLimitReached()) {
        onAllUrlsDownloaded();
    }
    else if (!allUrlsProcessed()) {
        continueDownload();
    }
    else {
        qDebug() << QString("Waiting for active downloads...");
    }
}

void TaskExecuter::onAllUrlsDownloaded()
{
    qDebug() << QString("All urls were downloaded.");
    while (!mTaskQueue.empty() && !mTextWasFound) {
        TaskInfo task = mTaskQueue.front();
        if (!task.finished) {
            // Start of child URLs
            qDebug() << QString("Current level was processed.");
            break;
        }
        if (!mSearchInProgress) {
            qDebug() << QString("Search was cancelled.");
            break;
        }
        emit scanningUrl(task.searchUrl);
        bool rv = TextFinder::findTextInContent(mCurrentTask.searchString,
                                                task.content);
        if (rv) {
            onTextFoundAtUrl(task.searchUrl);
            mTextWasFound = true;
            QString context = TextFinder::getTextContext(
                        mCurrentTask.searchString, task.content);
            emit logMessage(QString("Text was found in the following context: %1")
                            .arg(context));
        }
        else {
            onTextNotFoundAtUrl(task.searchUrl);
            QStringList childUrls = TextFinder::findUrlsInContent(task.content);
            addUrlsToQueue(childUrls);
        }
        mTaskQueue.pop_front();
    }
    if (mTextWasFound) {
        qDebug() << QString("Text was found!");
        onSearchCompleted();
    }
    else if (urlLimitReached()) {
        qDebug() << QString("URL limit was reached.");
        emit urlLimitWasReached();
        onSearchCompleted();
    }
    else {
        qDebug() << QString("Downloading next level.");
        mNexTaskInQueue = 0;
        continueDownload();
    }
}

void TaskExecuter::onTextFoundAtUrl(QString url)
{
    qDebug() << QString("Text found at %1").arg(url);
    emit textFoundAtUrl(url);
}

void TaskExecuter::onTextNotFoundAtUrl(QString url)
{
    qDebug() << QString("Text was not found at %1").arg(url);
    emit textNotFoundAtUrl(url);
}

void TaskExecuter::onErrorOccuredAtUrl(QString url, QString errorString)
{
    qDebug() << QString("Error at %1 : %2").arg(url).arg(errorString);
    emit errorOccuredAtUrl(url, errorString);
    onTextNotFoundAtUrl(url);
}

void TaskExecuter::onSearchCompleted()
{
    qDebug() << QString("Search completed.");
    mSearchInProgress = false;
    emit searchCompleted(mTextWasFound);
    cleanup();
}



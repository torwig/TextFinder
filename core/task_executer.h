#ifndef TASKEXECUTER_H
#define TASKEXECUTER_H

#include "base/search_task.h"
#include "core/page_downloader.h"

#include <QObject>
#include <QVector>
#include <QThread>
#include <QQueue>
#include <QMap>

class TaskExecuter : public QObject
{
    Q_OBJECT

public:
    struct WorkerInfo {
        QThread* thread;
        bool hasWork {false};
        QString searchUrl;
    };

    struct TaskInfo {
        QString searchUrl;
        QByteArray content;
        bool finished {false};
    };

    explicit TaskExecuter(QObject* parent = nullptr);
    ~TaskExecuter();

signals:
    void downloadingUrl(QString url);
    void urlDownloaded(QString url);
    void scanningUrl(QString url);
    void textNotFoundAtUrl(QString url);
    void textFoundAtUrl(QString url);
    void errorOccuredAtUrl(QString url, QString errorString);
    void searchCompleted(bool success);
    void allThreadsStopped();
    void urlLimitWasReached();
    void pagesProcessing(int);
    void logMessage(QString);

public slots:
    void startSearchTask(SearchTask task);
    void stopSearchTask();
    void stopAllWorkers();

private:
    void continueDownload();
    void increaseNumberOfThreads();
    void connectWorkerSignals(PageDownloader* pageDownloader);
    void cleanup();
    bool allUrlsProcessed();
    bool urlLimitReached();
    int visitedUrlNumber();
    QString getNextUrlForDownload();
    void addUrlsToQueue(QStringList urls);

private slots:
    void onDownloadCompletedAtUrl(const QByteArray& data);
    void onTextFoundAtUrl(QString url);
    void onTextNotFoundAtUrl(QString url);
    void onErrorOccuredAtUrl(QString url, QString errorString);
    void onSearchCompleted();
    void onAllUrlsDownloaded();

private:
    SearchTask mCurrentTask;
    bool mSearchInProgress {false};
    bool mTextWasFound {false};
    QMap<PageDownloader*, WorkerInfo> mWorkers;
    QSet<QString> mUrlsInProcess;
    QSet<QString> mVisitedUrls;
    QQueue<TaskInfo> mTaskQueue;
    int mNexTaskInQueue {0};

};

#endif // TASKEXECUTER_H

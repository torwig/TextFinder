#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QShowEvent>

#include "base/search_task.h"
#include "core/task_executer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void onDownloadingUrl(const QString& url);
    void onUrlDownloaded(const QString& url);
    void onScanningUrl(const QString& url);
    void onErrorOccured(const QString& url, const QString& errorString);
    void onTextFoundAtUrl(const QString& url);
    void onTextNotFoundAtUrl(const QString& url);
    void onTextNotFound();
    void onUrlWasReached();
    void onPagesProcessing(int n);
    void onSearchCompleted(bool success);
    void onAllThreadsStopped();
    void onLogMessage(const QString& msg);

signals:
    void startSearch(SearchTask task);
    void stopSearch();
    void prepareForExit();

protected:
    void showEvent(QShowEvent* ev);
    void closeEvent(QCloseEvent* ev);

private slots:
    void startSearchTask();
    void stopSearchTask();
    void onExitClicked();

private:
    bool searchTaskValid(const SearchTask& task);
    void logMessage(const QString& msg);
    void updateUi();
    void setSearchStatus(const QString& status);

private:
    Ui::MainWindow* mMainWindow;
    TaskExecuter* mTaskExecuter;
    bool mSearchInProgress {false};
    bool mWasFound {false};
    SearchTask mCurrentTask;

};

#endif // MAIN_WINDOW_H

#ifndef PAGEDOWNLOADER_H
#define PAGEDOWNLOADER_H

#include <QObject>
#include <QtNetwork>

class PageDownloader : public QObject
{
    Q_OBJECT

public:
    static const int kOkStatusCode {200};
    static const int kMovedPermanentlyStatusCode {301};
    static const int kMovedTemporarilyStatusCode {302};

    explicit PageDownloader(QObject* parent = nullptr);
    ~PageDownloader();

signals:
    void downloadingUrl(QString url);
    void errorOccuredAtUrl(QString url, QString errorString);
    void downloadCompleted(QByteArray);
    void logMessage(QString);

public slots:
    void download(QString url);

private slots:
    void onReplyFinished(QNetworkReply* reply);
    void onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

private:
    void cleanup();
    void downloadUrl(const QUrl& url);
    void processReply(QNetworkReply* reply);
    void processRedirection(QNetworkReply* reply);
    void onDownloadCompleted();

private:
    QString mUrlString;
    QScopedPointer<QNetworkAccessManager> mNetworkManager;

};

#endif // PAGEDOWNLOADER_H

#include "page_downloader.h"
#include "base/config.h"

#include <QDebug>

PageDownloader::PageDownloader(QObject* parent)
    : QObject(parent)
{
    qDebug() << QString("New PageDownloader created.");
}

PageDownloader::~PageDownloader()
{

}

void PageDownloader::download(QString url)
{
    qDebug() << QString("Downloading %1...").arg(url);
    if (!mNetworkManager) {
        mNetworkManager.reset(new QNetworkAccessManager);
        connect(mNetworkManager.data(), &QNetworkAccessManager::finished,
                this, &PageDownloader::onReplyFinished);
    }
    mUrlString = url;
    QUrl searchUrl(url);
    downloadUrl(searchUrl);
}

void PageDownloader::downloadUrl(const QUrl& url)
{
    qDebug() << QString("Downloading %1...").arg(url.toString());
    if (url.isValid()) {
        emit downloadingUrl(url.toString());
        mNetworkManager->get(QNetworkRequest(url));
    }
    else {
        emit errorOccuredAtUrl(mUrlString, QString("Invalid URL."));
        onDownloadCompleted();
    }
}

void PageDownloader::onReplyFinished(QNetworkReply* reply)
{
    int statusCode = reply->attribute(
                QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << QString("Reply from %1 finished with status code: %2.")
                .arg(mUrlString).arg(statusCode);
    if (statusCode == kOkStatusCode) {
        processReply(reply);
    }
    else if (statusCode == kMovedPermanentlyStatusCode
             || statusCode == kMovedTemporarilyStatusCode) {
        processRedirection(reply);
    }
    else {
        QString reasonPhrase = reply->attribute(
                    QNetworkRequest::HttpReasonPhraseAttribute).toString();
        qDebug() << QString("Error: %1").arg(reasonPhrase);
        emit errorOccuredAtUrl(mUrlString, reasonPhrase);
        emit downloadCompleted(QByteArray());
        onDownloadCompleted();
    }
    reply->deleteLater();
}

void PageDownloader::processReply(QNetworkReply* reply)
{
    emit downloadCompleted(reply->readAll());
    onDownloadCompleted();
}

void PageDownloader::processRedirection(QNetworkReply* reply)
{
    QUrl newUrl = reply->attribute(
                QNetworkRequest::RedirectionTargetAttribute).toUrl();
    qDebug() << QString("Redirection from %1 to %2 detected.")
                .arg(mUrlString).arg(newUrl.toString());
    downloadUrl(newUrl);
}

void PageDownloader::onDownloadCompleted()
{
    qDebug() << QString("Dowmloading of %1 completed.").arg(mUrlString);
    cleanup();
}

void PageDownloader::onSslErrors(QNetworkReply* reply,
                             const QList<QSslError>& errors)
{
    // Ignoring SSL errors
    reply->ignoreSslErrors(errors);
}

void PageDownloader::cleanup()
{
    mUrlString.clear();
}



#include "text_finder.h"

#include <QDebug>
#include <QUrl>

TextFinder::TextFinder()
{

}

TextFinder::~TextFinder()
{

}

bool TextFinder::findTextInContent(const QString& text, const QByteArray& data)
{
    qDebug() << QString("Searching for text %1 in page content...").arg(text);
    QString content = QString::fromUtf8(data);
    bool found = content.contains(text, Qt::CaseInsensitive);
    return found;
}

QStringList TextFinder::findUrlsInContent(const QByteArray& data)
{
    qDebug() << QString("Searching for child URLs...");
    QString source = QUrl::fromPercentEncoding(data);
    QStringList childUrlList;
    int urlStartPos = source.indexOf(kStartUrlPattern);
    while (urlStartPos != -1) {
        int urlEndPos = findUrlEndPos(source, urlStartPos + kStartUrlPattern.size());
        QString url = source.mid(urlStartPos, urlEndPos - urlStartPos);
        if (url != kStartUrlPattern && !childUrlList.contains(url)) {
            childUrlList.append(url);
        }
        urlStartPos = source.indexOf(kStartUrlPattern, urlEndPos);
    }
    return childUrlList;
}

QString TextFinder::getTextContext(const QString& text, const QByteArray& data)
{
    QString content = QString::fromUtf8(data);
    int foundAtPos = content.indexOf(text, 0, Qt::CaseInsensitive);
    const int boundary = 50;
    int startPos = foundAtPos > boundary ? foundAtPos - boundary : 0;
    QString context = content.mid(startPos, text.length() + 2*boundary);
    qDebug() << QString("Found in context: \"%1\".").arg(context);
    return context;
}

int TextFinder::findUrlEndPos(const QString& data, int startPos)
{
    int urlEndPos {data.size()};
    for (int i = 0; i < kInvalidUrlCharacters.size(); ++i) {
        int newUrlEndPos = data.indexOf(kInvalidUrlCharacters[i], startPos);
        if (newUrlEndPos != -1 && newUrlEndPos < urlEndPos) {
            urlEndPos = newUrlEndPos;
        }
    }
    return urlEndPos;
}

#ifndef TEXTFINDER_H
#define TEXTFINDER_H

#include <QString>
#include <QStringList>
#include <QByteArray>

const QString kStartUrlPattern {"https://"};
const QString kInvalidUrlCharacters {"!\"'*,:;<=>[]^`{|} \t\n"};

class TextFinder
{

public:
    TextFinder();
    ~TextFinder();

    static bool findTextInContent(const QString& text, const QByteArray& data);
    static QStringList findUrlsInContent(const QByteArray& data);
    static QString getTextContext(const QString& text, const QByteArray& data);

private:
    static int findUrlEndPos(const QString& data, int startPos);

};

#endif // TEXTFINDER_H

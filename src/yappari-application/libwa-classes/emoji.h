#ifndef EMOJI_H
#define EMOJI_H

#include <QObject>

class Emoji : public QObject
{
    Q_OBJECT
public:
    explicit Emoji(QObject *parent = 0);
    static Emoji *GetInstance(QObject *parent = 0);

    Q_INVOKABLE QString waTextToHtml(QString data, int iconSize = 32, bool urlConversion = false, const QString &linkColor = QString());
    Q_INVOKABLE QString htmlToWaText(QString html);

private:
    QString makeEmojiHtml(const QString &header, const QString &path, const QString &first, const QString &second);
    QString shortURL(QString url);

    QList<quint32> emojiList;

};

#endif // EMOJI_H

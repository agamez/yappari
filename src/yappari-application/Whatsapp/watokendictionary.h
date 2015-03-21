#ifndef WATOKENDICTIONARY_H
#define WATOKENDICTIONARY_H

#include <QStringList>

class WATokenDictionary : public QObject
{
    Q_OBJECT
public:
    WATokenDictionary(QObject * parent = 0);

    bool tryGetToken(const QString &string, bool &subdict, int &token);
    void getToken(QString &string, bool &subdict, int token);

    int primarySize() const;
    int secondarySize() const;

private:
    QStringList primaryStrings;
    QStringList secondaryStrings;
};

#endif // WATOKENDICTIONARY_H

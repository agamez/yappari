#ifndef MEDIADOWNLOAD_H
#define MEDIADOWNLOAD_H

#include <QFile>

#include "fmessage.h"

#include "httprequestv2.h"

class MediaDownload : public HttpRequestv2
{
    Q_OBJECT
public:
    explicit MediaDownload(QObject *parent = 0);

    void backgroundTransfer(FMessage message);

signals:
    void progress(FMessage msg, float p);
    void downloadFinished(MediaDownload *,FMessage msg);
    void httpError(MediaDownload *, FMessage msg, int error);

public slots:
    void onResponse();
    void writeToFile();

private:
    FMessage message;
    QString fileName;
    QFile file;
    qint64 totalLength;
    qint64 bytesWritten;

    QString getFileNameForMessage(FMessage message);
};

#endif // MEDIADOWNLOAD_H

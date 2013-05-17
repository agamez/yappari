#include "mediadownload.h"

#include "util/utilities.h"

#include "globalconstants.h"

#include "client.h"

MediaDownload::MediaDownload(QObject *parent) :
    HttpRequestv2(parent)
{
}

void MediaDownload::backgroundTransfer(FMessage message)
{
    this->message = message;
    fileName = getFileNameForMessage(message);
    file.setFileName(fileName);

    connect(this,SIGNAL(finished()),
            this,SLOT(onResponse()));

    get(message.media_url);
}

void MediaDownload::onResponse()
{
    if (errorCode != 200)
    {
        emit httpError(this, message, errorCode);
        return;
    }

    totalLength = getHeader("Content-Length").toLongLong();
    bytesWritten = 0;

    if (!file.open(QIODevice::WriteOnly))
    {
        // An error has occurred
        Utilities::logData("MediaDownload: Error while trying to opening file: " + fileName);
        socket->close();
    }

    if (socket->bytesAvailable())
        writeToFile();

    connect(socket,SIGNAL(readyRead()),this,SLOT(writeToFile()));
}

void MediaDownload::writeToFile()
{
    qint64 bytesToRead = socket->bytesAvailable();

    QByteArray buffer;
    buffer.resize(bytesToRead);

    if (bytesToRead + bytesWritten > totalLength)
        bytesToRead = totalLength - bytesWritten;

    if (file.write(socket->read(bytesToRead)) != bytesToRead)
    {
        // An error has occurred
        Utilities::logData("MediaDownload: Error while trying to opening file");
        file.close();
        socket->close();
    }

    bytesWritten += bytesToRead;

    float p = ((float)((bytesWritten) * 100.0)) / ((float)totalLength);

    emit progress(message,p);

    if (bytesWritten == totalLength)
    {
        file.close();
        socket->close();

        Utilities::logData("MediaDownload: Downloading finished.");

        message.local_file_uri = fileName;

        emit downloadFinished(this, message);
    }
}

QString MediaDownload::getFileNameForMessage(FMessage message)
{
    QString path = Utilities::getPathFor(message.media_wa_type, false);

    // Let's try to be organized here with the downloads
    if (Client::importMediaToGallery)
    {
        path.append(YAPPARI_DIR);
        QDir home = QDir::home();
        if (!home.exists(path))
            home.mkpath(path);
    }

    QString fileName = path + "/" + message.media_name;
    int pos = fileName.lastIndexOf('.');
    if (pos < 0)
        pos = fileName.length();

    QString extension = fileName.mid(pos);

    // Loop until a fileName is not currently being used
    QFile file(fileName);
    for (int count = 1; file.exists(); count++)
        file.setFileName(fileName.left(pos) + "_" + QString::number(count) + extension);

    return file.fileName();
}

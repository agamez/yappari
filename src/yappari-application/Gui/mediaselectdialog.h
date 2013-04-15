#ifndef MEDIASELECTDIALOG_H
#define MEDIASELECTDIALOG_H

#include <QDialog>

#include "Whatsapp/fmessage.h"

namespace Ui {
    class MediaSelectDialog;
}

class MediaSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MediaSelectDialog(QWidget *parent = 0);
    ~MediaSelectDialog();

    int getMediaSelected();

private:
    Ui::MediaSelectDialog *ui;

    int mediaWaTypeSelected;

public slots:
    void imageSelected();
    void videoSelected();
    void audioSelected();

};

#endif // MEDIASELECTDIALOG_H

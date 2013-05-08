#include <QStringList>
#include <QMessageBox>

#include "statuswindow.h"
#include "ui_statuswindow.h"

#include "Whatsapp/util/utilities.h"

#include "client.h"

StatusWindow::StatusWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StatusWindow)
{
    ui->setupUi(this);
    ui->textEdit->setMainWindow(this);

    ui->centralwidget->installEventFilter(this);

    connect(ui->saveButton,SIGNAL(clicked()),this,SLOT(verifyStatus()));
    connect(ui->textEdit,SIGNAL(returnPressed()),this,SLOT(verifyStatus()));
    connect(ui->selectEmojiButton,SIGNAL(clicked()),
            ui->textEdit,SLOT(selectEmojiButtonClicked()));
    connect(ui->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),
            this,SLOT(statusSelected(QListWidgetItem*)));

    QStringList defaultStatuses;

    defaultStatuses << "In a meeting" << "At the gym" << "Sleeping"
                    << "Battery about to die" << "At work" << "At the movies"
                    << "At school" << "Busy" << "Available" << "I am using Yappari!";

    foreach (QString status, defaultStatuses)
    {
        QListWidgetItem *item = new QListWidgetItem(status);
        ui->listWidget->insertItem(0,item);
    }

    ui->textEdit->setText(Utilities::WATextToHtml(Client::myStatus,32));
    ui->textEdit->moveCursor(QTextCursor::End);
    ui->textEdit->setFocus();
}

StatusWindow::~StatusWindow()
{
    delete ui;
}

bool StatusWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->centralwidget)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            ui->textEdit->closeEmojiWidget();

            return true;
        }
    }

    return QMainWindow::eventFilter(obj,event);
}

void StatusWindow::statusSelected(QListWidgetItem *item)
{
    ui->textEdit->setText(Utilities::WATextToHtml(item->text(),32));
    ui->listWidget->clearSelection();
}

void StatusWindow::verifyStatus()
{
    QString status = Utilities::htmlToWAText(ui->textEdit->toHtml());

    if (status.isEmpty())
    {
        QMessageBox msg(this);

        msg.setText("Status can't be empty");
        msg.exec();
    }
    else
    {
        emit changeStatus(status);
        close();
    }
}



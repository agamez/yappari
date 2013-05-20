#include <QMaemo5InformationBox>
#include <QMaemo5Style>

#include "Whatsapp/util/utilities.h"

#include "groupsubjectwindow.h"
#include "ui_groupsubjectwindow.h"

GroupSubjectWindow::GroupSubjectWindow(Group *group, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GroupSubjectWindow)
{
    this->group = group;

    ui->setupUi(this);

    ui->textEdit->setText(Utilities::WATextToHtml(group->name));
    ui->textEdit->setMainWindow(this);

    connect(ui->saveButton,SIGNAL(clicked()),this,SLOT(verifyNewSubject()));
    connect(ui->textEdit,SIGNAL(returnPressed()),this,SLOT(verifyNewSubject()));
    connect(ui->selectEmojiButton,SIGNAL(clicked()),
            ui->textEdit,SLOT(selectEmojiButtonClicked()));
}

GroupSubjectWindow::~GroupSubjectWindow()
{
    delete ui;
}


void GroupSubjectWindow::verifyNewSubject()
{
    QString newSubject = Utilities::htmlToWAText(ui->textEdit->toHtml());

    if (newSubject.isEmpty())
    {
        QMaemo5InformationBox::information(this,"Subject can't be empty",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else if (newSubject.length() > 25)
    {
        QMaemo5InformationBox::information(this,"Subject can't be longer than 25 characters",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        emit changeSubject(group->jid,newSubject);
        close();
    }
}

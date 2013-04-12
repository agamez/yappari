#ifndef FORMDATALIST_H
#define FORMDATALIST_H

#include <QList>

#include "formdata.h"

class FormDataList : public QList<FormData>
{
    Q_OBJECT
public:
    explicit FormDataList(QObject *parent = 0);

signals:

public slots:

};

#endif // FORMDATALIST_H

#ifndef GROUPPATICIPANTDELEGATE_H
#define GROUPPATICIPANTDELEGATE_H

#include <QStyledItemDelegate>

class GroupParticipantDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit GroupParticipantDelegate(QObject *parent = 0);

    void paint (QPainter *painter, const QStyleOptionViewItem& option,
                const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index ) const;

signals:

public slots:



};

#endif // GROUPPATICIPANTDELEGATE_H

#include <QTextDocument>
#include <QPainter>

#include "groupparticipantdelegate.h"

GroupParticipantDelegate::GroupParticipantDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void GroupParticipantDelegate::paint (QPainter *painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    if (option.state & QStyle::State_Selected){
        painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
    }

    QString html = index.data(Qt::DisplayRole).toString();
    QImage photo = index.data(Qt::UserRole).value<QImage>();

    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(html);

    // UGLY HACK BUT COULDN'T FIND ANOTHER WAY
    // doc.setTextWidth(TEXT_WIDTH);
    // doc.adjustSize();

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    painter->translate(options.rect.left(), options.rect.top());
    QRect imageClip(options.rect.width()-70, 5, 64, 64);

    if (!photo.isNull())
        painter->drawImage(imageClip, photo);

    int top = ((options.rect.height() / 2) - (doc.size().toSize().height() / 2) - 5);
    painter->translate(options.rect.left(), top);

    QRect clip(0, 0, options.rect.width()-70, doc.size().toSize().height());

    doc.drawContents(painter, clip);


    painter->restore();
}

QSize GroupParticipantDelegate::sizeHint(const QStyleOptionViewItem& option,
                                         const QModelIndex& index ) const
{
    Q_UNUSED(index);

    QSize size = option.rect.size();

    size.setHeight(78);

    return size;
}

/* Copyright 2013 Naikel Aparicio. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL EELI REILIN OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the author and should not be interpreted as representing
 * official policies, either expressed or implied, of the copyright holder.
 */

#include <QTextDocument>

#include "chatdisplaydelegate.h"

#define TEXT_WIDTH  700

ChatDisplayDelegate::ChatDisplayDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void ChatDisplayDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option,
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
    doc.setTextWidth(TEXT_WIDTH);
    // doc.adjustSize();

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    painter->translate(options.rect.left(), options.rect.top() + 5);
    QRect clip(0, 0, options.rect.width()-70, options.rect.height() - 5);

    doc.drawContents(painter, clip);

    QRect imageClip(options.rect.width()-70, 0, 64, 64);

    if (!photo.isNull())
        painter->drawImage(imageClip, photo);


    painter->restore();
}

QSize ChatDisplayDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex& index ) const
{
    // SIZE SHOULD BE IN THE INDEX.DATA
    // DOCUMENTS SHOULD BE CREATED IN THE ITEM CREATION

    Q_UNUSED(option);

    QTextDocument doc;

    QString html = index.data(Qt::DisplayRole).toString();

    doc.setHtml(html);

    // UGLY HACK BUT COULDN'T FIND ANOTHER WAY
    doc.setTextWidth(TEXT_WIDTH);
    //doc.setTextWidth(option.rect.width());
    // doc.adjustSize();

    QSize size = doc.size().toSize();
    size.setHeight(size.height()+20);

    return size;
}

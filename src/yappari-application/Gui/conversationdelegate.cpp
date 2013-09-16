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

#include "conversationdelegate.h"
#include <QFont>
#include <QTextDocument>
#include <QPushButton>

#include <QTextBrowser>

ConversationDelegate::ConversationDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void ConversationDelegate::paint (QPainter * painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    if(option.state & QStyle::State_Selected){
            painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
    }

    QString html = index.data(Qt::DisplayRole).toString();
    bool from_me = index.data(Qt::UserRole).toBool();

    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setTextWidth(options.rect.right());
    doc.setHtml(html);

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    painter->translate(options.rect.left(), options.rect.top()+8);
    QRect clip(0, 0, options.rect.width(), options.rect.height()-19);

    if (from_me)
    {
        QColor bg(0x33,0x33,0x33);
        QBrush brush(bg);
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setBackground(brush);
        QRect r(options.rect.left(), options.rect.top(),
                options.rect.width(),options.rect.height());
        painter->fillRect(clip,bg);
    }
    doc.drawContents(painter, clip);

    painter->restore();
}


QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index ) const
{
    // SIZE SHOULD BE IN THE INDEX.DATA
    // DOCUMENTS SHOULD BE CREATED IN THE ITEM CREATION

    QTextDocument doc;

    QString html = index.data(Qt::DisplayRole).toString();

    doc.setTextWidth(option.rect.width());
    doc.setHtml(html);

    QSize size = doc.size().toSize();
    size.setHeight(size.height()+20);

    return size;
}

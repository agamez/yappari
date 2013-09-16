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

#ifndef YAPPARI_STATUS_MENU_ITEM_H
#define YAPPARI_STATUS_MENU_ITEM_H

#include <libhildondesktop/libhildondesktop.h>

G_BEGIN_DECLS

#define YAPPARI_TYPE_STATUS_MENU_ITEM            (yappari_status_menu_item_get_type ())
#define YAPPARI_STATUS_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), YAPPARI_TYPE_STATUS_MENU_ITEM, YappariStatusMenuItem))
#define YAPPARI_STATUS_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), YAPPARI_TYPE_STATUS_MENU_ITEM, YappariStatusMenuItemClass))
#define YAPPARI_IS_STATUS_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), YAPPARI_TYPE_STATUS_MENU_ITEM))
#define YAPPARI_IS_STATUS_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), YAPPARI_TYPE_STATUS_MENU_ITEM))
#define YAPPARI_STATUS_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), YAPPARI_TYPE_STATUS_MENU_ITEM, YappariStatusMenuItemClass))

typedef struct _YappariStatusMenuItem        YappariStatusMenuItem;
typedef struct _YappariStatusMenuItemClass   YappariStatusMenuItemClass;
typedef struct _YappariStatusMenuItemPrivate YappariStatusMenuItemPrivate;

struct _YappariStatusMenuItem
{
  HDStatusMenuItem       parent;

  YappariStatusMenuItemPrivate       *priv;
};

struct _YappariStatusMenuItemClass
{
  HDStatusMenuItemClass  parent;
};

GType yappari_status_menu_item_get_type (void);

G_END_DECLS

#endif


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
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Eeli Reilin.
 */

#include <string.h>
#include <gtk/gtk.h>
#include <hildon/hildon.h>
#include <libosso.h>

#include "yappari-status-menu-plugin.h"

#define YAPPARI_STATUS_MENU_ITEM_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE (obj, YAPPARI_TYPE_STATUS_MENU_ITEM, YappariStatusMenuItemPrivate))

#define STATUSMENU_EXAMPLE_LIBOSSO_SERVICE_NAME "yappari_status_menu_item"
#define STATUSMENU_EXAMPLE_ICON_SIZE         48
#define STATUSMENU_EXAMPLE_AREA_ICON_SIZE    16

#define OSSO_YAPPARI_NAME    "Yappari"
#define OSSO_YAPPARI_SERVICE "org.scorpius."OSSO_YAPPARI_NAME
#define OSSO_YAPPARI_OBJECT  "/org/scorpius/"OSSO_YAPPARI_NAME
#define OSSO_YAPPARI_IFACE   "org.scorpius."OSSO_YAPPARI_NAME
#define OSSO_YAPPARI_MESSAGE "Waiting for a connection"

#define OSSO_APPLET_NAME    "YappariApplet"
#define OSSO_APPLET_SERVICE "org.scorpius."OSSO_APPLET_NAME
#define OSSO_APPLET_OBJECT  "/org/scorpius/"OSSO_APPLET_NAME
#define OSSO_APPLET_IFACE   "org.scorpius."OSSO_APPLET_NAME

#define METHOD_HIDE_APPLET  	"HideApplet"
#define METHOD_SHOW_APPLET  	"ShowApplet"
#define METHOD_STATUS_UPDATE	"status"
#define METHOD_NOTIFY           "Notify"

struct _YappariStatusMenuItemPrivate
{
  /* Widgets */
  GtkWidget *ex_button;

  /* Context */
  osso_context_t *osso;

  YappariStatusMenuItem *item;
};

HD_DEFINE_PLUGIN_MODULE (YappariStatusMenuItem, yappari_status_menu_item, HD_TYPE_STATUS_MENU_ITEM);

static void
yappari_status_menu_item_set_area_icon (YappariStatusMenuItem *item)
{
  /* ExampleStatusMenuItemPrivate *priv = item->priv; */
  GdkPixbuf *pixbuf = NULL;
  GtkIconTheme *icon_theme;


  icon_theme = gtk_icon_theme_get_default();
  pixbuf = gtk_icon_theme_load_icon(icon_theme, "yappari",
				    STATUSMENU_EXAMPLE_AREA_ICON_SIZE,
				    GTK_ICON_LOOKUP_NO_SVG,
				    NULL);

  /* Set the small status area icon */
  hd_status_plugin_item_set_status_area_icon (HD_STATUS_PLUGIN_ITEM(item),
                                              pixbuf);
  if (pixbuf != NULL)
        g_object_unref(pixbuf);

}

/**
 * Callback for the button clicked signal
 */
static void yappari_status_menu_icon_pressed_cb(GtkButton *button,
                                                YappariStatusMenuItem *item)
{
    YappariStatusMenuItemPrivate *priv = item->priv;
    osso_return_t ret;
    osso_rpc_t retval;

    ret = osso_rpc_run(priv->osso,
                       OSSO_YAPPARI_SERVICE,
                       OSSO_YAPPARI_OBJECT,
                       OSSO_YAPPARI_IFACE,
                       "ShowWindow", &retval, 
                       DBUS_TYPE_INVALID);

    osso_rpc_free_val(&retval);
}



static void
yappari_status_menu_item_class_finalize (YappariStatusMenuItemClass *klass)
{
}

static void
yappari_status_menu_item_finalize (GObject *object)
{
  YappariStatusMenuItem *item =
    YAPPARI_STATUS_MENU_ITEM (object);
  YappariStatusMenuItemPrivate *priv = item->priv;

  if(priv->osso)
    {
      osso_deinitialize(priv->osso);
      priv->osso = NULL;
    }

  G_OBJECT_CLASS (yappari_status_menu_item_parent_class)->finalize (object);
}

static void
yappari_status_menu_item_class_init (YappariStatusMenuItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = yappari_status_menu_item_finalize;

  g_type_class_add_private (klass, sizeof (YappariStatusMenuItemPrivate));
}

static void
yappari_status_menu_item_create_widgets (YappariStatusMenuItem *item)
{
  YappariStatusMenuItemPrivate *priv = item->priv;
  GtkWidget *button;
  GtkWidget *image;
  GtkIconTheme *icon_theme;
  GdkPixbuf *pixbuf;

  button = hildon_button_new (HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
                              HILDON_BUTTON_ARRANGEMENT_VERTICAL);

  hildon_button_set_style(HILDON_BUTTON(button),HILDON_BUTTON_STYLE_PICKER);
  hildon_button_set_alignment(HILDON_BUTTON(button),0.2,0.5,1,1);
  hildon_button_set_text(HILDON_BUTTON(button),OSSO_YAPPARI_NAME,OSSO_YAPPARI_MESSAGE);

  icon_theme = gtk_icon_theme_get_default();
  pixbuf = gtk_icon_theme_load_icon(icon_theme, "yappari",
		 		    STATUSMENU_EXAMPLE_ICON_SIZE,
		                    GTK_ICON_LOOKUP_NO_SVG, NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);
  if (pixbuf != NULL)
        g_object_unref(pixbuf);

  hildon_button_set_image(HILDON_BUTTON(button),image);

  priv->ex_button = button;

  g_signal_connect_after (G_OBJECT (button), "clicked",
                          G_CALLBACK (yappari_status_menu_icon_pressed_cb),
                          item);
  gtk_container_add (GTK_CONTAINER (item), button);
  gtk_widget_show_all (GTK_WIDGET (item));
  gtk_widget_hide (GTK_WIDGET (item));
}

/* Callback for normal D-BUS messages */
gint dbus_req_handler(const gchar * interface, const gchar * method,
                      GArray * arguments, gpointer data,
                      osso_rpc_t * retval)
{

    YappariStatusMenuItemPrivate *priv;
    priv = (YappariStatusMenuItemPrivate *) data;

    //osso_system_note_infoprint(priv->osso, method, retval);
    //osso_rpc_free_val(retval);

    if (!g_strcmp0(method,METHOD_HIDE_APPLET))
    {
        gtk_widget_hide(GTK_WIDGET(priv->item));
        hd_status_plugin_item_set_status_area_icon (HD_STATUS_PLUGIN_ITEM(priv->item),
                                                    NULL);
    }
    else if (!g_strcmp0(method,METHOD_SHOW_APPLET))
    {
        gtk_widget_show(GTK_WIDGET(priv->item));
        yappari_status_menu_item_set_area_icon(priv->item);
    }
    else if (!g_strcmp0(method,METHOD_STATUS_UPDATE))
    {
	osso_rpc_t arg = g_array_index(arguments,osso_rpc_t,0);
        hildon_button_set_text(HILDON_BUTTON(priv->ex_button),OSSO_YAPPARI_NAME,arg.value.s);
    }

    return OSSO_OK;
}

static void
yappari_status_menu_item_init (YappariStatusMenuItem *item)
{
  YappariStatusMenuItemPrivate *priv;
  osso_return_t result;

  priv = item->priv = YAPPARI_STATUS_MENU_ITEM_GET_PRIVATE (item);
  priv->item = item;

  /* Initialize */
  yappari_status_menu_item_create_widgets (item);
  priv->osso = osso_initialize (STATUSMENU_EXAMPLE_LIBOSSO_SERVICE_NAME,
                                "0.1",
                                TRUE,
                                NULL);
    /* Add handler for hello D-BUS messages */
    result = osso_rpc_set_cb_f(priv->osso, 
                               OSSO_APPLET_SERVICE, 
                               OSSO_APPLET_OBJECT, 
                               OSSO_APPLET_IFACE,
                               dbus_req_handler, priv);
    if (result != OSSO_OK) {
        g_print("Error setting D-BUS callback (%d)\n", result);
        // return OSSO_ERROR;
    }
}



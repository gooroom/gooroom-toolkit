/*
 *  Copyright (C) 2015-2017 Gooroom <gooroom@gooroom.kr>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <glib.h>
#include <gtk/gtk.h>

#include "grmkit-button.h"

struct _GrmkitButtonPrivate
{
    int index;
    GtkWidget* icon;
    GtkWidget* lbl_title;
    GtkWidget* lbl_desc;
};

G_DEFINE_TYPE_WITH_PRIVATE (GrmkitButton, grmkit_button, GTK_TYPE_BUTTON);

static void
grmkit_button_init (GrmkitButton *item)
{
    GrmkitButtonPrivate *priv;
    priv = item->priv = grmkit_button_get_instance_private (item);
    gtk_widget_init_template (GTK_WIDGET (item));
}

static void
grmkit_button_class_init (GrmkitButtonClass *klass)
{
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
                                      "/kr/gooroom/toolkit/button_main.ui");
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrmkitButton, icon);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrmkitButton, lbl_title);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrmkitButton, lbl_desc);
}

GrmkitButton *
grmkit_button_new (void)
{
    GrmkitButton *item;
    item = g_object_new (GRMKIT_TYPE_BUTTON, NULL);
    return item;
}

void
grmkit_button_set_title (GrmkitButton *button, const char* szTitle)
{
    GrmkitButtonPrivate *priv = button->priv;	
    gtk_label_set_text (GTK_LABEL(priv->lbl_title), szTitle);
}

void
grmkit_button_set_description (GrmkitButton *button, const char* szDesc)
{
    GrmkitButtonPrivate *priv = button->priv;
    gtk_label_set_text (GTK_LABEL(priv->lbl_desc), szDesc);
}

void 
grmkit_button_set_icon_from_file (GrmkitButton *button, const char* szFilePath)
{
    GrmkitButtonPrivate *priv = button->priv;

    GdkPixbuf *pixbuf;
    pixbuf = gdk_pixbuf_new_from_file (szFilePath, NULL);
    pixbuf = gdk_pixbuf_scale_simple (pixbuf, 32, 32, GDK_INTERP_HYPER);	
    gtk_image_set_from_pixbuf (GTK_IMAGE(priv->icon), pixbuf);
    //gtk_image_set_from_file(GTK_IMAGE(priv->icon), szFilePath);
}

void 
grmkit_button_set_index (GrmkitButton *button, int index)
{
    GrmkitButtonPrivate *priv = button->priv;
    priv->index = index;
}

int
grmkit_button_get_index (GrmkitButton *button)
{
    GrmkitButtonPrivate *priv = button->priv;

    return priv->index;
}

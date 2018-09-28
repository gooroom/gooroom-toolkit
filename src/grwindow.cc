/* grwindow.cc
 *
 * Copyright (C) 2015-2017 Gooroom <gooroom@gooroom.kr>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <apt-pkg/fileutl.h>
#include "config.h"
#include <glib/gi18n.h>
#include "grwindow.h"
#include "grutils.h"
#include <iostream>

GRWindow::GRWindow(GRWindow *parent, string strName)
{
    char *filename = NULL;
    char *main_widget = NULL;

    filename = g_strdup_printf("/kr/gooroom/toolkit/window_%s.ui", strName.c_str());

    _builder = gtk_builder_new_from_resource (filename);
    if (!_builder)
    {
        g_warning ("Couldn't load builder file: %s", filename);
        g_free (filename);
        return;
    }

    main_widget = g_strdup_printf("window_%s", strName.c_str());
    _win = GTK_WIDGET (gtk_builder_get_object (_builder, main_widget));

    gtk_window_set_title(GTK_WINDOW(_win), strName.c_str());

    g_object_set_data(G_OBJECT(_win), "me", this);

    g_signal_connect(G_OBJECT(_win), "delete-event", G_CALLBACK(windowCloseCallback), this);

    gtk_widget_realize(_win);

    if(parent != NULL)
        gtk_window_set_transient_for(GTK_WINDOW(_win), GTK_WINDOW(parent->window()));

    gtk_window_set_position(GTK_WINDOW(_win), GTK_WIN_POS_CENTER_ON_PARENT);

    g_free(filename);
    g_free(main_widget);
}

GRWindow::~GRWindow()
{
#ifdef DEBUG_MSG
    cout<< "~GRWindow" << endl;
#endif
    g_object_unref (_builder);
    gtk_widget_destroy(_win);
}

void
GRWindow::setTitle(string strTitle)
{
    gtk_window_set_title(GTK_WINDOW(_win), strTitle.c_str());
}

bool
GRWindow::close()
{
#ifdef DEBUG_MSG
    cout<< "GRWindow::close()" << endl;
#endif
    hide();
    return true;
}

bool
GRWindow::windowCloseCallback(GtkWidget *window, GdkEvent * event)
{

#ifdef DEBUG_MSG
    cout << "windowCloseCallback" << endl;
#endif

    GRWindow *rwin = (GRWindow *) g_object_get_data(G_OBJECT(window), "me");

    return rwin->close();
}

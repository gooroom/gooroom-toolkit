/* gruserdialog.cc
 *
 * Copyright (C) 2015-2017 Gooroom <gooroom@gooroom.kr>
 * Copyright (c) 2000, 2001 Conectiva S/A
 *               2003 Michael Vogt
 *
 * Author: Alfredo K. Kojima <kojima@conectiva.com.br>
 *         Michael Vogt <mvo@debian.org>
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

#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>
#include <apt-pkg/fileutl.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <assert.h>
#include <string>
#include <glib/gi18n.h>

#include "gruserdialog.h"
#include "grutils.h"

static void
actionResponse(GtkDialog *dialog, gint id, gpointer user_data)
{
    GtkResponseType *res = (GtkResponseType *) user_data;
    *res = (GtkResponseType) id;
}

bool
GRUserDialog::showErrors()
{
    if (_error->empty())
        return false;

    while (!_error->empty())
    {
        std::string message;
        bool iserror = _error->PopMessage(message);

        // Ignore some stupid error messages.
        if (message == "Tried to dequeue a fetching object")
            continue;

        if (!message.empty())
        {
            if (iserror)
                error(message.c_str());
            else
                warning(message.c_str());
        }
    }

    return true;
}

bool
GRUserDialog::message(const char *msg,
                      GRUserDialog::DialogType dialog,
                      GRUserDialog::ButtonsType buttons, bool defres)
{
    GtkWidget *dia;
    GtkResponseType res;
    GtkMessageType gtkmessage;
    GtkButtonsType gtkbuttons;

    switch (dialog)
    {
        case GRUserDialog::DialogInfo:
            gtkmessage = GTK_MESSAGE_INFO;
            gtkbuttons = GTK_BUTTONS_CLOSE;
            break;
        case GRUserDialog::DialogWarning:
            gtkmessage = GTK_MESSAGE_WARNING;
            gtkbuttons = GTK_BUTTONS_CLOSE;
            break;
        case GRUserDialog::DialogError:
            gtkmessage = GTK_MESSAGE_ERROR;
            gtkbuttons = GTK_BUTTONS_CLOSE;
            break;
        case GRUserDialog::DialogQuestion:
            gtkmessage = GTK_MESSAGE_QUESTION;
            gtkbuttons = GTK_BUTTONS_YES_NO;
            break;
    }

    switch (buttons)
    {
        case GRUserDialog::ButtonsDefault:
            break;
        case GRUserDialog::ButtonsOk:
            gtkbuttons = GTK_BUTTONS_CLOSE;
            break;
        case GRUserDialog::ButtonsOkCancel:
            gtkbuttons = GTK_BUTTONS_OK_CANCEL;
            break;
        case GRUserDialog::ButtonsYesNo:
            gtkbuttons = GTK_BUTTONS_YES_NO;
            break;
    }

    dia = gtk_message_dialog_new (GTK_WINDOW(_parentWindow),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  gtkmessage, gtkbuttons, NULL);

    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG(dia), utf8(msg));
    gtk_container_set_border_width(GTK_CONTAINER(dia), 6);

    if (defres)
    {
        switch (buttons)
        {
            case GRUserDialog::ButtonsOkCancel:
                gtk_dialog_set_default_response(GTK_DIALOG(dia), GTK_RESPONSE_OK);
                break;
            case GRUserDialog::ButtonsYesNo:
                gtk_dialog_set_default_response(GTK_DIALOG(dia), GTK_RESPONSE_YES);
                break;
        }
    }
    else
    {
        switch (buttons)
        {
            case GRUserDialog::ButtonsOkCancel:
                gtk_dialog_set_default_response(GTK_DIALOG(dia), GTK_RESPONSE_CANCEL);
                break;
            case GRUserDialog::ButtonsYesNo:
                gtk_dialog_set_default_response(GTK_DIALOG(dia), GTK_RESPONSE_NO);
                break;
        }
    }

    g_signal_connect(G_OBJECT(dia), "response", G_CALLBACK(actionResponse), (gpointer) & res);
    
    // honor foreign parent windows (to make embedding easy)
    int id = _config->FindI("Volatile::ParentWindowId", -1);
    if (id > 0)
    {
        GdkWindow *win = gdk_x11_window_foreign_new_for_display(gdk_display_get_default(), id);
        if(win)
        {
            gtk_widget_realize(dia);
            gdk_window_set_transient_for(GDK_WINDOW(gtk_widget_get_window(dia)), win);
        }
    }

    gtk_dialog_run(GTK_DIALOG(dia));
    gtk_widget_destroy(dia);

    return (res == GTK_RESPONSE_OK) || (res == GTK_RESPONSE_YES) || (res == GTK_RESPONSE_CLOSE);
}

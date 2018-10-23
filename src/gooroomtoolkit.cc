/* gooroomtoolkit.cc
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

#include <apt-pkg/init.h>
#include "config.h"

#include "gooroomtoolkit.h"
#include "grmainwindow.h"
#include "grutils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

static GtkWidget *window;

static void
on_app_startup_cb (GtkApplication *app, gpointer data)
{
    string strConfigDir = PACKAGE_CONFIGDIR;
    string strInfoFile = strConfigDir + "toolpackages.json";

    GRMainWindow *mainWindow = new GRMainWindow(strInfoFile.c_str());
    mainWindow->setApplication(app);
    window = mainWindow->window();
    gtk_application_add_window(app, GTK_WINDOW(window));

    RGFlushInterface();
}

static void
on_app_activate_cb (GtkApplication *app)
{
    gtk_window_present (GTK_WINDOW (window));
}

int
main(int argc, char *argv[])
{
    int status;
    GtkApplication *app;

    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    if (getuid () != 0) {
        GtkWidget *message = gtk_message_dialog_new (NULL,
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_OK,
                                                     _("Exit Program"));

        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (message),
                                 _("Root privileges are required for running this program."));

        gtk_window_set_title (GTK_WINDOW (message), _("Notification"));

        gtk_dialog_run (GTK_DIALOG (message));
        gtk_widget_destroy (message);
        return -1;
    }

    app = gtk_application_new ("kr.gooroom.gooroom-toolkit", G_APPLICATION_FLAGS_NONE);

    g_signal_connect (app, "activate", G_CALLBACK (on_app_activate_cb), NULL);
    g_signal_connect (app, "startup", G_CALLBACK (on_app_startup_cb), NULL);

    status = g_application_run (G_APPLICATION (app), argc, argv);

    g_object_unref (app);

    return status;
}

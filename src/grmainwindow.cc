/* grmainwindow.cc
 * 
 * Copyright (C) 2018-2019 Gooroom <gooroom@gooroom.kr>
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

#include <cassert>
#include <string>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include "stdio.h"
#include <glib/gi18n.h>
#include "grutils.h"
#include "grmainwindow.h"
#include "grpackage.h"
#include "gruserdialog.h"
#include "grmkit-button.h"

#include <gdk/gdkx.h>

#define GDK_WINDOW_XWINDOW(win)	(gdk_x11_window_get_xid (win))

GRMainWindow::GRMainWindow(const char* jsonFile)
   : GRWindow(NULL, "main"), _toolManager(NULL)
{
    assert(_win);

    _toolManager = new GRPackageManager(this);
    _toolManager->initPackage(jsonFile);
    _userDialog = new GRUserDialog(_win);

    //window
    gtk_window_set_title (GTK_WINDOW (_win), _("Gooroom Toolkit"));
    gtk_window_set_default_size (GTK_WINDOW (_win), 230, 196);

    _mainBox = GTK_WIDGET (gtk_builder_get_object (_builder, "box_main"));
    assert(_mainBox);

    vector<GRPackage*> packages = _toolManager->getPackages();

    int index = 0;
    for (vector <GRPackage*>::iterator I = packages.begin(); I != packages.end(); I++)
    {
        GRPackage* package =  (GRPackage*)(*I);
        package->setIndex(index);
        GtkWidget* button = setPackage(package);
        gtk_box_pack_start (GTK_BOX(_mainBox), button, false, false, 0);
        g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(btnImageClicked), package);
        index++;
    }

    _closeButton = GTK_WIDGET (gtk_builder_get_object (_builder, "close_button"));
    g_signal_connect (G_OBJECT(_closeButton), "clicked", G_CALLBACK(btnCloseClicked), this);

    gtk_widget_show_all(_mainBox);
}

GRMainWindow::~GRMainWindow()
{
#ifdef DEBUG_MSG
    cout << "GRMainWindow::~GRMainWindow()" << endl;
#endif

    close();
}

bool
GRMainWindow::close()
{
#ifdef DEBUG_MSG
    cout << "GRMainWindow::close()" << endl;
#endif

    if (_toolManager)
        delete _toolManager;

    if (_userDialog)
        delete _userDialog;

    g_application_quit(G_APPLICATION(_app));
    return true;
}

GtkWidget*
GRMainWindow::setPackage(GRPackage* package)
{
    string strName = package->name();
    string strDesc = package->desc();

    GrmkitButton* button = grmkit_button_new();
    grmkit_button_set_description (GRMKIT_BUTTON(button), strDesc.c_str());
    grmkit_button_set_index (GRMKIT_BUTTON(button), package->getIndex());

    string strImagePath = package->icon();
    if (strImagePath.empty())
    {
        strImagePath = package->name();
        strImagePath += "_tool";
    }

    grmkit_button_set_icon_from_file (GRMKIT_BUTTON(button), strImagePath.c_str());

    //Button Enable
    string strVersion = package->version();
    string strFormat = package->format();

    gchar* title;
    bool isDisabled = _toolManager->isInstallPackage(strName, strVersion, strFormat);
    if (isDisabled)
    {
        gtk_widget_set_sensitive(GTK_WIDGET(button), false);
        title = g_strdup_printf ("%s (%s)", _(strName.c_str()), _("Installed"));
    }
    else
    {
        title = g_strdup_printf ("%s", strName.c_str());
    }

    grmkit_button_set_title (GRMKIT_BUTTON(button), title);
    g_object_set_data(G_OBJECT(button), "me", this);
    g_free (title);

    return GTK_WIDGET(button);
}

void
GRMainWindow::btnImageClicked(GtkWidget* self, void *data)
{
    GRPackage* package = (GRPackage*)data;
    string strExec = package->exec();
    char szMsg[512];
    snprintf(szMsg, sizeof(szMsg), _("Do you want to <b>%s</b> install?"), _(strExec.c_str()));

    GRMainWindow* me = (GRMainWindow*)g_object_get_data(G_OBJECT(self), "me");
    assert(me);

    if (!me->getUserDialog()->confirm(szMsg))
        return;

    RGFlushInterface();

    GtkWidget* closeButton = me->_closeButton;
    gtk_widget_set_sensitive (closeButton, false);

    string strFormat = package->format();

    if (strFormat.compare("package") == 0)
    {
        GtkWidget* main_window = me->window();
        GdkWindow* g_window = gtk_widget_get_window(GTK_WIDGET(main_window));

        string strPackage = strExec + "\tinstall\n";
        const char* szPackage = utf8(strPackage.c_str());

        char buff [L_tmpnam];
        tmpnam(buff);

        FILE* tmp;
        tmp = fopen(buff, "wx");
        fputs(szPackage, tmp);
        rewind(tmp);

        fclose(tmp);

        char szCmd[512];
        snprintf(szCmd, sizeof(szCmd), "'pkexec' '/usr/sbin/synaptic' '--hide-main-window' '--non-interactive' '--parent-window-id' '%u' '-o' 'Synaptic::closeZvg=false' '--set-selections-file' '%s'", GDK_WINDOW_XWINDOW(g_window), buff);

        system(szCmd);

        gtk_widget_set_sensitive (GTK_WIDGET(closeButton), true);

        string strName = package->name();
        gchar* gProgram = g_find_program_in_path (strName.c_str());
        if (gProgram == NULL)
        {
            char* szFailed = _("Failed to install package");
            me->getUserDialog()->error(szFailed);
            RGFlushInterface();
            return;
        }

        const gchar* title = grmkit_button_get_title (GRMKIT_BUTTON(self));
        gchar* update_title = g_strdup_printf ("%s (%s)", title, _("Installed"));
        grmkit_button_set_title (GRMKIT_BUTTON(self), update_title);
        g_free(update_title);
        gtk_widget_set_sensitive (GTK_WIDGET(self), false);
    }
    else
    {
        me->getManager()->startDownload(package->getIndex());
        me->getManager()->showError();
    }

    RGFlushInterface();
}

void
GRMainWindow::updateWindow(int index)
{
    GList *list = gtk_container_get_children (GTK_CONTAINER(_mainBox));
    for (GList *li = g_list_first(list); li != NULL; li = g_list_next(li))
    {
        GrmkitButton* button = (GrmkitButton*)li->data;
        int btnIndex = grmkit_button_get_index(button);
        if (btnIndex == index)
        {
            gtk_widget_set_sensitive (GTK_WIDGET(button), false);
            const gchar* title = grmkit_button_get_title (GRMKIT_BUTTON(button));
            gchar* update_title = g_strdup_printf ("%s (%s)", title, _("Installed"));

            grmkit_button_set_title (GRMKIT_BUTTON(button), update_title);
            g_free(update_title);
        }
    }

    gtk_widget_set_sensitive (GTK_WIDGET(_closeButton), true);
}

void
GRMainWindow::btnCloseClicked(GtkWidget* self, void *data)
{
    GRMainWindow* main = (GRMainWindow*)data;
    main->close();
}

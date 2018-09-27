/* grmainwindow.cc
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
//
GRMainWindow::GRMainWindow(const char* jsonFile)
   : GRWindow(NULL, "main"), _toolManager(NULL)
{
    assert(_win);

    _toolManager = new GRPackageManager(this);
    _toolManager->initPackage(jsonFile);
    _userDialog = new GRUserDialog(_win);

    //window
    gtk_window_set_title (GTK_WINDOW (_win), _("Gooroom Toolkit"));
    gtk_window_set_default_size (GTK_WINDOW (_win), 230, 350);

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
        g_signal_connect(G_OBJECT(button), "clicked", (GCallback)btnImageClicked, package);
        index++;
    }

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
    
    gtk_main_quit();

    return true;
}

GtkWidget* 
GRMainWindow::setPackage(GRPackage* package)
{
    string strName = package->name();
    string strDesc = package->desc();

    GrmkitButton* button = grmkit_button_new();
    grmkit_button_set_title (GRMKIT_BUTTON(button), strName.c_str());
    grmkit_button_set_description (GRMKIT_BUTTON(button), strDesc.c_str());
    grmkit_button_set_index (GRMKIT_BUTTON(button), package->getIndex());

    string strImagePath = package->icon();
    if (strImagePath.empty())
        strImagePath = package->name();
    
    grmkit_button_set_icon_from_file (GRMKIT_BUTTON(button), strImagePath.c_str());

    //Button Enable
    string strPackageName = package->name();
    string strPackageVer = package->version();

    bool isDisabled = _toolManager->checkVersion(strPackageName, strPackageVer);
    if (isDisabled)
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

    g_object_set_data(G_OBJECT(button), "me", this);

    return GTK_WIDGET(button);
}

void
GRMainWindow::btnImageClicked(GtkWidget* self, void *data)
{
    GRPackage* package = (GRPackage*)data;
    string strName = package->name();
    char szMsg[512];
    snprintf(szMsg, sizeof(szMsg), _("Do you want to <b>%s</b> install?"), _(strName.c_str()));
    
    GRMainWindow* me = (GRMainWindow*)g_object_get_data(G_OBJECT(self), "me");
    assert(me);
    
    if (!me->getUserDialog()->confirm(szMsg))
        return;

    me->getManager()->startDownload(package->getIndex());
    me->getManager()->showError();

    RGFlushInterface();
}

void
GRMainWindow::updateWindow(int index)
{
    //TODO 개선 필요...
    cout << "UpdateWindow" << index << endl;
    GList *list = gtk_container_get_children(GTK_CONTAINER(_mainBox));
    for (GList *li = g_list_first(list); li != NULL; li = g_list_next(li))
    {
        GrmkitButton* button = (GrmkitButton*)li->data;
        int btnIndex = grmkit_button_get_index(button);
        if (btnIndex == index)
        {
            gtk_widget_set_sensitive(GTK_WIDGET(button), false);
        }
    }
}


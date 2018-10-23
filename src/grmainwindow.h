/* grmainwindow.h
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

#ifndef _GRMAINWINDOW_H_
#define _GRMAINWINDOW_H_

#include "config.h"

#include <gtk/gtk.h>

#include "grwindow.h"
#include "grpackagemanager.h"
#include "grpackage.h"
#include "gruserdialog.h"

using namespace std;
class GRPackageManager;

class GRMainWindow : public GRWindow {
    enum
    {
        TOOLPACKAGE_NAME = 0,
        TOOLPACKAGE_URL,
        NUM_COLUMNS
    };
    public:
        GtkWidget* _mainBox;
        GtkWidget* _closeButton;

    private:
        GtkApplication* _app;

        GRPackageManager* _toolManager;
        GRUserDialog* _userDialog;

    public:
        GRMainWindow(const char* jsonFile);
        virtual ~GRMainWindow();
        virtual bool close();

        void updateWindow(int index);

        GRPackageManager* getManager() { return _toolManager; }
        GRUserDialog* getUserDialog() {return _userDialog; }

        GtkWidget* setPackage(GRPackage* package);
        void setApplication(GtkApplication *app) { _app = app; }
        GtkApplication* getApplication() { return _app; }

        static void btnImageClicked(GtkWidget *self, void *data);
        static void btnCloseClicked(GtkWidget *self, void *data);
};

#endif

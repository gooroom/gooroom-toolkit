/* grfetchprogress.h
 *
 * Copyright (c) 2000, 2001 Conectiva S/A
 * Copyright (C) 2018-2019 Gooroom <gooroom@gooroom.kr>
 *
 * Author: Alfredo K. Kojima <kojima@conectiva.com.br>
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


#ifndef _GRFETCHPROGRESS_H_
#define _GRFETCHPROGRESS_H_

#include <apt-pkg/acquire.h>

#include <vector>
#include <set>

#include "grwindow.h"

class GRFetchProgress : public pkgAcquireStatus, public GRWindow {

    struct Item {
       string descr;
       string uri;
       string size;
       int status;
    };

    vector<Item> _items;

    GtkWidget *_table;
    GtkListStore *_tableListStore;
    set<int> _tableRows;

    GtkWidget *_mainProgressBar; // GtkProgressBar

    GtkWidget *_sock;

    //PangoLayout *_layout;
    GtkTreeViewColumn *_statusColumn;
    GtkCellRenderer *_statusRenderer;
    bool _cancelled;

    void updateStatus(pkgAcquire::ItemDesc & Itm, int status);
    static void stopDownload(GtkWidget *self, void *data);

    static void cursorChanged(GtkTreeView *treeview, gpointer user_data);
    static void expanderActivate(GObject *object, GParamSpec *param_spec, gpointer user_data);
    bool _cursorDirty;

    char *getStatusStr(int status);
    int getStatusPercent(int status);
    void refreshTable(int row, bool append = false);
    //GdkPixmap *statusDraw(int width, int height, int status);

public:
    virtual bool MediaChange(string Media, string Drive);
    virtual void IMSHit(pkgAcquire::ItemDesc &Itm);
    virtual void Fetch(pkgAcquire::ItemDesc &Itm);
    virtual void Done(pkgAcquire::ItemDesc &Itm);
    virtual void Fail(pkgAcquire::ItemDesc &Itm);
    virtual void Start();
    virtual void Stop();
    virtual bool close();

    bool Pulse(pkgAcquire * Owner);
    bool IsCancelled() { return _cancelled; }
    // set description of the current task (main and additonal explaination)
    void setDescription(string mainText, string secondText="");

    GRFetchProgress(GRWindow *win);
};

#endif

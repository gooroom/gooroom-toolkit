/* grfetchprogress.cc
 *
 * Copyright (c) 2000, 2001 Conectiva S/A
 *               2002, 2003 Michael Vogt <mvo@debian.org>
 * Copyright (C) 2018-2019 Gooroom <gooroom@gooroom.kr>
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

#include "config.h"

#include <math.h>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/acquire-worker.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>
#include <apt-pkg/strutl.h>

#include "grfetchprogress.h"
#include "gruserdialog.h"
#include "grutils.h"

#include <stdio.h>
#include <gtk/gtk.h>
#include <cassert>

#include <glib/gi18n.h>

enum {
    DLDone = -1,
    DLQueued = -2,
    DLFailed = -3,
    DLHit = -4
};

enum {
    FETCH_PROGRESS_COLUMN,
    FETCH_PROGRESS_TEXT_COLUMN,
    FETCH_SIZE_COLUMN,
    FETCH_DESCR_COLUMN,
    FETCH_URL_COLUMN,
    N_FETCH_COLUMNS
};

static const int COLUMN_PERCENT_WIDTH=100;
static const int COLUMN_PERCENT_HEIGHT=18;


bool
GRFetchProgress::close()
{
    stopDownload(NULL, this);

    return TRUE;
}

GRFetchProgress::GRFetchProgress(GRWindow *main)
   : GRWindow(main, "fetch"), _cursorDirty(0), _sock(0)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    _mainProgressBar = GTK_WIDGET(gtk_builder_get_object(_builder, "progressbar_download"));
    assert(_mainProgressBar);

    _table = GTK_WIDGET(gtk_builder_get_object(_builder, "treeview_fetch"));
    _tableListStore = gtk_list_store_new(N_FETCH_COLUMNS,
                                         G_TYPE_INT,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING,
                                         G_TYPE_STRING);

    gtk_tree_view_set_model(GTK_TREE_VIEW(_table), GTK_TREE_MODEL(_tableListStore));

    /* percent column */
    renderer = gtk_cell_renderer_progress_new();
    column = gtk_tree_view_column_new_with_attributes(_("Status"), renderer,
                                             "value", FETCH_PROGRESS_COLUMN,
                                             "text", FETCH_PROGRESS_TEXT_COLUMN,
                                              NULL);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(column, COLUMN_PERCENT_WIDTH);
    _statusColumn = column;
    _statusRenderer = renderer;
    gtk_tree_view_append_column(GTK_TREE_VIEW(_table), column);

    /* size */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Size"), renderer,
                                                      "text", FETCH_SIZE_COLUMN,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(_table), column);

    /* descr */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("Package"), renderer,
                                                     "text",FETCH_DESCR_COLUMN,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(_table), column);

    /* url */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(_("URI"), renderer,
                                                     "text", FETCH_URL_COLUMN,
                                                     NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(_table), column);

    g_signal_connect(GTK_WIDGET(gtk_builder_get_object(_builder, "button_cancel")),
                                                     "clicked",
                                                     G_CALLBACK(stopDownload), this);
    gtk_widget_realize(_win);
    gtk_window_set_urgency_hint(GTK_WINDOW(_win), FALSE);
    // emit a signal if the user changed the cursor
    g_signal_connect(G_OBJECT(_table), "cursor-changed",  G_CALLBACK(cursorChanged), this);
    // emit a signal if the user changed the cursor
    GtkWidget *expander = GTK_WIDGET(gtk_builder_get_object(_builder, "expander"));
    assert(expander);
    g_signal_connect (expander, "notify::expanded", G_CALLBACK (expanderActivate), this);

    setTitle(_("Download"));
}

void
GRFetchProgress::expanderActivate(GObject    *object,
                                  GParamSpec *param_spec,
                                  gpointer    data)
{
    GtkExpander *expander = GTK_EXPANDER (object);
    GRFetchProgress *me = (GRFetchProgress*)data;

    GtkWidget *win = GTK_WIDGET(gtk_builder_get_object(me->_builder, "window_fetch"));
    if (gtk_expander_get_expanded (expander))
        gtk_window_set_resizable(GTK_WINDOW(win),TRUE);
    else
        gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
}


void
GRFetchProgress::cursorChanged(GtkTreeView *self, void *data)
{
    GRFetchProgress *me = (GRFetchProgress *)data;
    me->_cursorDirty=true;
}

void
GRFetchProgress::setDescription(string mainText, string secondText)
{
    gchar *str;
    gtk_window_set_title(GTK_WINDOW(_win), mainText.c_str());
    if(secondText.empty())
        str = g_strdup_printf("<big><b>%s</b></big>",mainText.c_str());
    else
        str = g_strdup_printf("<big><b>%s</b></big> \n\n%s",mainText.c_str(), secondText.c_str());

    GObject *label_desc = gtk_builder_get_object(_builder, "label_description");
    gtk_label_set_markup(GTK_LABEL(GTK_WIDGET(label_desc)), str);

    g_free(str);
}

bool
GRFetchProgress::MediaChange(string Media, string Drive)
{
#ifdef DEBUG_MSG
    cout << "GRFetchProgress::MediaChange" <<endl;
#endif
    gchar * msg;

    msg = g_strdup_printf(_("Please insert the disk labeled:\n%s\nin drive %s"), Media.c_str(), Drive.c_str());

    GRUserDialog userDialog(this);
    _cancelled = !userDialog.proceed(msg);

    RGFlushInterface();
    g_free(msg);
    return true;
}

void
GRFetchProgress::updateStatus(pkgAcquire::ItemDesc & Itm, int status)
{
    if (Itm.Owner->ID == 0)
    {
        Item item;
        item.descr = Itm.ShortDesc;
        item.uri = Itm.Description;
        item.size = string(SizeToStr(Itm.Owner->FileSize));
        item.status = status;
        _items.push_back(item);
        Itm.Owner->ID = _items.size();
        refreshTable(Itm.Owner->ID - 1, true);
    }
    else if (_items[Itm.Owner->ID - 1].status != status)
    {
        _items[Itm.Owner->ID - 1].status = status;
        refreshTable(Itm.Owner->ID - 1, false);
    }
}

void
GRFetchProgress::IMSHit(pkgAcquire::ItemDesc & Itm)
{
    updateStatus(Itm, DLHit);
    RGFlushInterface();
}

void
GRFetchProgress::Fetch(pkgAcquire::ItemDesc & Itm)
{
    updateStatus(Itm, DLQueued);
    RGFlushInterface();
}

void
GRFetchProgress::Done(pkgAcquire::ItemDesc &Itm)
{
    updateStatus(Itm, DLDone);
    RGFlushInterface();
}

void
GRFetchProgress::Fail(pkgAcquire::ItemDesc &Itm)
{
    if (Itm.Owner->Status == pkgAcquire::Item::StatIdle)
        return;

    updateStatus(Itm, DLFailed);
    RGFlushInterface();
}

bool
GRFetchProgress::Pulse(pkgAcquire *Owner)
{
    pkgAcquireStatus::Pulse(Owner);

    // only show here if there is actually something to download/get
    if (TotalBytes > 0 && !gtk_widget_get_visible(_win))
        show();

    float percent = long (double ((CurrentBytes + CurrentItems) * 100.0) / double (TotalBytes + TotalItems));

    // work-around a stupid problem with libapt
    if(CurrentItems == TotalItems)
        percent=100.0;

    // only do something if there is some real progress
    if (fabsf(percent- gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(_mainProgressBar))*100.0) < 0.1)
    {
        RGFlushInterface();
        return !_cancelled;
    }

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(_mainProgressBar), percent / 100.0);

    for (pkgAcquire::Worker * I = Owner->WorkersBegin(); I != 0; I = Owner->WorkerStep(I))
    {
        if (I->CurrentItem == 0)
            continue;

        if (I->TotalSize > 0)
            updateStatus(*I->CurrentItem, long (double (I->CurrentSize * 100.0) / double (I->TotalSize)));
        else
            updateStatus(*I->CurrentItem, 100);
    }

    unsigned long ETA;
    if (CurrentCPS > 0)
        ETA = (unsigned long)((TotalBytes - CurrentBytes) / CurrentCPS);
    else
        ETA = 0;

    // if the ETA is greater than two weeks, show unknown time
    if (ETA > 14*24*60*60)
        ETA = 0;
    long i = CurrentItems < TotalItems ? CurrentItems + 1 : CurrentItems;
    gchar *s;
    GObject *label_eta = gtk_builder_get_object(_builder, "label_eta");
    if (CurrentCPS != 0 && ETA != 0)
    {
        s = g_strdup_printf(_("Download rate: %s/s - %s remaining"),
                            SizeToStr(CurrentCPS).c_str(),
                            TimeToStr(ETA).c_str());
        gtk_label_set_text(GTK_LABEL(GTK_WIDGET(label_eta)),s);
        g_free(s);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(GTK_WIDGET(label_eta)),_("Download rate: ..."));
    }

    s = g_strdup_printf(_("Downloading file %li of %li"), i, TotalItems);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(_mainProgressBar), s);
    g_free(s);

    RGFlushInterface();

    return !_cancelled;
}

void
GRFetchProgress::Start()
{
   pkgAcquireStatus::Start();
   _cancelled = false;

   RGFlushInterface();
}

void
GRFetchProgress::Stop()
{
    RGFlushInterface();
    if(_sock != NULL)
    {
        gtk_widget_destroy(_sock);
    } else
    {
        hide();
    }
    pkgAcquireStatus::Stop();

    //FIXME: this needs to be handled in a better way (gtk-2 maybe?)
    sleep(1);                    // this sucks, but if ommited, the window will not always
    // closed (e.g. when a package is only deleted)
    RGFlushInterface();
}

void
GRFetchProgress::stopDownload(GtkWidget *self, void *data)
{
    GRFetchProgress *me = (GRFetchProgress *) data;
    me->_cancelled = true;
}

char*
GRFetchProgress::getStatusStr(int status)
{
    // special status
    if (status < 0)
    {
        switch (status)
        {
            case DLQueued:
                return _("Queued");
                break;
            case DLDone:
                return _("Done");
                break;
            case DLHit:
                return _("Hit");
                break;
            case DLFailed:
                return _("Failed");
                break;
        }
    }
    // default label
    return NULL;
}

int
GRFetchProgress::getStatusPercent(int status)
{
    // special status
    if (status < 0)
    {
        // on done/hit show full status, otherwise empty
        if (status == DLDone || status == DLHit)
            return 100;
        else
            return 0;
    }

    return status;
}

void
GRFetchProgress::refreshTable(int row, bool append)
{
    GtkTreeIter iter;
    GtkTreePath *path;

    // find the right iter
    if (append == true)
    {
        gtk_list_store_insert(_tableListStore, &iter, row);

        if(!_cursorDirty)
        {
            path = gtk_tree_path_new();
            gtk_tree_path_append_index(path, row);
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(_table),
                                         path, NULL, TRUE, 0.0, 0.0);
            gtk_tree_path_free(path);
        }
    }
    else
    {
       gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(_tableListStore),
                                     &iter, NULL, row);
    }

    // set the data
    gtk_list_store_set(_tableListStore, &iter,
                       FETCH_PROGRESS_COLUMN, getStatusPercent(_items[row].status),
                       FETCH_PROGRESS_TEXT_COLUMN, getStatusStr(_items[row].status),
                       FETCH_SIZE_COLUMN, _items[row].size.c_str(),
                       FETCH_DESCR_COLUMN, _items[row].descr.c_str(),
                       FETCH_URL_COLUMN, _items[row].uri.c_str(), -1);
    path = gtk_tree_model_get_path(GTK_TREE_MODEL(_tableListStore), &iter);
    if(!_cursorDirty)
        gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(_table), path, NULL, TRUE, 0.0, 0.0);
    gtk_tree_path_free(path);
}

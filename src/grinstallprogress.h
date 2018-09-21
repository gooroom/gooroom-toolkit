/* grinstallprogress.h
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


#ifndef _GRINSTALLPROGRESS_H_
#define _GRINSTALLPROGRESS_H_

#include "config.h"
#include <vte/vte.h>

#include "grdefault.h"
#include "gruserdialog.h"
#include "grwindow.h"
//#include "grbuilderwindow.h"

using namespace Result;

class GRMainWindow;

class GRInstallProgress: public GRWindow 
{
    typedef enum {
        EDIT_COPY,
  	EDIT_SELECT_ALL,
  	EDIT_SELECT_NONE,
    } TermAction;
    
    // widgets
    GtkWidget *_label_status;
    GtkWidget *_labelSummary;
    GtkWidget *_pbarTotal;
    GtkWidget *_term; 
    GtkWidget *_popupMenu; // Popup menu of the terminal
    
    GtkWidget *_sock; // if we run embedded
    GtkCssProvider *_cssProvider;
    GRUserDialog *_userDialog;
    
    OrderResult _systemRes;
    
    bool child_has_exited;
    bool _startCounting;
    bool _updateFinished;
    bool _autoClose;
    
    int _childin;
    int _progress;
    int _totalActions;
    int _terminalTimeout; // when the internal terminal timesout after no activity
    
    pid_t _child_id;
    time_t last_term_action; // last time something changed
    
    static void child_exited(VteTerminal *vteterminal, gint ret, gpointer data);
    static void terminalAction(GtkWidget *terminal, TermAction action); 

protected:
    void startUpdate();
    void updateInterface();
    void finishUpdate();
    virtual bool close();
    
    // gtk stuff
    static void cbCancel(GtkWidget *self, void *data);
    static void cbClose(GtkWidget *self, void *data);
    static void content_changed(GObject *object, gpointer    user_data);
    static void expander_callback(GObject *object,GParamSpec *param_spec, gpointer user_data);
    
    static gboolean cbTerminalClicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
    static void cbMenuitemClicked(GtkMenuItem *menuitem, gpointer user_data);

public:
    GRInstallProgress(GRMainWindow *main, bool autoClose = false);
    virtual ~GRInstallProgress();
    
    void start(string strExecCommand, string strFormat);
    void start(string strSrc, string strFormat, string strVersion, string strDownloadSrc); 
    OrderResult getResultCode () { return _systemRes; }
};

#endif

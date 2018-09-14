/* gruserdialog.h
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

#ifndef _GRUSERDIALOG_H_
#define _GRUSERDIALOG_H_

#include "grwindow.h"

class GRUserDialog
{
public:
    enum ButtonsType {
       ButtonsDefault,
       ButtonsOk,
       ButtonsOkCancel,
       ButtonsYesNo
    };
    
    enum DialogType {
       DialogInfo,
       DialogWarning,
       DialogQuestion,
       DialogError
    };

protected:
    GtkWidget *_parentWindow;

public:
    GRUserDialog() : _parentWindow(0) {};
    GRUserDialog(GRWindow *parent) : _parentWindow(parent->window()) {};
    GRUserDialog(GtkWidget *parent) : _parentWindow(parent) {};
    
    bool showErrors();

    bool confirm(const char *msg, bool defres = true) {
        return message(msg, DialogQuestion, ButtonsYesNo, defres);
    };

    bool proceed(const char *msg, bool defres = true) {
        return message(msg, DialogInfo, ButtonsOkCancel, defres);
    };

    bool warning(const char *msg, bool nocancel = true) {
        return nocancel ? message(msg, DialogWarning)
            : message(msg, DialogWarning, ButtonsOkCancel, false);
    };

    void error(const char *msg) {
        message(msg, DialogInfo);
    };         

    void message_ex(const char *msg) {
        message(msg, DialogError);
    };           

private:
    bool message(const char *msg,
	    GRUserDialog::DialogType dialog=GRUserDialog::DialogInfo,
	    GRUserDialog::ButtonsType buttons=GRUserDialog::ButtonsOk,
	    bool defres=true);  
};

#endif

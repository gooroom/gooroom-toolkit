/* grwindow.h
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


#ifndef _GRWINDOW_H_
#define _GRWINDOW_H_

#include <gtk/gtk.h>
#include <string>

using namespace std;

class GRWindow {
  public:
    GRWindow(GRWindow *parent, string strName);
    ~GRWindow();

  protected:
    GtkWidget *_win;
    GtkBuilder *_builder;

    virtual bool close();
    virtual void setTitle(string strTitle);
    static bool windowCloseCallback(GtkWidget *widget, GdkEvent * event);

  public:
    inline virtual GtkWidget *window() { return _win;  };
    inline virtual void hide() { gtk_widget_hide(_win); };
    inline virtual void show() { gtk_widget_show(_win); };

    GtkBuilder* getGtkBuilder() {return _builder;};   
};
#endif

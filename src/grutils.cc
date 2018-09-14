/* grutil.cc 
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

#include <apt-pkg/fileutl.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>

#include <glib/gi18n.h>
#include "grutils.h"

// helper
GdkPixbuf *
get_gdk_pixbuf(const gchar *name, int size)
{
    GtkIconTheme *theme;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    
    theme = gtk_icon_theme_get_default();
    pixbuf = gtk_icon_theme_load_icon(theme, name, size, (GtkIconLookupFlags)0, &error);

    if (pixbuf == NULL) 
        std::cerr << "Warning, failed to load: " << name << error->message << std::endl;
    
    return pixbuf;
}

GtkWidget *get_gtk_image(const gchar *name, int size)
{
    GdkPixbuf *buf;
    buf = get_gdk_pixbuf(name, size);

    if(!buf)
        return NULL;

    return gtk_image_new_from_pixbuf(buf);
}

void RGFlushInterface()
{
    XSync(GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), False);
    
    while (gtk_events_pending())
    {
        gtk_main_iteration();
    }
}

/*
 * SizeToStr: Converts a size long into a human-readable SI string
 * ----------------------------------------------------
 * A maximum of four digits are shown before conversion to the next highest
 * unit. The maximum length of the string will be five characters unless the
 * size is more than ten yottabytes.
 *
 * mvo: we use out own SizeToStr function as the SI spec says we need a 
 *      space between the number and the unit (this isn't the case in stock apt
 */
std::string SizeToStr(double Size)
{
    char S[300];
    double ASize;
    if (Size >= 0)
    {
        ASize = Size;
    } 
    else
    {
        ASize = -1 * Size;
    }
    
    /* Bytes, kilobytes, megabytes, gigabytes, terabytes, petabytes, exabytes,
     * zettabytes, yottabytes.
     */
    char Ext[] = { ' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y' };
    int I = 0;
    while (I <= 8)
    {
        if (ASize < 100 && I != 0) 
	{
            snprintf(S, 300, "%.1f %cB", ASize, Ext[I]);
            break;
        }
        
        if (ASize < 10000) 
	{
            snprintf(S, 300, "%.0f %cB", ASize, Ext[I]);
           break;
        }
        ASize /= 1000.0;
        I++;
    }
    return S;
}

const char *utf8_to_locale(const char *str)
{
    static char *_str = NULL;
    if (str == NULL)
        return NULL;
    if (g_utf8_validate(str, -1, NULL) == false)
        return NULL;
    g_free(_str);
    _str = NULL;
    _str = g_locale_from_utf8(str, -1, NULL, NULL, NULL);
    return _str;
}

const char *utf8(const char *str)
{
   static char *_str = NULL;
   if (str == NULL)
       return NULL;
   if (g_utf8_validate(str, -1, NULL) == true)
       return str;
   g_free(_str);
   _str = NULL;
   _str = g_locale_to_utf8(str, -1, NULL, NULL, NULL);
   return _str;
}

bool 
is_directory_exists( const char* path )
{
    if ( path == NULL) 
        return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir (path);

    if (pDir != NULL)
    {
        bExists = true;    
        (void) closedir (pDir);
    }

    return bExists;
}

bool 
create_directory(const char* path)
{
    if ( path == NULL) 
        return false;

    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    int ret = mkdir(path, mode);

    if (ret == 0)
    {
        return true;
    }

    string strPath = path;

    switch (errno)
    {
    case ENOENT:
        {
            int pos = strPath.find_last_of('/');
            if (pos == std::string::npos)
                return false;
            if (!create_directory(strPath.substr(0, pos).c_str()))
                return false;   

            int ret = mkdir(path, mode);
            std::cout << ret << std::endl;
            return (0 == ret) ? true : false;
        }            
    case EEXIST:    
        return is_directory_exists(path);

    }    
    return true;
}

bool 
delete_directory (const char* path)
{
    return false;
}

std::string 
get_find_extension (const char* path)
{
    string strFileName = path;

    if (strFileName.find_last_of(".") != std::string::npos)
        return strFileName.substr(strFileName.find_last_of(".") + 1);
        
    return "";
}

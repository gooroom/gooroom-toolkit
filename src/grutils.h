/* grutil.h
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

#ifndef _GRUTIL_H_
#define _GRUTIL_H_

#include <string>
#include <stdio.h>
#include <vector>
#include <dirent.h>

void RGFlushInterface();

const char *utf8_to_locale(const char *str);
const char *utf8(const char *str);

GtkWidget *get_gtk_image(const char *name, int size=16);
GdkPixbuf *get_gdk_pixbuf(const gchar *name, int size=16);

std::string SizeToStr(double Bytes);

bool is_directory_exists(const char* path);
bool create_directory(const char* path);
bool delete_directory(const char* path);
std::string get_find_extension(const char* path);

#endif

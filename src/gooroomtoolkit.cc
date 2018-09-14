/* gooroomtoolkit.cc
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

#include <apt-pkg/init.h>
#include "config.h"

#include "gooroomtoolkit.h"
#include "grmainwindow.h"
#include "grutils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

int main(int argc, char *argv[])
{
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
    
    gtk_init(&argc, &argv);
    
    if (!pkgInitConfig(*_config))
        return false;
    
    string strConfigDir = PACKAGE_VERDIR;
    string strInfoFile;
    if (argc == 2)
        strInfoFile = argv[1];    
    else               
        strInfoFile = strConfigDir + "toolpackages.json";     
    
    GRMainWindow *mainWindow = new GRMainWindow(strInfoFile.c_str());
    mainWindow->show();
    
    RGFlushInterface();
    
    gtk_main();
    return 0;
}

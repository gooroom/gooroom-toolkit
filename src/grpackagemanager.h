/* grpackagemanager.h
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

#ifndef _GRPACKAGEMANAGER_H_
#define _GRPACKAGEMANAGER_H_

#include "grdefault.h"
#include "grwindow.h"
#include "grparser.h"
#include "grpackage.h"
#include "grmainwindow.h"
#include "gruserdialog.h"

#include <apt-pkg/acquire-item.h>

#define pkgAcqFileSane pkgAcqFile

using namespace std;
using namespace Result;

class GRMainWindow;

class GRPackageManager {
public:
    GRPackageManager(GRMainWindow* win);
    ~GRPackageManager();

    //enum OrderResult {Completed,Failed,Incomplete, Cancel};

    void initPackage(string strFile);
    void startDownload(int index);
    void removePackage(int index);
    void removeAllPackage();
    void showError();
    bool isInstallPackage(string strPackageName, string strVersion, string strFormat);

    vector<GRPackage*> getPackages();

private:
    void addArchive(pkgAcquire *fetcher, GRPackage* package, string downloadPath);
    bool downloadPackage(GRPackage* package, bool checkPackage, bool autoClose = false);
    void downloadArchive(GRPackage* package);
    void removeDownloadFile(GRPackage* package, string strDownloadPath);

    OrderResult startScriptInstall(GRPackage* package);
    OrderResult startPackageInstall(GRPackage* package, bool autoClose);
private:
    GRMainWindow *_win;
    GRParser* _parser;
    GRUserDialog *_userDialog;
    OrderResult _res;
    vector<GRPackage*> _packages;
};

#endif

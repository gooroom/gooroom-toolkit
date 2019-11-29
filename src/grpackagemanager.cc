/* grpackagemanager.cc
 * 
 * Copyright (C) 2018-2019 Gooroom <gooroom@gooroom.kr>
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

#include <cassert>
#include <gtk/gtk.h>

#include "grutils.h"

#include "grpackagemanager.h"
#include "grfetchprogress.h"
#include "grinstallprogress.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib/gi18n.h>

GRPackageManager::GRPackageManager(GRMainWindow* win)
: _win(NULL)
{
    _win = win;
    assert(_win);

    _res = OrderResult::Failed;

    _parser = new GRParser();
    _userDialog = new GRUserDialog(_win);
}

GRPackageManager::~GRPackageManager()
{
#ifdef DEBUG_MSG
    cout << "GRPackageManager::~GRPackageManager()" << endl;
#endif

    if (_parser)
        delete _parser;

    if (_userDialog)
        delete _userDialog;

    for (vector <GRPackage*>::iterator I = _packages.begin(); I != _packages.end(); I++)
    {
        delete (*I);
    }
}

void
GRPackageManager::initPackage(string strFile)
{
    if (FileExists(strFile))
        _packages = _parser->getPackageFromJson(strFile);

    string strConfigDirectory = PACKAGE_VERDIR;
    string strConfigFile = strConfigDirectory + ".version.json";
    if (!FileExists(strConfigFile))
    {
        create_directory(strConfigDirectory.c_str());
        if (!is_directory_exists(strConfigDirectory.c_str()))
            return;

        _parser->createPackageVersion(strConfigFile);
    }
}

vector<GRPackage*>
GRPackageManager::getPackages()
{
    return _packages;
}

void
GRPackageManager::removePackage(int index)
{
    GRPackage* pPackage = _packages.at(index);

#ifdef DEBUG_MSG
    cout << pPackage->name().c_str() << endl;
#endif

    if (pPackage)
        delete pPackage;

    _packages.erase(_packages.begin() + index);
}

void
GRPackageManager::removeAllPackage()
{
    for (vector <GRPackage*>::iterator I = _packages.begin(); I != _packages.end(); I++)
    {
        delete (*I);
    }
}

void
GRPackageManager::addArchive(pkgAcquire *fetcher, GRPackage* pPackage, string strDownloadPath)
{
    string strName = pPackage->name();
    string strUrl = pPackage->url();
    size_t nfound = strUrl.find_last_of("/\\");
    string strToFile = strUrl.substr(nfound+1);
    string strExtension = get_find_extension(strToFile.c_str());

    if (strExtension.empty())
        strToFile = pPackage->file();

    string strAddr = strDownloadPath;
    string strFromFile = pPackage->file();
    strAddr += "/";
    strAddr += strFromFile;
    unlink(strAddr.c_str());

    char szUri[512];
    const char* pzUTFURL = utf8_to_locale(strUrl.c_str());
    snprintf(szUri, 512, pzUTFURL, strToFile.c_str());
    new pkgAcqFileSane(fetcher, szUri, HashStringList(), 0, strUrl.c_str(), strName.c_str(), "", strAddr);
}

bool
GRPackageManager::downloadPackage(GRPackage* pPackage, bool checkPackage, bool autoClose)
{
    string strName = pPackage->name();
    gchar* gProgram = g_find_program_in_path(strName.c_str());
    if (gProgram == NULL)
    {
        if (checkPackage)
        {
            char szMsg[512];
            snprintf(szMsg, sizeof(szMsg), _("No <b>%s</b> packages available. Do you want to install?"), strName.c_str());
            if (!_userDialog->confirm(szMsg))
            {
                _res = OrderResult::Cancel;
                return false;
            }
        }

        startPackageInstall(pPackage, autoClose);

        g_free(gProgram);
        gProgram = g_find_program_in_path(strName.c_str());

        if (gProgram == NULL)
        {
            _error->Error(_("Package installation error"));
            return false;
        }
    }
    g_free(gProgram);

    return true;
}

void
GRPackageManager::downloadArchive(GRPackage* pPackage)
{
    GRMainWindow* me = _win;
    GRFetchProgress *pStatus = new GRFetchProgress(me);
    pkgAcquire fetcher(pStatus);

    string strName = pPackage->name();
    string pkgDownloadDir = PACKAGE_DOWNLOADDIR;
    string strAddr = pkgDownloadDir + strName;

    if (!strAddr.empty())
    {
        bool bIsExist = is_directory_exists(strAddr.c_str());
        if (!bIsExist)
        {
            bool bIsCreate = create_directory(strAddr.c_str());
            if (!bIsCreate)
            {
                _error->Error(_("Error creating directory"));
                delete pStatus;
            }
        }
    }

    if (!pPackage->url().empty())
        addArchive(&fetcher, pPackage, strAddr);

    if (0 < pPackage->getDepModuleSize())
    {
        string strDepAddr = pkgDownloadDir + pPackage->name();
        vector <GRPackage*> depPackages = pPackage->getDepModule();
        for (vector <GRPackage*>::iterator dep = depPackages.begin(); dep != depPackages.end(); dep++)
        {
            GRPackage* pDepPackage =  (GRPackage*)(*dep);
            string strFormat = pDepPackage->format();

            if (strFormat.compare("package") == 0)
            {
                bool res = downloadPackage(pDepPackage, true, true);
                if (!res)
                    return;
            }

            if (strFormat.compare("archive") != 0)
                continue;

            string strDepName = pDepPackage->name();
            addArchive(&fetcher, pDepPackage, strDepAddr);
        }
    }

    if (fetcher.Run(50000) == pkgAcquire::Failed)
    {
        _error->Error(_("Package downloading error"));
        fetcher.Shutdown();
        delete pStatus;
        return;
    }

    int nNumPackages = 0;
    int nNumPackagesTotal = 0;
    bool bTransient = false;
    bool bFailed = false;
    for (pkgAcquire::ItemIterator I = fetcher.ItemsBegin(); I != fetcher.ItemsEnd(); I++)
    {
        nNumPackagesTotal += 1;

        if ((*I)->Status == pkgAcquire::Item::StatDone && (*I)->Complete)
        {
            nNumPackages += 1;
            continue;
        }

        if ((*I)->Status == pkgAcquire::Item::StatIdle)
        {
            bTransient = true;
            continue;
        }

        (*I)->Finished();

        bFailed = true;
        string errm = (*I)->ErrorText;
        char szErrorMsg[512];
        snprintf(szErrorMsg, 512, _("Failed to fetch %s\n  %s\n\n"), (*I)->DescURI().c_str(), errm.c_str());
        _error->Warning("%s", szErrorMsg);
    }

    if (bFailed == true)
    {
        if (pStatus->IsCancelled())
            _res = OrderResult::Cancel;
        else
            _error->Error("Unable to correct missing packages");
        fetcher.Shutdown();
        delete pStatus;
        return;
    }

    _res = startScriptInstall(pPackage);

    removeDownloadFile(pPackage, strAddr);

    fetcher.Shutdown();
    delete pStatus;
}

void
GRPackageManager::startDownload(int index)
{
    GRPackage* pPackage = _packages.at(index);
    if (!pPackage)
    {
        _error->Error(_("No packages"));
        return;
    }

    string strFormat = pPackage->format();
    if (strFormat.compare("package") == 0)
        downloadPackage(pPackage, false);
    else
        downloadArchive(pPackage);

    return;
}

OrderResult
GRPackageManager::startScriptInstall(GRPackage* package)
{
    if (!package)
    {
        _error->Error(_("No packages selected"));
        return OrderResult::Failed;
    }

    InstallInfo* info = package->getInstallInfo();
    string strDownloadDir = PACKAGE_DOWNLOADDIR + package->name();

    string strInstallDir = package->addr();
    bool bIsExist = is_directory_exists(strInstallDir.c_str());
    if (!bIsExist)
    {
        bool bIsCreate = create_directory(strInstallDir.c_str());
        if (!bIsCreate)
        {
            _error->Error(_("Error creating directory"));
            return OrderResult::Failed;
        }
    }

    GRInstallProgress* installProgress = new GRInstallProgress(_win);
    installProgress->start(info->fileSrc, info->fileFormat, package->name(), package->version(), strDownloadDir);
    _res = installProgress->getResultCode();
    delete installProgress;

    if (_res == OrderResult::Completed)
    {
        string strName = package->name();
        string strVersion = package->version();
        _parser->updatePackageVersion(strName, strVersion);

        int index = package->getIndex();
        _win->updateWindow(index);

        string strPrestart = info->prestart;
        if (!strPrestart.empty())
            system(strPrestart.c_str());
    }
    return _res;
}

OrderResult
GRPackageManager::startPackageInstall(GRPackage* package, bool autoClose)
{
    string strExec = package->exec();
    GRInstallProgress* installProgress = new GRInstallProgress(_win, autoClose);
    installProgress->start(strExec, "shell");
    _res = installProgress->getResultCode();
    delete installProgress;

    if (_res == OrderResult::Completed)
    {
        string strName = package->name();
        string strVersion = package->version();
        _parser->updatePackageVersion(strName, strVersion);

        int index = package->getIndex();
        _win->updateWindow(index);
    }
    return _res;
}

void
GRPackageManager::showError()
{
    if (_res == OrderResult::Failed)
    {
        _userDialog->showErrors();
    }
}

bool
GRPackageManager::isInstallPackage(string strPackageName, string strVersion, string strFormat)
{
    if (strFormat.compare("package") == 0)
    {
        gchar* gProgram = g_find_program_in_path(strPackageName.c_str());
        if (gProgram == NULL)
        {
            bool bCheck =  _parser->checkInstallPackageVersion(strPackageName, strVersion);
            if (bCheck)
                _parser->deletePackage(strPackageName);

            return false;
        }
        g_free(gProgram);
        return true;
    }

    return _parser->checkInstallPackageVersion(strPackageName, strVersion);
}

void
GRPackageManager::removeDownloadFile(GRPackage* package, string strDownloadPath)
{
    string strDeleteFile = strDownloadPath;
    strDeleteFile += "/";
    strDeleteFile += package->file();
    remove(strDeleteFile.c_str());

    vector <GRPackage*> depPackages = package->getDepModule();
    for (vector <GRPackage*>::iterator I = depPackages.begin(); I != depPackages.end(); I++)
    {
        GRPackage* depPackage =  (*I);
        string strDepDeleteFile = strDownloadPath;
        strDepDeleteFile += "/";
        strDepDeleteFile += depPackage->file();
        remove(strDepDeleteFile.c_str());
    }
}

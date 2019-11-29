/* grpackage.h
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

#ifndef _GRPACKAGE_H_
#define _GRPACKAGE_H_
#include "config.h"

#include <string>
#include <vector>

using namespace std;

typedef struct {
    string  fileSrc;
    string  fileDesc;
    string  fileFormat;
    string  prestart;
} InstallInfo;

typedef struct {
    string  _name;
    string  _url;
    string  _addr;
    string  _file;
    string  _desc;
    string  _format;
    string  _version;
    string  _exec;
    string  _icon;
} PackageInfo;

class GRPackage {
public:
    GRPackage();
    ~GRPackage();

private:
    int _index;
    PackageInfo _packageInfo;
    InstallInfo _installInfo;
    vector <GRPackage*>  _depMoudle;

public:
    void addDepModule (GRPackage* package);
    vector <GRPackage*> getDepModule();
    int getDepModuleSize();

    int setIndex(int index) { _index = index; }
    int getIndex() { return _index; }

    void setPackageInfo (PackageInfo info) { _packageInfo = info; }
    void setInstallInfo(InstallInfo info) { _installInfo = info; }
    InstallInfo* getInstallInfo() { return &_installInfo; }

    void setName(string name)   { _packageInfo._name = name; }
    void setURL(string url)     { _packageInfo._url = url; }
    void setAddr(string addr)   { _packageInfo._addr = addr; }
    void setFile(string file)   { _packageInfo._file = file; }
    void setDesc(string disc)   { _packageInfo._desc = disc; }
    void setFormat(string format) { _packageInfo._format = format; }
    void setVersion(string version) { _packageInfo._version = version; }
    void setExec(string exec) { _packageInfo._exec = exec; }
    void setIcon(string icon) { _packageInfo._icon = icon; }

    string name()       { return _packageInfo._name; }
    string url()        { return _packageInfo._url; }
    string addr()       { return _packageInfo._addr; }
    string file()       { return _packageInfo._file; }
    string desc()       { return _packageInfo._desc; }
    string format()     { return _packageInfo._format; }
    string version()    { return _packageInfo._version; }
    string exec()       { return _packageInfo._exec; }
    string icon()       { return _packageInfo._icon; }
};

#endif


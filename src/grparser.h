/* grparser.h
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

#ifndef _GRPARSER_H_
#define _GRPARSER_H_
#include "config.h"

#include <string>
#include <vector>
#include "grpackage.h"

using namespace std;

class GRParser {
public:
    GRParser();
    ~GRParser();

private:
    string _configFile;

public:    
    vector <GRPackage*> getPackageFromJson(string jsonFile);
    void createPackageVersion(string jsonFile);
    void updatePackageVersion(string packageName, string version);
    bool checkInstallPackageVersion(string packageName, string version);
};

#endif


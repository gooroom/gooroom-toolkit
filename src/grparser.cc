/* grparser.cc
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
#include "config.h"

#include <fstream>
#include <iostream>
#include <glib/gi18n.h>

#include <json/json.h>
#include "grparser.h"

#define fopen_s(fp, fmt, mode)          *(fp)=fopen( (fmt), (mode))

GRParser::GRParser()
{
    string strConfigDirectory = PACKAGE_VERDIR;
    _configFile = strConfigDirectory + ".version.json";           
}

GRParser::~GRParser()
{

}

vector <GRPackage*> 
GRParser::getPackageFromJson(string jsonFile)
{
    vector <GRPackage*> vPackages;

    Json::Value value;
    std::ifstream json_doc(jsonFile, std::ifstream::binary);
    json_doc >> value;

    Json::Value packages = value["packages"];
    
    //TODO Exception...
    for (auto &v : packages["package"])
    {
        GRPackage* package = new GRPackage();

        string name = v["name"].isString() ? v["name"].asString() : "";
        string version = v["version"].isString() ? v["version"].asString() : "";
        string addr = v["addr"].isString() ? v["addr"].asString() : "";
        string file = v["file"].isString() ? v["file"].asString() : "";
        string url = v["url"].isString() ? v["url"].asString() : "";
        string desc = v["desc"].isString() ? v["desc"].asString() : "";
        string format = v["format"].isString() ? v["format"].asString() : "";
        string exec = v["exec"].isString() ? v["exec"].asString() : "";
        string icon = v["icon"].isString() ? v["icon"].asString() : "";
        
        for (auto &dep : v["dependency"])
        {
            string dep_name = dep["name"].isString() ? dep["name"].asString() : "";
            string dep_url = dep["url"].isString() ? dep["url"].asString() : "";
            string dep_file = dep["file"].isString() ? dep["file"].asString() : "";
            string dep_format = dep["format"].isString() ? dep["format"].asString() : "";
            string dep_exec = dep["exec"].isString() ? dep["exec"].asString() : "";
            GRPackage* depPackage = new GRPackage();

            depPackage->setName(dep_name);
            depPackage->setURL(dep_url);
            depPackage->setFile(dep_file);
            depPackage->setFormat(dep_format);
            depPackage->setExec(dep_exec);

            package->addDepModule(depPackage);
        }
    
        InstallInfo installInfo;
        Json::Value vInstallInfo = v["install_info"];
        installInfo.fileSrc = vInstallInfo["src"].isString() ? vInstallInfo["src"].asString() : "";
        installInfo.fileDesc = vInstallInfo["desc"].isString() ? vInstallInfo["desc"].asString() : "";        
        installInfo.fileFormat = vInstallInfo["format"].isString() ? vInstallInfo["format"].asString() : "";

        PackageInfo packageInfo;
        packageInfo._name = name;
        packageInfo._url = url;
        packageInfo._file = file;
        packageInfo._addr = addr;
        packageInfo._format = format;
        packageInfo._desc = desc;
        packageInfo._version = version;
        packageInfo._exec = exec;
        packageInfo._icon = icon;
        package->setPackageInfo(packageInfo);
        package->setInstallInfo(installInfo);

        vPackages.push_back(package);
    }

    //fclose(value);

    return vPackages;
}

void
GRParser::createPackageVersion(string strFile)
{
    _configFile = strFile;

    Json::Value root;
    Json::Value packages;
    root["packages"] = packages;

    Json::StyledWriter writer;
    std::string ouputConfig = writer.write(root);

    
    FILE* jsonFile = NULL;
    fopen_s(&jsonFile, strFile.c_str(), "wb");

    if (jsonFile == nullptr)
        return;
    

    size_t fileSize = fwrite(ouputConfig.c_str(), 1, ouputConfig.length(), jsonFile);
    fclose(jsonFile);    
}

void
GRParser::updatePackageVersion(string packageName, string version)
{
    cout << "updatePackageVersion" << endl;
    
    vector <GRPackage*> vPackages;

    Json::Value root;
    std::ifstream json_doc(_configFile, std::ifstream::binary);
    json_doc >> root;

    Json::Value packages = root["packages"];

    bool bCreate = true;
    int size = packages.size();

    if (size > 0)    
    {        
        for (auto &v : root["packages"])
        {
            string strName = v["name"].isString() ? v["name"].asString() : "";
            if (strName.compare(packageName) == 0)
            {                
                bCreate = false;
                v["version"] = version;
            }                                 
        }
    }

    if (bCreate)
    {    
        Json::Value  value;
        value["name"] = packageName;
        value["version"] = version;
        root["packages"].append(value);
    }

    Json::StyledWriter writer;
    std::string ouputConfig = writer.write(root);

    FILE* jsonFile = NULL;
    fopen_s(&jsonFile, _configFile.c_str(), "wb");

    if (jsonFile == nullptr)
        return;
  
    size_t fileSize = fwrite(ouputConfig.c_str(), 1, ouputConfig.length(), jsonFile);
    fclose(jsonFile);       
}

bool 
GRParser::checkInstallPackageVersion(string packageName, string version)
{
    vector <GRPackage*> vPackages;

    Json::Value root;
    std::ifstream json_doc(_configFile, std::ifstream::binary);
    json_doc >> root;

    Json::Value packages = root["packages"];
    int size = packages.size();

    if (size > 0)    
    {        
        for (auto &v : root["packages"])
        {
            string strName = v["name"].isString() ? v["name"].asString() : "";
            if (strName.compare(packageName) == 0)
            {                
                string strVersion = v["version"].isString() ? v["version"].asString() : "";
                if (strVersion.compare(version) == 0)
                    return true;
            }                                 
        }
    }    
    return false;
}

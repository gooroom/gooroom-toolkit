/* grpackage.cc
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

#include "grpackage.h"
#include <gtk/gtk.h>

GRPackage::GRPackage()
{

}

GRPackage::~GRPackage()
{    
#ifdef DEBUG_MSG
    g_print ("GRPackage::~GRPackage()\n");
#endif    
    
    for (vector <GRPackage*>::iterator I = _depMoudle.begin(); I != _depMoudle.end(); I++)
    {
        delete (*I);
    }
}

void 
GRPackage::addDepModule (GRPackage* package)
{
    
    _depMoudle.push_back (package);
}

vector <GRPackage*> 
GRPackage::getDepModule()
{
    return _depMoudle;
}

int 
GRPackage::getDepModuleSize()
{
    return _depMoudle.size();
}

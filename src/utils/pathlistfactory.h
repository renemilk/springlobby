#ifndef SPRINGLOBBY_PATHLISTFACTORY_H
#define SPRINGLOBBY_PATHLISTFACTORY_H

class wxString;
class wxPathList;

class PathlistFactory {
public:
  static wxPathList fromSinglePath(wxString path);
  static wxPathList ConfigFileSearchPaths();
  static wxPathList UikeysLocations();
  static wxPathList AdditionalSearchPaths(wxPathList& pl);
};

#endif // SPRINGLOBBY_PATHLISTFACTORY_H

/**
    This file is part of SpringLobby,
    Copyright (C) 2007-2011

    SpringLobby is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published by
    the Free Software Foundation.

    SpringLobby is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SpringLobby.  If not, see <http://www.gnu.org/licenses/>.
**/

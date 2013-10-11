#ifndef SPRINGLOBBY_HEADER_GUARD_TORRENTOPTIONSPANEL_H_INCLUDED
#define SPRINGLOBBY_HEADER_GUARD_TORRENTOPTIONSPANEL_H_INCLUDED

#include <wx/scrolwin.h>

class wxCheckBox;
class wxTextCtrl;
class Ui;
class wxStaticBox;
class wxRadioButton;
class wxButton;
class wxStaticText;
class wxSpinCtrl;
class wxStaticBoxSizer;
class TorrentOptionsPanel : public wxScrolledWindow {
public:
  TorrentOptionsPanel(wxWindow* parent);
  ~TorrentOptionsPanel();

  void OnApply(wxCommandEvent& event);
  void OnRestore(wxCommandEvent& event);

private:
  wxSpinCtrl* m_parallel_http;
  wxStaticBoxSizer* m_parallel_http_sizer;
  void EnableSettings(bool enable);
  void SetStatusDisplay();

  enum {
    ID_ENABLEP2P = wxID_HIGHEST,
    ID_MAXUP,
    ID_MAXDOWN,
    ID_P2PPORT,
    ID_MAXCONNECTIONS,
    ID_APPLY,
    ID_RESTORE,
    ID_GAMESTART_RADIO,
    ID_INGAME_UP,
    ID_INGAME_DOWN
  };

  DECLARE_EVENT_TABLE()
};

#endif // SPRINGLOBBY_HEADER_GUARD_TORRENTOPTIONSPANEL_H_INCLUDED

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

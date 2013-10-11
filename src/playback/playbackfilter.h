#ifndef SPRINGLOBBY_PLAYBACKFILTER_H_INCLUDED
#define SPRINGLOBBY_PLAYBACKFILTER_H_INCLUDED

#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

class ReplayTab;
class wxToggleButton;
class wxCheckBox;
class wxStaticText;
class wxTextCtrl;
class wxChoice;
class wxButton;
class wxRegEx;
class wxStaticText;

/** \brief The panel contained in BattleListTab used to filter for diff info of battles
 * \todo DOCMEMORE */
template <class PlaybackTabType>
class PlaybackListFilter : public wxPanel {
protected:
  typedef typename PlaybackTabType::PlaybackType PlaybackType;

public:
  PlaybackListFilter(wxWindow* parent, wxWindowID id, PlaybackTabType* parentTab, const wxPoint& pos,
                     const wxSize& size, long style);

  void OnPlayerButton(wxCommandEvent& event);
  void OnFilesizeButton(wxCommandEvent& event);
  void OnDurationButton(wxCommandEvent& event);

  void OnActivate(wxCommandEvent& event);

  void SetActiv(bool state);

  void OnChange(wxCommandEvent& event);
  void OnChangeMap(wxCommandEvent& event);
  void OnChangeMod(wxCommandEvent& event);
  void OnChangeFilesize(wxCommandEvent& event);
  void OnChangeDuration(wxCommandEvent& event);

  void OnPlayerChange(wxCommandEvent& event);

  bool FilterPlayback(const PlaybackType& playback);
  bool GetActiv() const;

  void SetFilterHighlighted(bool state);

  void SaveFilterValues();

protected:
  enum m_button_mode {
    m_equal,
    m_bigger,
    m_smaller
  };

  wxString _GetButtonSign(m_button_mode value);
  m_button_mode _GetNextMode(m_button_mode value);
  m_button_mode _GetButtonMode(wxString sign);
  bool _IntCompare(int a, int b, m_button_mode mode);

  bool m_activ;

  PlaybackTabType* m_parent_tab;
#if wxUSE_TOGGLEBTN
  wxToggleButton* m_filter_show;
#else
  wxCheckBox* m_filter_show;
#endif
  wxStaticText* m_filter_text;

  wxCheckBox* m_filter_activ;

  // Player
  wxStaticText* m_filter_player_text;
  wxButton* m_filter_player_button;
  m_button_mode m_filter_player_mode;
  wxChoice* m_filter_player_choice;
  int m_filter_player_choice_value;

  // Filesize
  wxStaticText* m_filter_filesize_text;
  wxButton* m_filter_filesize_button;
  m_button_mode m_filter_filesize_mode;
  wxTextCtrl* m_filter_filesize_edit;

  // Duration
  wxStaticText* m_filter_duration_text;
  wxButton* m_filter_duration_button;
  m_button_mode m_filter_duration_mode;
  wxTextCtrl* m_filter_duration_edit;
  long m_duration_value;
  void SetDurationValue();

  // Map
  wxStaticText* m_filter_map_text;
  wxTextCtrl* m_filter_map_edit;
  wxCheckBox* m_filter_map_show;
  wxRegEx* m_filter_map_expression;

  // Mod
  wxStaticText* m_filter_mod_text;
  wxTextCtrl* m_filter_mod_edit;
  wxCheckBox* m_filter_mod_show;
  wxRegEx* m_filter_mod_expression;

  DECLARE_EVENT_TABLE()
};

enum {
  PLAYBACK_FILTER_MAP_EDIT,
  PLAYBACK_FILTER_MOD_EDIT,
  PLAYBACK_FILTER_FILESIZE_EDIT,
  PLAYBACK_FILTER_DURATION_EDIT,
  PLAYBACK_FILTER_PLAYER_CHOICE,
  PLAYBACK_FILTER_MAP_SHOW,
  PLAYBACK_FILTER_MOD_SHOW,
  PLAYBACK_FILTER_PLAYER_BUTTON,
  PLAYBACK_FILTER_DURATION_BUTTON,
  PLAYBACK_FILTER_FILESIZE_BUTTON
};

#include "playbackfilter.cpp"
#endif // SPRINGLOBBY_PLAYBACKFILTER_H_INCLUDED

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

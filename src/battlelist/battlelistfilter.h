#ifndef SPRINGLOBBY_HEADERGUARD_BATTLELISTFILTER_H
#define SPRINGLOBBY_HEADERGUARD_BATTLELISTFILTER_H

#include <wx/panel.h>
#include <wx/bmpcbox.h>

#include "battlelisttab.h"
#include "utils/mixins.hh"
///////////////////////////////////////////////////////////////////////////

class BattleListTab;
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
class BattleListFilter : public wxPanel, public SL::NonCopyable {
public:
  BattleListFilter(wxWindow* parent, wxWindowID id, BattleListTab* parentBattleListTab, const wxPoint& pos,
                   const wxSize& size, long style);

  void OnRankButton(wxCommandEvent& event);
  void OnPlayerButton(wxCommandEvent& event);
  void OnMaxPlayerButton(wxCommandEvent& event);
  void OnSpectatorButton(wxCommandEvent& event);
  void OnActivate(wxCommandEvent& event);

  void SetActiv(bool state);

  void OnChange(wxCommandEvent& event);
  void OnChangeMap(wxCommandEvent& event);
  void OnChangeMod(wxCommandEvent& event);
  void OnChangeDescription(wxCommandEvent& event);
  void OnChangeHost(wxCommandEvent& event);

  void OnRankChange(wxCommandEvent& event);
  void OnPlayerChange(wxCommandEvent& event);
  void OnMaxPlayerChange(wxCommandEvent& event);
  void OnSpectatorChange(wxCommandEvent& event);

  bool FilterBattle(IBattle& battle);
  bool GetActiv() const;

  void SetFilterHighlighted(bool state);

  void SaveFilterValues();

  enum ButtonMode {
    BUTTON_MODE_EQUAL,
    BUTTON_MODE_BIGGER,
    BUTTON_MODE_SMALLER
  };

protected:
  wxString _GetButtonSign(ButtonMode value);
  ButtonMode _GetNextMode(ButtonMode value);
  ButtonMode _GetButtonMode(const wxString& sign);
  bool _IntCompare(int a, int b, ButtonMode mode);

  /** A function callback used to transform an input string. */
  typedef wxString (*StringTransformFunction)(const wxString& input);

  bool m_activ;

  BattleListTab* m_parent_battlelisttab;
#if wxUSE_TOGGLEBTN
  wxToggleButton* m_filter_show;
#else
  wxCheckBox* m_filter_show;
#endif
  wxStaticText* m_filter_text;

  wxCheckBox* m_filter_activ;

  // Host
  wxStaticText* m_filter_host_text;
  wxTextCtrl* m_filter_host_edit;
  wxRegEx* m_filter_host_expression;

  // Status
  wxStaticText* m_filter_status_text;
  wxStaticText* m_filter_status_text1;
  wxCheckBox* m_filter_status_locked;
  wxCheckBox* m_filter_status_pass;
  wxCheckBox* m_filter_status_start;
  wxCheckBox* m_filter_status_full;
  wxCheckBox* m_filter_status_open;

  // Rank
  wxStaticText* m_filter_rank_text;
  ButtonMode m_filter_rank_mode;
  wxButton* m_filter_rank_button;
  wxBitmapComboBox* m_filter_rank_choice;
  int m_filter_rank_choice_value;

  // Description
  wxStaticText* m_filter_description_text;
  wxTextCtrl* m_filter_description_edit;
  wxRegEx* m_filter_description_expression;

  // Player
  wxStaticText* m_filter_player_text;
  wxButton* m_filter_player_button;
  ButtonMode m_filter_player_mode;
  wxChoice* m_filter_player_choice;
  int m_filter_player_choice_value;

  // Map
  wxStaticText* m_filter_map_text;
  wxTextCtrl* m_filter_map_edit;
  wxCheckBox* m_filter_map_show;
  wxRegEx* m_filter_map_expression;

  // Max Player
  wxStaticText* m_filter_maxplayer_text;
  wxButton* m_filter_maxplayer_button;
  ButtonMode m_filter_maxplayer_mode;
  wxChoice* m_filter_maxplayer_choice;
  int m_filter_maxplayer_choice_value;

  // Mod
  wxStaticText* m_filter_mod_text;
  wxTextCtrl* m_filter_mod_edit;
  wxCheckBox* m_filter_mod_show;
  wxRegEx* m_filter_mod_expression;

  // Spectator
  wxStaticText* m_filter_spectator_text;
  wxButton* m_filter_spectator_button;
  ButtonMode m_filter_spectator_mode;
  wxChoice* m_filter_spectator_choice;
  int m_filter_spectator_choice_value;

  wxCheckBox* m_filter_highlighted;

public:
  /** Test whether an input string matches a filter string.
  *  The filter will be tried as both a fixed string, and as a regular expression.
  *
  * @param input The input string to test.
  *
  * @param filter_string A fixed string to match against.
  *
  * @param filter_regex A regular expression object pointer.  If
  * the expression is valid, it will be used to match the input if
  * the fixed string does not.
  *
  * @param additional_transform A function that should be called to
  * generate an additional <em>input</em> string.
  *
  * @param case_sensitive @c true if the comparison should be done case-sensitively.
  *
  * @return @c true if the input or generated input matches the filter string, and @c false otherwise.
  */
  static bool StringMatches(const wxString& input, const wxString& filter_string, const wxRegEx* filter_regex = NULL,
                            StringTransformFunction additional_transform = NULL, bool case_sensitive = false);

protected:
  DECLARE_EVENT_TABLE()
};

enum {
  BATTLE_FILTER_HOST_EDIT,
  BATTLE_FILTER_DESCRIPTION_EDIT,
  BATTLE_FILTER_MAP_EDIT,
  BATTLE_FILTER_MOD_EDIT,
  BATTLE_FILTER_LOCKED,
  BATTLE_FILTER_OPEN,
  BATTLE_FILTER_PASSWORDED,
  BATTLE_FILTER_FULL,
  BATTLE_FILTER_STARTED,
  BATTLE_FILTER_RANK_CHOICE,
  BATTLE_FILTER_RANK_BUTTON,
  BATTLE_FILTER_PLAYER_CHOICE,
  BATTLE_FILTER_MAXPLAYER_CHOICE,
  BATTLE_FILTER_SPECTATOR_CHOICE,
  BATTLE_FILTER_MAP_SHOW,
  BATTLE_FILTER_MOD_SHOW,
  BATTLE_FILTER_PLAYER_BUTTON,
  BATTLE_FILTER_MAXPLAYER_BUTTON,
  BATTLE_FILTER_SPECTATOR_BUTTON,
  BATTLE_FILTER_HIGHLIGHTED
};

#endif // SPRINGLOBBY_HEADERGUARD_BATTLELISTFILTER_H

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

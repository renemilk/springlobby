/* Copyright (C) 2007-2011 The SpringLobby Team. All rights reserved. */
//
// Class: BattleOptionsTab
//

#if wxUSE_TOGGLEBTN
#include <wx/tglbtn.h>
#endif
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/intl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/statbox.h>
#include <wx/event.h>
#include <wx/regex.h>

#include "battlelistfilter.h"
#include "battlelistfiltervalues.h"
#include "battlelistctrl.h"
#include "battle.h"
#include "uiutils.h"
#include "utils/tasutil.h"
#include "settings.h"
#include "aui/auimanager.h"
#include "useractions.h"

#include "images/rank0.xpm"
#include "images/rank1.xpm"
#include "images/rank2.xpm"
#include "images/rank3.xpm"
#include "images/rank4.xpm"
#include "images/rank5.xpm"
#include "images/rank6.xpm"
#include "images/rank7.xpm"
///////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(BattleListFilter, wxPanel)

EVT_BUTTON(BATTLE_FILTER_RANK_BUTTON, BattleListFilter::OnRankButton)
EVT_BUTTON(BATTLE_FILTER_PLAYER_BUTTON, BattleListFilter::OnPlayerButton)
EVT_BUTTON(BATTLE_FILTER_MAXPLAYER_BUTTON, BattleListFilter::OnMaxPlayerButton)
EVT_BUTTON(BATTLE_FILTER_SPECTATOR_BUTTON, BattleListFilter::OnSpectatorButton)
EVT_CHOICE(BATTLE_FILTER_SPECTATOR_CHOICE, BattleListFilter::OnSpectatorChange)
EVT_CHOICE(BATTLE_FILTER_MAXPLAYER_CHOICE, BattleListFilter::OnMaxPlayerChange)
EVT_CHOICE(BATTLE_FILTER_PLAYER_CHOICE, BattleListFilter::OnPlayerChange)
EVT_COMBOBOX(BATTLE_FILTER_RANK_CHOICE, BattleListFilter::OnRankChange)
EVT_CHECKBOX(BATTLE_FILTER_LOCKED, BattleListFilter::OnChange)
EVT_CHECKBOX(BATTLE_FILTER_OPEN, BattleListFilter::OnChange)
EVT_CHECKBOX(BATTLE_FILTER_PASSWORDED, BattleListFilter::OnChange)
EVT_CHECKBOX(BATTLE_FILTER_FULL, BattleListFilter::OnChange)
EVT_CHECKBOX(BATTLE_FILTER_STARTED, BattleListFilter::OnChange)
EVT_TEXT(BATTLE_FILTER_HOST_EDIT, BattleListFilter::OnChangeHost)
EVT_TEXT(BATTLE_FILTER_DESCRIPTION_EDIT, BattleListFilter::OnChangeDescription)
EVT_TEXT(BATTLE_FILTER_MAP_EDIT, BattleListFilter::OnChangeMap)
EVT_TEXT(BATTLE_FILTER_MOD_EDIT, BattleListFilter::OnChangeMod)
EVT_CHECKBOX(BATTLE_FILTER_MAP_SHOW, BattleListFilter::OnChange)
EVT_CHECKBOX(BATTLE_FILTER_MOD_SHOW, BattleListFilter::OnChange)
EVT_CHECKBOX(BATTLE_FILTER_HIGHLIGHTED, BattleListFilter::OnChange)

END_EVENT_TABLE()

BattleListFilter::BattleListFilter(wxWindow* parent, wxWindowID id, BattleListTab* parentBattleListTab,
                                   const wxPoint& pos, const wxSize& size, long style)
  : wxPanel(parent, id, pos, size, style)
  , m_parent_battlelisttab(parentBattleListTab)
  , m_filter_host_edit(0)
  , m_filter_host_expression(0)
  , m_filter_description_edit(0)
  , m_filter_description_expression(0)
  , m_filter_map_edit(0)
  , m_filter_map_expression(0)
  , m_filter_mod_edit(0)
  , m_filter_mod_expression(0)
  , m_filter_highlighted(0) {
  GetAui().manager->AddPane(this, wxLEFT, _T( "battlelistfilter" ));

  BattleListFilterValues f_values = sett().GetBattleFilterValues(sett().GetLastBattleFilterProfileName());

  wxBoxSizer* m_filter_sizer;
  m_filter_sizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBoxSizer* m_filter_body_sizer;
  m_filter_body_sizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxEmptyString), wxVERTICAL);

  wxBoxSizer* m_filter_body_row1_sizer;
  m_filter_body_row1_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* m_filter_column_1;
  m_filter_column_1 = new wxBoxSizer(wxHORIZONTAL);

  m_filter_host_text = new wxStaticText(this, wxID_ANY, _("Host:"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_host_text->Wrap(-1);
  m_filter_host_text->SetMinSize(wxSize(90, -1));

  m_filter_column_1->Add(m_filter_host_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_host_edit = new wxTextCtrl(this, BATTLE_FILTER_HOST_EDIT, f_values.host, wxDefaultPosition, wxSize(-1, -1),
                                      0 | wxSIMPLE_BORDER);
  m_filter_host_edit->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString));
  m_filter_host_edit->SetMinSize(wxSize(220, -1));
  m_filter_host_expression = new wxRegEx(m_filter_host_edit->GetValue(), wxRE_ICASE);

  m_filter_column_1->Add(m_filter_host_edit, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row1_sizer->Add(m_filter_column_1, 0, wxEXPAND, 5);

  wxBoxSizer* m_filter_status_sizer1;
  m_filter_status_sizer1 = new wxBoxSizer(wxHORIZONTAL);

  m_filter_status_text = new wxStaticText(this, wxID_ANY, _("Status:"), wxPoint(-1, -1), wxDefaultSize, 0);
  m_filter_status_text->Wrap(-1);
  m_filter_status_sizer1->Add(m_filter_status_text, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_status_locked = new wxCheckBox(this, BATTLE_FILTER_LOCKED, _("Locked"), wxDefaultPosition, wxDefaultSize, 0);
  m_filter_status_locked->SetValue(f_values.status_locked);

  m_filter_status_sizer1->Add(m_filter_status_locked, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

  m_filter_status_pass =
      new wxCheckBox(this, BATTLE_FILTER_PASSWORDED, _("Passworded"), wxDefaultPosition, wxDefaultSize, 0);
  m_filter_status_pass->SetValue(f_values.status_passworded);

  m_filter_status_sizer1->Add(m_filter_status_pass, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

  m_filter_highlighted =
      new wxCheckBox(this, BATTLE_FILTER_HIGHLIGHTED, _("Highlighted only"), wxDefaultPosition, wxDefaultSize, 0);
  m_filter_highlighted->SetValue(f_values.highlighted_only);

  m_filter_status_sizer1->Add(m_filter_highlighted, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

  m_filter_body_row1_sizer->Add(m_filter_status_sizer1, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_rank_sizer;
  m_filter_rank_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_rank_text = new wxStaticText(this, wxID_ANY, _("Rank Limit:"), wxDefaultPosition, wxDefaultSize, 0);
  m_filter_rank_text->Wrap(-1);
  m_filter_rank_sizer->Add(m_filter_rank_text, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_rank_button =
      new wxButton(this, BATTLE_FILTER_RANK_BUTTON, f_values.rank_mode, wxDefaultPosition, wxSize(25, 25), 0);
  m_filter_rank_sizer->Add(m_filter_rank_button, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_rank_choice = new wxBitmapComboBox(this, BATTLE_FILTER_RANK_CHOICE, _T("All"), wxDefaultPosition,
                                              wxSize(-1, -1), 0, NULL, wxSIMPLE_BORDER | wxCB_READONLY);
  m_filter_rank_choice->Append(_T("All"));
  m_filter_rank_choice->Append(_T("1"), wxBitmap(rank0_xpm));
  m_filter_rank_choice->Append(_T("2"), wxBitmap(rank1_xpm));
  m_filter_rank_choice->Append(_T("3"), wxBitmap(rank2_xpm));
  m_filter_rank_choice->Append(_T("4"), wxBitmap(rank3_xpm));
  m_filter_rank_choice->Append(_T("5"), wxBitmap(rank4_xpm));
  m_filter_rank_choice->Append(_T("6"), wxBitmap(rank5_xpm));
  m_filter_rank_choice->Append(_T("7"), wxBitmap(rank6_xpm));
  m_filter_rank_choice->Append(_T("8"), wxBitmap(rank7_xpm));

  m_filter_rank_choice->SetSelection(GetIntParam(f_values.rank));
  m_filter_rank_choice->SetMinSize(wxSize(60, -1));

  m_filter_rank_sizer->Add(m_filter_rank_choice, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row1_sizer->Add(m_filter_rank_sizer, 0, wxEXPAND, 5);

  m_filter_body_sizer->Add(m_filter_body_row1_sizer, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_body_row2_sizer;
  m_filter_body_row2_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* m_filter_description_sizer;
  m_filter_description_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_description_text = new wxStaticText(this, wxID_ANY, _("Description:"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_description_text->Wrap(-1);
  m_filter_description_text->SetMinSize(wxSize(90, -1));

  m_filter_description_sizer->Add(m_filter_description_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_description_edit = new wxTextCtrl(this, BATTLE_FILTER_DESCRIPTION_EDIT, f_values.description,
                                             wxDefaultPosition, wxSize(-1, -1), 0 | wxSIMPLE_BORDER);
  m_filter_description_edit->SetMinSize(wxSize(220, -1));
  m_filter_description_expression = new wxRegEx(m_filter_description_edit->GetValue(), wxRE_ICASE);

  m_filter_description_sizer->Add(m_filter_description_edit, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row2_sizer->Add(m_filter_description_sizer, 0, wxEXPAND, 5);

  wxBoxSizer* m_filter_sizer2;
  m_filter_sizer2 = new wxBoxSizer(wxHORIZONTAL);

  m_filter_status_text1 = new wxStaticText(this, wxID_ANY, _("Status:"), wxPoint(-1, -1), wxDefaultSize, 0);
  m_filter_status_text1->Wrap(-1);
  m_filter_sizer2->Add(m_filter_status_text1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_status_start =
      new wxCheckBox(this, BATTLE_FILTER_STARTED, _("Started"), wxDefaultPosition, wxDefaultSize, 0);
  m_filter_status_start->SetValue(f_values.status_start);

  m_filter_sizer2->Add(m_filter_status_start, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

  m_filter_status_full = new wxCheckBox(this, BATTLE_FILTER_FULL, _("Full"), wxDefaultPosition, wxDefaultSize, 0);
  m_filter_status_full->SetValue(f_values.status_full);

  m_filter_sizer2->Add(m_filter_status_full, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

  m_filter_status_open = new wxCheckBox(this, BATTLE_FILTER_OPEN, _("Open"), wxDefaultPosition, wxDefaultSize, 0);
  m_filter_status_open->SetValue(f_values.status_open);

  m_filter_sizer2->Add(m_filter_status_open, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 5);

  m_filter_body_row2_sizer->Add(m_filter_sizer2, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_player_sizer;
  m_filter_player_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_player_text = new wxStaticText(this, wxID_ANY, _("Player:"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_player_text->Wrap(-1);
  m_filter_player_sizer->Add(m_filter_player_text, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_player_button =
      new wxButton(this, BATTLE_FILTER_PLAYER_BUTTON, f_values.player_mode, wxDefaultPosition, wxSize(25, 25), 0);
  m_filter_player_sizer->Add(m_filter_player_button, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  wxArrayString m_filter_player_choiceChoices;

  m_filter_player_choiceChoices.Add(_("All"));
  for (wxLongLong i = 0; i <= 32; i++)
    m_filter_player_choiceChoices.Add(i.ToString());

  m_filter_player_choice = new wxChoice(this, BATTLE_FILTER_PLAYER_CHOICE, wxDefaultPosition, wxSize(-1, -1),
                                        m_filter_player_choiceChoices, 0);
  m_filter_player_choice->SetSelection(GetIntParam(f_values.player_num));
  m_filter_player_choice->SetMinSize(wxSize(40, -1));

  m_filter_player_sizer->Add(m_filter_player_choice, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row2_sizer->Add(m_filter_player_sizer, 0, wxEXPAND | wxLEFT, 5);

  m_filter_body_sizer->Add(m_filter_body_row2_sizer, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_body_row3_sizer;
  m_filter_body_row3_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* m_filter_map_sizer;
  m_filter_map_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_map_text = new wxStaticText(this, wxID_ANY, _("Map:"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_map_text->Wrap(-1);
  m_filter_map_text->SetMinSize(wxSize(90, -1));

  m_filter_map_sizer->Add(m_filter_map_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_map_edit = new wxTextCtrl(this, BATTLE_FILTER_MAP_EDIT, f_values.map, wxDefaultPosition, wxSize(-1, -1),
                                     0 | wxSIMPLE_BORDER);
  m_filter_map_edit->SetMinSize(wxSize(140, -1));
  m_filter_map_expression = new wxRegEx(m_filter_map_edit->GetValue(), wxRE_ICASE);

  m_filter_map_sizer->Add(m_filter_map_edit, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row3_sizer->Add(m_filter_map_sizer, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_only_map_sizer;
  m_filter_only_map_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_map_show =
      new wxCheckBox(this, BATTLE_FILTER_MAP_SHOW, _("Only maps i have"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_map_show->SetValue(f_values.map_show);
  m_filter_map_show->SetMinSize(wxSize(140, -1));

  m_filter_only_map_sizer->Add(m_filter_map_show, 1,
                               wxEXPAND | wxALIGN_CENTER_HORIZONTAL | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row3_sizer->Add(m_filter_only_map_sizer, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_maxplayer_sizer;
  m_filter_maxplayer_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_maxplayer_text = new wxStaticText(this, wxID_ANY, _("Max.Player:"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_maxplayer_text->Wrap(-1);
  m_filter_maxplayer_sizer->Add(m_filter_maxplayer_text, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_maxplayer_button =
      new wxButton(this, BATTLE_FILTER_MAXPLAYER_BUTTON, f_values.maxplayer_mode, wxDefaultPosition, wxSize(25, 25), 0);
  m_filter_maxplayer_sizer->Add(m_filter_maxplayer_button, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  wxArrayString m_filter_maxplayer_choiceChoices;
  m_filter_maxplayer_choiceChoices.Add(_T( "All" ));
  for (wxLongLong i = 0; i <= 32; i++)
    m_filter_maxplayer_choiceChoices.Add(i.ToString());

  m_filter_maxplayer_choice = new wxChoice(this, BATTLE_FILTER_MAXPLAYER_CHOICE, wxDefaultPosition, wxSize(-1, -1),
                                           m_filter_maxplayer_choiceChoices, 0);
  m_filter_maxplayer_choice->SetSelection(GetIntParam(f_values.maxplayer));
  m_filter_maxplayer_choice->SetMinSize(wxSize(40, -1));

  m_filter_maxplayer_sizer->Add(m_filter_maxplayer_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row3_sizer->Add(m_filter_maxplayer_sizer, 0, wxEXPAND, 5);

  m_filter_body_sizer->Add(m_filter_body_row3_sizer, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_body_row4_sizer;
  m_filter_body_row4_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* m_filter_mod_sizer;
  m_filter_mod_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_mod_text = new wxStaticText(this, wxID_ANY, _("Game:"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_mod_text->Wrap(-1);
  m_filter_mod_text->SetMinSize(wxSize(90, -1));

  m_filter_mod_sizer->Add(m_filter_mod_text, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_mod_edit = new wxTextCtrl(this, BATTLE_FILTER_MOD_EDIT, f_values.mod, wxDefaultPosition, wxSize(-1, -1),
                                     0 | wxSIMPLE_BORDER);
  m_filter_mod_edit->SetMinSize(wxSize(140, -1));
  m_filter_mod_expression = new wxRegEx(m_filter_mod_edit->GetValue(), wxRE_ICASE);

  m_filter_mod_sizer->Add(m_filter_mod_edit, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row4_sizer->Add(m_filter_mod_sizer, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_only_mod_sizer;
  m_filter_only_mod_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_mod_show =
      new wxCheckBox(this, BATTLE_FILTER_MOD_SHOW, _("Only games I have"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_mod_show->SetValue(f_values.mod_show);
  m_filter_mod_show->SetMinSize(wxSize(140, -1));

  m_filter_only_mod_sizer->Add(m_filter_mod_show, 1,
                               wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 5);

  m_filter_body_row4_sizer->Add(m_filter_only_mod_sizer, 1, wxEXPAND, 5);

  wxBoxSizer* m_filter_spectator_sizer;
  m_filter_spectator_sizer = new wxBoxSizer(wxHORIZONTAL);

  m_filter_spectator_text = new wxStaticText(this, wxID_ANY, _("    Spectator:"), wxDefaultPosition, wxSize(-1, -1), 0);
  m_filter_spectator_text->Wrap(-1);
  m_filter_spectator_sizer->Add(m_filter_spectator_text, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_filter_spectator_button =
      new wxButton(this, BATTLE_FILTER_SPECTATOR_BUTTON, f_values.spectator_mode, wxDefaultPosition, wxSize(25, 25), 0);
  m_filter_spectator_sizer->Add(m_filter_spectator_button, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  wxArrayString m_filter_spectator_choiceChoices;
  m_filter_spectator_choiceChoices.Add(_T( "All" ));
  for (wxLongLong i = 0; i <= 32; i++)
    m_filter_spectator_choiceChoices.Add(i.ToString());

  m_filter_spectator_choice = new wxChoice(this, BATTLE_FILTER_SPECTATOR_CHOICE, wxDefaultPosition, wxSize(-1, -1),
                                           m_filter_spectator_choiceChoices, 0);
  m_filter_spectator_choice->SetSelection(GetIntParam(f_values.spectator));
  m_filter_spectator_choice->SetMinSize(wxSize(40, -1));

  m_filter_spectator_sizer->Add(m_filter_spectator_choice, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_filter_body_row4_sizer->Add(m_filter_spectator_sizer, 0, wxEXPAND, 5);

  m_filter_body_sizer->Add(m_filter_body_row4_sizer, 1, wxEXPAND, 5);

  m_filter_sizer->Add(m_filter_body_sizer, 1, wxEXPAND | wxALIGN_CENTER_HORIZONTAL, 5);

  m_activ = false;
  m_filter_rank_mode = _GetButtonMode(f_values.rank_mode);
  m_filter_player_mode = _GetButtonMode(f_values.player_mode);
  m_filter_maxplayer_mode = _GetButtonMode(f_values.maxplayer_mode);
  m_filter_spectator_mode = _GetButtonMode(f_values.spectator_mode);
  m_filter_rank_choice_value = m_filter_rank_choice->GetSelection() - 1;
  m_filter_player_choice_value = m_filter_player_choice->GetSelection() - 1;
  m_filter_maxplayer_choice_value = m_filter_maxplayer_choice->GetSelection() - 1;
  m_filter_spectator_choice_value = m_filter_spectator_choice->GetSelection() - 1;

  this->SetSizer(m_filter_sizer);
  this->Layout();
  m_filter_sizer->Fit(this);

  delete m_filter_map_expression;
  m_filter_map_expression = new wxRegEx(m_filter_map_edit->GetValue(), wxRE_ICASE);

  delete m_filter_mod_expression;
  m_filter_mod_expression = new wxRegEx(m_filter_mod_edit->GetValue(), wxRE_ICASE);

  delete m_filter_description_expression;
  m_filter_description_expression = new wxRegEx(m_filter_description_edit->GetValue(), wxRE_ICASE);

  delete m_filter_host_expression;
  m_filter_host_expression = new wxRegEx(m_filter_host_edit->GetValue(), wxRE_ICASE);

  wxCommandEvent dummy;
  OnChange(dummy);
}

BattleListFilter::ButtonMode BattleListFilter::_GetButtonMode(const wxString& sign) {
  if (sign == _T( "<" ))
    return BattleListFilter::BUTTON_MODE_SMALLER;
  if (sign == _T( ">" ))
    return BattleListFilter::BUTTON_MODE_BIGGER;
  return BattleListFilter::BUTTON_MODE_EQUAL;
}

wxString BattleListFilter::_GetButtonSign(const BattleListFilter::ButtonMode value) {
  switch (value) {
    case BUTTON_MODE_EQUAL:
      return _T( "=" );
    case BUTTON_MODE_SMALLER:
      return _T( "<" );
    case BUTTON_MODE_BIGGER:
    default:
      return _T( ">" );
  }
}

BattleListFilter::ButtonMode BattleListFilter::_GetNextMode(const BattleListFilter::ButtonMode value) {
  switch (value) {
    case BUTTON_MODE_EQUAL:
      return BUTTON_MODE_SMALLER;
    case BUTTON_MODE_SMALLER:
      return BUTTON_MODE_BIGGER;
    case BUTTON_MODE_BIGGER:
    default:
      return BUTTON_MODE_EQUAL;
  }
}

bool BattleListFilter::_IntCompare(const int a, const int b, const BattleListFilter::ButtonMode mode) {
  switch (mode) {
    case BUTTON_MODE_EQUAL:
      return (a == b);
    case BUTTON_MODE_SMALLER:
      return (a < b);
    case BUTTON_MODE_BIGGER:
      return (a > b);
    default:
      return false;
  }
}

bool BattleListFilter::StringMatches(const wxString& input, const wxString& filter_string, const wxRegEx* filter_regex,
                                     StringTransformFunction additional_transform, bool case_sensitive) {
  if (filter_string.Len() < 1 || filter_string == _T(""))
    return true;

  wxString input_cased(input);
  wxString filter_cased(filter_string);
  bool use_regex(filter_regex && filter_regex->IsValid());

  /* Case-(in)sensitivize the input and filter strings. */
  if (!case_sensitive) {
    input_cased.MakeUpper();
    filter_cased.MakeUpper();
  }

  if (input_cased.Find(filter_cased) != wxNOT_FOUND || (use_regex && filter_regex->Matches(input)))
    return true;
  else if (additional_transform) {
    /* Try matching using a transformed input string. */
    wxString input_generated(additional_transform(input));
    if (!case_sensitive)
      input_generated.MakeUpper();

    if (input_generated.Find(filter_cased) != wxNOT_FOUND || (use_regex && filter_regex->Matches(input_generated)))
      return true;
  }

  return false;
}

void BattleListFilter::OnRankButton(wxCommandEvent& event) {
  m_filter_rank_mode = _GetNextMode(m_filter_rank_mode);
  m_filter_rank_button->SetLabel(_GetButtonSign(m_filter_rank_mode));
  OnChange(event);
}

void BattleListFilter::OnPlayerButton(wxCommandEvent& event) {
  m_filter_player_mode = _GetNextMode(m_filter_player_mode);
  m_filter_player_button->SetLabel(_GetButtonSign(m_filter_player_mode));
  OnChange(event);
}

void BattleListFilter::OnMaxPlayerButton(wxCommandEvent& event) {
  m_filter_maxplayer_mode = _GetNextMode(m_filter_maxplayer_mode);
  m_filter_maxplayer_button->SetLabel(_GetButtonSign(m_filter_maxplayer_mode));
  OnChange(event);
}

void BattleListFilter::OnSpectatorButton(wxCommandEvent& event) {
  m_filter_spectator_mode = _GetNextMode(m_filter_spectator_mode);
  m_filter_spectator_button->SetLabel(_GetButtonSign(m_filter_spectator_mode));
  OnChange(event);
}

void BattleListFilter::SetActiv(bool state) {
  m_activ = state;
  if (m_parent_battlelisttab != 0) {
    m_parent_battlelisttab->UpdateList();
  }
}

bool BattleListFilter::FilterBattle(IBattle& battle) {
  if (!m_activ)
    return true;

  if (m_filter_highlighted->IsChecked()) {
    try {
      wxString host = battle.GetFounder().GetNick();
      if (!useractions().DoActionOnUser(UserActions::ActHighlight, host))
        return false;

      for (unsigned int i = 0; i < battle.GetNumUsers(); ++i) {
        wxString name = battle.GetUser(i).GetNick();
        if (!useractions().DoActionOnUser(UserActions::ActHighlight, name))
          return false;
      }
    }
    catch (...) {
    }
  }

  // Battle Status Check
  if (!m_filter_status_start->GetValue() && battle.GetInGame())
    return false;
  if (!m_filter_status_locked->GetValue() && battle.IsLocked())
    return false;
  if (!m_filter_status_pass->GetValue() && battle.IsPassworded())
    return false;
  if (!m_filter_status_full->GetValue() && battle.IsFull())
    return false;
  if (!m_filter_status_open->GetValue() && !battle.IsPassworded() && !battle.IsLocked() && !battle.GetInGame() &&
      !battle.IsFull())
    return false;

  // Rank Check

  /** @fixme Is `nonsenserank' useful, or can it be removed?  Why is
   * it here in the first place?
   */
  /* `Nonsense', in this context, apparently means that the battle
   * requires rank 100, exactly, AND we're filtering for values less
   * than some number.
   */
  bool nonsenserank = (m_filter_rank_mode == BUTTON_MODE_SMALLER) && (battle.GetRankNeeded() == 100);

  if (m_filter_rank_choice_value != -1 /* don't have "all" selected */
      &&
      !nonsenserank /* Nonsensical `nonsenserank' flag isn't set. */
      &&
      !_IntCompare(battle.GetRankNeeded(), m_filter_rank_choice_value, m_filter_rank_mode))
    return false;

  // Player Check
  if ((m_filter_player_choice_value != -1) &&
      !_IntCompare(battle.GetNumUsers() - battle.GetSpectators(), m_filter_player_choice_value, m_filter_player_mode))
    return false;

  // MaxPlayer Check
  if ((m_filter_maxplayer_choice_value != -1) &&
      !_IntCompare(battle.GetMaxPlayers(), m_filter_maxplayer_choice_value, m_filter_maxplayer_mode))
    return false;

  // Spectator Check
  if ((m_filter_spectator_choice_value != -1) &&
      !_IntCompare(battle.GetSpectators(), m_filter_spectator_choice_value, m_filter_spectator_mode))
    return false;

  // Only Maps i have Check
  if (m_filter_map_show->GetValue() && !battle.MapExists())
    return false;

  // Only Mods i have Check
  if (m_filter_mod_show->GetValue() && !battle.ModExists())
    return false;

  // Strings Plain Text & RegEx Check (Case insensitiv)

  // Description:
  if (!StringMatches(battle.GetDescription(), m_filter_description_edit->GetValue(), m_filter_description_expression))
    return false;

  // Host:
  try { //!TODO
    if (!StringMatches(battle.GetFounder().GetNick(), m_filter_host_edit->GetValue(), m_filter_host_expression))
      return false;
  }
  catch (...) {
  }

  // Map:
  if (!StringMatches(battle.GetHostMapName(), m_filter_map_edit->GetValue(), m_filter_map_expression))
    return false;

  // Mod:
  if (!StringMatches(battle.GetHostModName(), m_filter_mod_edit->GetValue(), m_filter_mod_expression))
    return false;

  return true;
}

void BattleListFilter::OnChange(wxCommandEvent& /*unused*/) {
  if (!m_activ)
    return;
  m_parent_battlelisttab->UpdateList();
}

void BattleListFilter::OnChangeMap(wxCommandEvent& event) {
  if (m_filter_map_edit == NULL)
    return;

  if (m_filter_map_expression == NULL)
    m_filter_map_expression = new wxRegEx(m_filter_map_edit->GetValue(), wxRE_ICASE);
  else
    m_filter_map_expression->Compile(m_filter_map_edit->GetValue(), wxRE_ICASE);

  OnChange(event);
}

void BattleListFilter::OnChangeMod(wxCommandEvent& event) {
  if (m_filter_mod_edit == NULL)
    return;

  if (m_filter_mod_expression == NULL)
    m_filter_mod_expression = new wxRegEx(m_filter_mod_edit->GetValue(), wxRE_ICASE);
  else
    m_filter_mod_expression->Compile(m_filter_mod_edit->GetValue(), wxRE_ICASE);

  OnChange(event);
}

void BattleListFilter::OnChangeDescription(wxCommandEvent& event) {
  if (m_filter_description_edit == NULL)
    return;

  if (m_filter_description_expression == NULL)
    m_filter_description_expression = new wxRegEx(m_filter_description_edit->GetValue(), wxRE_ICASE);
  else
    m_filter_description_expression->Compile(m_filter_description_edit->GetValue(), wxRE_ICASE);

  OnChange(event);
}

void BattleListFilter::OnChangeHost(wxCommandEvent& event) {
  if (m_filter_host_edit == NULL)
    return;

  if (m_filter_host_expression == NULL)
    m_filter_host_expression = new wxRegEx(m_filter_host_edit->GetValue(), wxRE_ICASE);
  else
    m_filter_host_expression->Compile(m_filter_host_edit->GetValue(), wxRE_ICASE);

  OnChange(event);
}

void BattleListFilter::OnRankChange(wxCommandEvent& event) {
  m_filter_rank_choice_value = m_filter_rank_choice->GetSelection() - 1;
  OnChange(event);
}

void BattleListFilter::OnPlayerChange(wxCommandEvent& event) {
  m_filter_player_choice_value = m_filter_player_choice->GetSelection() - 1;
  OnChange(event);
}

void BattleListFilter::OnMaxPlayerChange(wxCommandEvent& event) {
  m_filter_maxplayer_choice_value = m_filter_maxplayer_choice->GetSelection() - 1;
  OnChange(event);
}

void BattleListFilter::OnSpectatorChange(wxCommandEvent& event) {
  m_filter_spectator_choice_value = m_filter_spectator_choice->GetSelection() - 1;
  OnChange(event);
}

bool BattleListFilter::GetActiv() const { return m_activ; }

void BattleListFilter::SaveFilterValues() {
  BattleListFilterValues filtervalues;
  filtervalues.description = m_filter_description_edit->GetValue();
  filtervalues.host = m_filter_host_edit->GetValue();
  filtervalues.map = m_filter_map_edit->GetValue();
  filtervalues.map_show = m_filter_map_show->GetValue();
  filtervalues.map = m_filter_map_edit->GetValue();
  filtervalues.maxplayer = wxFormat(_("%d")) % m_filter_maxplayer_choice->GetSelection();
  filtervalues.maxplayer_mode = _GetButtonSign(m_filter_maxplayer_mode);
  filtervalues.mod = m_filter_mod_edit->GetValue();
  filtervalues.mod_show = m_filter_mod_show->GetValue();
  filtervalues.player_mode = _GetButtonSign(m_filter_player_mode);
  filtervalues.player_num = wxFormat(_("%d")) % m_filter_player_choice->GetSelection();
  filtervalues.rank = wxFormat(_("%d")) % m_filter_rank_choice->GetSelection();
  filtervalues.rank_mode = _GetButtonSign(m_filter_rank_mode);
  filtervalues.spectator = wxFormat(_("%d")) % m_filter_spectator_choice->GetSelection();
  filtervalues.spectator_mode = _GetButtonSign(m_filter_spectator_mode);
  filtervalues.status_full = m_filter_status_full->IsChecked();
  filtervalues.status_locked = m_filter_status_locked->IsChecked();
  filtervalues.status_open = m_filter_status_open->IsChecked();
  filtervalues.status_passworded = m_filter_status_pass->IsChecked();
  filtervalues.status_start = m_filter_status_start->IsChecked();
  filtervalues.highlighted_only = m_filter_highlighted->IsChecked();
  sett().SetBattleFilterValues(filtervalues);
}

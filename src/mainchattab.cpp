/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */
//
// Class: MainChatTab
//

#include <stdexcept>
#include <wx/intl.h>
#include <wx/imaglist.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/image.h>
#include <wx/log.h>

#include "aui/auimanager.h"
#include "mainchattab.h"
#include "utils/debug.h"
#include "utils/conversion.h"
#include <lslutils/misc.h>
#include "utils/controls.h"
#include "mainwindow.h"
#include "channel/channel.h"
#include "user.h"
#include "chatpanel.h"
#include "server.h"
#include "settings.h"
#include "aui/artprovider.h"
#include "aui/slbook.h"

#include "images/close.xpm"
#include "images/server.xpm"
#include "images/channel.xpm"
#include "images/userchat.xpm"

BEGIN_EVENT_TABLE(MainChatTab, wxPanel)

EVT_AUINOTEBOOK_PAGE_CHANGED(CHAT_TABS, MainChatTab::OnTabsChanged)
EVT_AUINOTEBOOK_PAGE_CLOSE(CHAT_TABS, MainChatTab::OnTabClose)

END_EVENT_TABLE()

MainChatTab::MainChatTab(wxWindow* parent)
  : wxScrolledWindow(parent, -1, wxDefaultPosition, wxDefaultSize, 0, wxPanelNameStr) {
  GetAui().manager->AddPane(this, wxLEFT, _T( "mainchattab" ));

  m_newtab_sel = -1;
  m_server_chat = 0;

  m_main_sizer = new wxBoxSizer(wxVERTICAL);

  m_chat_tabs = new SLChatNotebook(
      this, CHAT_TABS, wxDefaultPosition, wxDefaultSize,
      wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TOP | wxAUI_NB_TAB_EXTERNAL_MOVE | wxAUI_NB_WINDOWLIST_BUTTON);
  m_chat_tabs->SetArtProvider(new SLArtProvider);

  wxBitmap userchat(userchat_xpm); //*charArr2wxBitmap(userchat_png, sizeof(userchat_png) );
  m_imagelist = new wxImageList(12, 12);
  m_imagelist->Add(wxBitmap(close_xpm));
  m_imagelist->Add(wxBitmap(server_xpm));
  m_imagelist->Add(wxBitmap(channel_xpm));
  m_imagelist->Add(wxBitmap(userchat_xpm));

  m_imagelist->Add(wxBitmap(ReplaceChannelStatusColour(wxBitmap(channel_xpm), sett().GetChatColorJoinPart())));
  m_imagelist->Add(wxBitmap(ReplaceChannelStatusColour(wxBitmap(userchat_xpm), sett().GetChatColorJoinPart())));

  m_imagelist->Add(wxBitmap(ReplaceChannelStatusColour(wxBitmap(channel_xpm), sett().GetChatColorMine())));
  m_imagelist->Add(wxBitmap(ReplaceChannelStatusColour(wxBitmap(userchat_xpm), sett().GetChatColorMine())));

  m_imagelist->Add(wxBitmap(ReplaceChannelStatusColour(wxBitmap(channel_xpm), sett().GetChatColorHighlight())));
  m_imagelist->Add(wxBitmap(ReplaceChannelStatusColour(wxBitmap(userchat_xpm), sett().GetChatColorHighlight())));

  m_imagelist->Add(wxBitmap(ReplaceChannelStatusColour(wxBitmap(server_xpm), sett().GetChatColorError())));

  m_main_sizer->Add(m_chat_tabs, 1, wxEXPAND);

  SetSizer(m_main_sizer);
  m_main_sizer->SetSizeHints(this);
  SetScrollRate(SCROLL_RATE, SCROLL_RATE);

  Layout();
}

MainChatTab::~MainChatTab() {}

ChatPanel& MainChatTab::ServerChat() {
  ASSERT_LOGIC(m_server_chat != 0, _T( "m_server_chat = 0" ));
  return *m_server_chat;
}

ChatPanel* MainChatTab::GetActiveChatPanel() {
  int selection = m_chat_tabs->GetSelection();
  if (selection < 0)
    return NULL;
  else
    return static_cast<ChatPanel*>(m_chat_tabs->GetPage(selection));
}

ChatPanel* MainChatTab::GetChannelChatPanel(const wxString& channel) {
  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
    if (tmp->GetPanelType() == CPT_Channel) {
      wxString name = m_chat_tabs->GetPageText(i);
      if (name.Lower() == channel.Lower())
        return (ChatPanel*)m_chat_tabs->GetPage(i);
    }
  }
  return 0;
}

void MainChatTab::UpdateNicklistHighlights() {
  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
    if (tmp->GetPanelType() == CPT_Channel) {
      tmp->UpdateNicklistHighlights();
    }
  }
  if (m_server_chat != 0) {
    m_server_chat->UpdateNicklistHighlights();
  }
}

ChatPanel* MainChatTab::GetUserChatPanel(const wxString& user) {
  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
    if (tmp->GetPanelType() == CPT_User) {
      wxString name = m_chat_tabs->GetPageText(i);
      if (name.Lower() == user.Lower())
        return (ChatPanel*)m_chat_tabs->GetPage(i);
    }
  }
  return 0;
}

void MainChatTab::OnUserConnected(User& user) {
  ChatPanel* panel = GetUserChatPanel(user.GetNick());
  if (panel != 0) {
    panel->SetUser(&user);
    panel->OnUserConnected();
    // TODO enable send button (koshi)
  }
  if (m_server_chat != 0) {
    m_server_chat->OnChannelJoin(user);
  }
}

void MainChatTab::OnUserDisconnected(User& user) {
  ChatPanel* panel = GetUserChatPanel(user.GetNick());
  if (panel != 0) {
    panel->OnUserDisconnected();
    panel->SetUser(0);
    // TODO disable send button (koshi)
  }
  if (m_server_chat != 0) {
    m_server_chat->Parted(user, _T(""));
  }
}

void MainChatTab::LeaveChannels() {
  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
    if (tmp->GetPanelType() == CPT_Channel) {
      tmp->StatusMessage(_("Disconnected from server, chat closed."));
      tmp->SetChannel(0);
    } else if (tmp->GetPanelType() == CPT_User) {
      tmp->StatusMessage(_("Disconnected from server, chat closed."));
      tmp->SetUser(0);
    }
  }
}

void MainChatTab::RejoinChannels() {
  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
    if (tmp->GetPanelType() == CPT_Channel) {

      // TODO: This will not rejoin passworded channels.
      wxString name = m_chat_tabs->GetPageText(i);
      bool alreadyin = false;
      try {
        serverSelector().GetServer().GetChannel(name).GetMe();
        alreadyin = true;
      }
      catch (...) {
      }
      if (!alreadyin) {
        serverSelector().GetServer().JoinChannel(name, _T( "" ));
        tmp->SetChannel(&serverSelector().GetServer().GetChannel(name));
      }

    } else if (tmp->GetPanelType() == CPT_User) {

      wxString name = m_chat_tabs->GetPageText(i);
      if (serverSelector().GetServer().UserExists(name))
        tmp->SetUser(&serverSelector().GetServer().GetUser(name));
    }
  }
}

ChatPanel* MainChatTab::AddChatPanel(Channel& channel, bool doFocus) {

  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    if (m_chat_tabs->GetPageText(i) == channel.GetName()) {
      ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
      if (tmp->GetPanelType() == CPT_Channel) {
        if (doFocus)
          m_chat_tabs->SetSelection(i);
        tmp->SetChannel(&channel);
        return tmp;
      }
    }
  }

  ChatPanel* chat = new ChatPanel(m_chat_tabs, channel, m_imagelist);
  m_chat_tabs->InsertPage(m_chat_tabs->GetPageCount() - 1, chat, channel.GetName(), doFocus, wxBitmap(channel_xpm));
  if (doFocus)
    chat->FocusInputBox();
  return chat;
}

ChatPanel* MainChatTab::AddChatPanel(Server& server, const wxString& name) {

  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    // if ( m_chat_tabs->GetPageText( i ) == name ) {
    if (true) { // wipe all old server tabs
      ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
      if (tmp->GetPanelType() == CPT_Server) {
        m_chat_tabs->DeletePage(i);
        i--;
      }
    }
  }

  ChatPanel* chat = new ChatPanel(m_chat_tabs, server, m_imagelist);
  m_server_chat = chat;
  m_chat_tabs->InsertPage(m_chat_tabs->GetPageCount() - 1, chat, name, true, wxBitmap(server_xpm));
  return chat;
}

ChatPanel* MainChatTab::AddChatPanel(const User& user) {
  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    if (m_chat_tabs->GetPageText(i) == user.GetNick()) {
      ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
      if (tmp->GetPanelType() == CPT_User) {
        m_chat_tabs->SetSelection(i);
        tmp->SetUser(&user);
        return tmp;
      }
    }
  }
  int selection = m_chat_tabs->GetSelection();
  ChatPanel* chat = new ChatPanel(m_chat_tabs, user, m_imagelist);
  m_chat_tabs->InsertPage(m_chat_tabs->GetPageCount() - 1, chat, user.GetNick(), true, wxBitmap(userchat_xpm));
  if (selection > 0)
    m_chat_tabs->SetSelection(selection);
  return chat;
}

void MainChatTab::BroadcastMessage(const wxString& message) {
  // spam the message in all channels
  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
    tmp->StatusMessage(message);
  }
}

ChatPanel* MainChatTab::GetCurrentPanel() {
  int sel = m_chat_tabs->GetSelection();
  if (sel < 0)
    return 0;
  return (ChatPanel*)m_chat_tabs->GetPage(sel);
}

void MainChatTab::OnTabClose(wxAuiNotebookEvent& event) {
  int selection = event.GetSelection();
  ChatPanel* panel = (ChatPanel*)m_chat_tabs->GetPage(selection);
  if (panel) {
    panel->Part();
    if (panel->IsServerPanel())
      m_server_chat = 0;
  }
}

void MainChatTab::OnTabsChanged(wxAuiNotebookEvent& event) {
  wxLogDebugFunc(_T( "" ));

  int oldsel = event.GetOldSelection();
  if (oldsel < 0)
    return;
  int newsel = event.GetSelection();
  if (newsel < 0)
    return;

  // change icon to default the icon to show that no new events happened
  size_t ImageIndex = ((ChatPanel*)m_chat_tabs->GetPage(newsel))->GetIconIndex();
  if (ImageIndex == 4 || ImageIndex == 6 || ImageIndex == 8) {
    GetActiveChatPanel()->SetIconIndex(2);
    m_chat_tabs->SetPageBitmap(newsel, wxBitmap(channel_xpm));
  } else if (ImageIndex == 5 || ImageIndex == 7 || ImageIndex == 9) {
    GetActiveChatPanel()->SetIconIndex(3);
    m_chat_tabs->SetPageBitmap(newsel, wxBitmap(userchat_xpm));
  } else if (ImageIndex == 10) {
    GetActiveChatPanel()->SetIconIndex(1);
    m_chat_tabs->SetPageBitmap(newsel, wxBitmap(server_xpm));
  }

  wxWindow* newpage = m_chat_tabs->GetPage(newsel);
  if (newpage == 0) { // Not sure what to do here
    wxLogError(_T( "Newpage NULL." ));
    return;
  }

  GetActiveChatPanel()->FocusInputBox();
}

wxImage MainChatTab::ReplaceChannelStatusColour(wxBitmap img, const wxColour& colour) const {
  wxImage ret = img.ConvertToImage();
  wxImage::HSVValue origcolour = wxImage::RGBtoHSV(wxImage::RGBValue(colour.Red(), colour.Green(), colour.Blue()));

  double bright = origcolour.value - 0.1 * origcolour.value;
  bright = LSL::Util::Clamp(bright, 0.0, 1.0);
  wxImage::HSVValue hsvdarker1(origcolour.hue, origcolour.saturation, bright);
  bright = origcolour.value - 0.5 * origcolour.value;
  bright = LSL::Util::Clamp(bright, 0.0, 1.0);
  wxImage::HSVValue hsvdarker2(origcolour.hue, origcolour.saturation, bright);

  wxImage::RGBValue rgbdarker1 = wxImage::HSVtoRGB(hsvdarker1);
  wxImage::RGBValue rgbdarker2 = wxImage::HSVtoRGB(hsvdarker2);

  ret.Replace(164, 147, 0, rgbdarker2.red, rgbdarker2.green, rgbdarker2.blue);

  ret.Replace(255, 228, 0, rgbdarker1.red, rgbdarker1.green, rgbdarker1.blue);

  ret.Replace(255, 253, 234, colour.Red(), colour.Green(), colour.Blue());

  ret.Replace(255, 228, 0, rgbdarker1.red, rgbdarker1.green, rgbdarker1.blue);

  // server icon

  ret.Replace(211, 211, 211, rgbdarker2.red, rgbdarker2.green, rgbdarker2.blue);

  ret.Replace(249, 249, 249, rgbdarker1.red, rgbdarker1.green, rgbdarker1.blue);

  ret.Replace(0, 180, 255, colour.Red(), colour.Green(), colour.Blue());

  return ret;
}

bool MainChatTab::RemoveChatPanel(ChatPanel* panel) {
  for (unsigned int i = 0; i < m_chat_tabs->GetPageCount(); i++) {
    ChatPanel* tmp = (ChatPanel*)m_chat_tabs->GetPage(i);
    if (tmp == panel && panel != 0) {
      m_chat_tabs->DeletePage(i);
      return true;
    }
  }
  return false;
}

void MainChatTab::LoadPerspective(const wxString& perspective_name) {
  LoadNotebookPerspective(m_chat_tabs, perspective_name);
}

void MainChatTab::SavePerspective(const wxString& perspective_name) {
  SaveNotebookPerspective(m_chat_tabs, perspective_name);
}

void MainChatTab::AdvanceSelection(bool forward) { m_chat_tabs->AdvanceSelection(forward); }

/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

//
// Class: MainWindow
//

#ifdef _MSC_VER
#ifndef NOMINMAX
    #define NOMINMAX
#endif // NOMINMAX
#include <winsock2.h>
#endif // _MSC_VER

#include <wx/frame.h>
#include <wx/intl.h>
#include <wx/textdlg.h>
#include <wx/imaglist.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/log.h>
#include <wx/dcmemory.h>
#include <wx/choicdlg.h>
#include <wx/wupdlock.h>
#include <wx/aui/auibook.h>
#include <wx/tooltip.h>

#include <stdexcept>
#include <iostream>

#include "aboutbox.h"
#include "aui/auimanager.h"
#include "aui/slbook.h"
#include "gui/statusbar.h"
#include "springlobbyapp.h"
#include "mainwindow.h"
#include "settings.h"
#include "iserver.h"
#include "serverselector.h"
#include "utils/debug.h"
#include "utils/platform.h"
#include "utils/version.h"
#include "battlelist/battlelisttab.h"
#include "mainchattab.h"
#include "hosting/mainjoinbattletab.h"
#include "hosting/mainsingleplayertab.h"
#include "battlelist/battlelisttab.h"
#include "options/mainoptionstab.h"
#include "uiutils.h"
#include "chatpanel.h"
#include "playback/playbacktraits.h"
#include "playback/playbacktab.h"
#include "infodialog.h"
#include "maindownloadtab.h"
#include "user.h"
#include "mapselectdialog.h"
#include "downloader/prdownloader.h"

#include "images/chat_icon.png.h"
#include "images/join_icon.png.h"
#include "images/single_player_icon.png.h"
#include "images/options_icon.png.h"
#include "images/downloads_icon.png.h"
#include "images/replay_icon.png.h"
#include "images/broom_tab_icon.png.h"
#include "images/floppy_icon.png.h"

#include "springsettings/frame.h"
#include "utils/customdialogs.h"
#include "utils/platform.h"
#include "utils/slpaths.h"

#include "updater/updatehelper.h"
#include "channel/autojoinchanneldialog.h"
#include "channel/channelchooserdialog.h"

#if defined(__WXMSW__)
    #include <wx/msw/winundef.h>
    #include <iostream>
#endif

SLCONFIG("/GUI/UseTabIcons", true, "Show icons in main tabs");

BEGIN_EVENT_TABLE(MainWindow, wxFrame)

  EVT_MENU( MENU_JOIN,					MainWindow::OnMenuJoin				)
  EVT_MENU( MENU_CHAT,					MainWindow::OnMenuChat				)
  EVT_MENU( MENU_CONNECT,				MainWindow::OnMenuConnect			)
  EVT_MENU( MENU_DISCONNECT,			MainWindow::OnMenuDisconnect		)
  EVT_MENU( MENU_DOWNLOAD,			    MainWindow::OnMenuDownload          )
  EVT_MENU( MENU_SAVE_OPTIONS,			MainWindow::OnMenuSaveOptions		)
  EVT_MENU( MENU_QUIT,					MainWindow::OnMenuQuit				)
  EVT_MENU( MENU_USYNC,					MainWindow::OnUnitSyncReload		)
  EVT_MENU( MENU_TRAC,					MainWindow::OnReportBug				)
  EVT_MENU( MENU_DOC,					MainWindow::OnShowDocs				)
  EVT_MENU( MENU_SETTINGSPP,			MainWindow::OnShowSettingsPP		)
  EVT_MENU( MENU_VERSION,				MainWindow::OnMenuVersion			)
  EVT_MENU( MENU_ABOUT,					MainWindow::OnMenuAbout				)
  EVT_MENU( MENU_PATHINFO,				MainWindow::OnMenuPathInfo			)
  EVT_MENU( MENU_RESET_LAYOUT,			MainWindow::OnMenuResetLayout		)
//  EVT_MENU( MENU_SHOW_TOOLTIPS,		MainWindow::OnShowToolTips			)
  EVT_MENU( MENU_AUTOJOIN_CHANNELS,		MainWindow::OnMenuAutojoinChannels	)
  EVT_MENU( MENU_SELECT_LOCALE,			MainWindow::OnMenuSelectLocale		)
  EVT_MENU( MENU_CHANNELCHOOSER,		MainWindow::OnShowChannelChooser	)
  EVT_MENU( MENU_SHOWWRITEABLEDIR,      MainWindow::OnShowWriteableDir		)
  EVT_MENU( MENU_PREFERENCES,			MainWindow::OnMenuPreferences		)
  EVT_MENU( MENU_GENERAL_HELP,			MainWindow::OnMenuFirstStart		)
  EVT_MENU( MENU_SERVER_TAB,			MainWindow::OnMenuServerTab			)
  EVT_SET_FOCUS(                        MainWindow::OnSetFocus              )
  EVT_KILL_FOCUS(                       MainWindow::OnKillFocus             )
  EVT_AUINOTEBOOK_PAGE_CHANGED( MAIN_TABS, MainWindow::OnTabsChanged )
  EVT_CLOSE( MainWindow::OnClose )
END_EVENT_TABLE()

MainWindow::TabNames MainWindow::m_tab_names;

MainWindow::MainWindow( )
	: wxFrame(NULL, -1, TowxString(getSpringlobbyName()) ),
	WindowAttributesPickle( _T("MAINWINDOW"), this, wxSize(720, 576) ),
	m_opts_dialog(NULL),
	m_autojoin_dialog(NULL),
	se_frame(NULL),
	m_channel_chooser(NULL),
	m_log_win(NULL),
	m_has_focus(true)
{
	SetIcons( icons().GetIcon(icons().ICON_SPRINGLOBBY) );

	GetAui().manager = new wxAuiManager( this );

	wxMenu *menuServer = new wxMenu;
	menuServer->Append(MENU_CONNECT, _("&Connect..."));
	menuServer->Append(MENU_DISCONNECT, _("&Disconnect"));
	menuServer->Append(MENU_SERVER_TAB, _("&Reopen server panel"));
	menuServer->AppendSeparator();
#ifndef NDEBUG
	//this is a prime spot to add experimental stuff, or stubs used to test things not really meant to be in mainwindow
	menuServer->Append(MENU_SAVE_OPTIONS, _("&Save options"));
	menuServer->AppendSeparator();
#endif
	menuServer->Append(MENU_QUIT, _("&Quit"));

	m_menuEdit = new wxMenu;
	m_menuEdit->Append(MENU_AUTOJOIN_CHANNELS, _("&Autojoin channels"));
	m_menuEdit->Append(MENU_PREFERENCES, _("&Preferences"));
	m_menuEdit->Append(MENU_SELECT_LOCALE, _("&Change language"));
	m_settings_menu = new wxMenuItem( m_menuEdit, MENU_SETTINGSPP, _("&Spring settings"), wxEmptyString, wxITEM_NORMAL );
	m_menuEdit->Append( MENU_RESET_LAYOUT, _("&Reset layout") );
	m_menuEdit->Append (m_settings_menu);



	m_menuTools = new wxMenu;
	m_menuTools->Append(MENU_JOIN, _("&Join channel..."));
	m_menuTools->Append(MENU_CHANNELCHOOSER, _("Channel &list"));
	m_menuTools->Append(MENU_CHAT, _("Open private &chat..."));
	m_menuTools->Append(MENU_SHOWWRITEABLEDIR, _("&Open Spring DataDir"));
	m_menuTools->AppendSeparator();
	m_menuTools->Append(MENU_DOWNLOAD, _("&Download Archives"));
	m_menuTools->AppendSeparator();
	m_menuTools->Append(MENU_USYNC, _("&Reload maps/games"));
	m_menuTools->AppendSeparator();
	m_menuTools->Append(MENU_VERSION, _("Check for new Version"));

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(MENU_GENERAL_HELP, _("&Help, tutorial and FAQ"));
	menuHelp->Append(MENU_ABOUT, _("&About"));
	menuHelp->Append(MENU_PATHINFO, _("&System Info"));
	menuHelp->Append(MENU_TRAC, _("&Report a bug..."));
	menuHelp->Append(MENU_DOC, _("&Documentation"));

	m_menubar = new wxMenuBar;
	m_menubar->Append(menuServer, _("&Server"));
	m_menubar->Append(m_menuEdit, _("&Edit"));

	m_menubar->Append(m_menuTools, _("&Tools"));
	m_menubar->Append(menuHelp, _("&Help"));
	SetMenuBar(m_menubar);

	m_main_sizer = new wxBoxSizer( wxHORIZONTAL );
	m_func_tabs = new SLNotebook(  this, _T("mainfunctabs"), MAIN_TABS, wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_WINDOWLIST_BUTTON | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_LEFT );
	m_func_tabs->SetArtProvider(new wxAuiDefaultTabArt);

	//IMPORTANT: event handling needs to be disabled while constructing, otherwise deadlock occurs
	SetEvtHandlerEnabled( false );

	m_chat_tab = new MainChatTab( m_func_tabs );
	m_list_tab = new BattleListTab( m_func_tabs );
	m_join_tab = new MainJoinBattleTab( m_func_tabs );
	m_sp_tab = new MainSinglePlayerTab( m_func_tabs );
//	m_savegame_tab = new SavegameTab( m_func_tabs );
	m_replay_tab = new ReplayTab ( m_func_tabs );
	m_torrent_tab = new MainDownloadTab( m_func_tabs);

    //use Insert so no Changepage events are triggered
    m_func_tabs->InsertPage( PAGE_CHAT,     m_chat_tab,     m_tab_names[PAGE_CHAT],     true  );
    m_func_tabs->InsertPage( PAGE_LIST,     m_list_tab,     m_tab_names[PAGE_LIST],     false  );
    m_func_tabs->InsertPage( PAGE_JOIN,     m_join_tab,     m_tab_names[PAGE_JOIN],     false );
    m_func_tabs->InsertPage( PAGE_SINGLE,   m_sp_tab,       m_tab_names[PAGE_SINGLE],   false );
//    m_func_tabs->InsertPage( PAGE_SAVEGAME, m_savegame_tab, m_tab_names[PAGE_SAVEGAME], false );
    m_func_tabs->InsertPage( PAGE_REPLAY,   m_replay_tab,   m_tab_names[PAGE_REPLAY],   false );
    m_func_tabs->InsertPage( PAGE_TORRENT,  m_torrent_tab,  m_tab_names[PAGE_TORRENT],  false );

    LoadPerspectives();
	SetTabIcons();
	m_main_sizer->Add( m_func_tabs, 1, wxEXPAND | wxALL, 0 );

	SetSizer( m_main_sizer );

	Layout();

	se_frame_active = false;

	wxToolTip::Enable(sett().GetShowTooltips());

	m_channel_chooser = new ChannelChooserDialog( this, -1, _("Choose channels to join") );

	m_statusbar = new Statusbar( this );
    SetStatusBar( m_statusbar );
    // re-enable eventhandling
    SetEvtHandlerEnabled( true );

	UpdateMainAppHasFocus(m_has_focus);
    Connect( MainwindowMessageEvent, wxCommandEventHandler( MainWindow::OnMessage ), NULL, this );

        //this should take off the firstload time considerably *ie nil it :P )
        mapSelectDialog(true, this);
}

wxBitmap MainWindow::GetTabIcon( const unsigned char* data, size_t size ) const
{
    if ( cfg().ReadBool(_T( "/GUI/UseTabIcons" )) )
        return charArr2wxBitmap( data , size );
    else
        return wxNullBitmap;
}

void MainWindow::SetTabIcons()
{
    m_func_tabs->SetPageBitmap( PAGE_CHAT    , GetTabIcon( chat_icon_png          , sizeof(chat_icon_png)  ) );
    m_func_tabs->SetPageBitmap( PAGE_LIST    , GetTabIcon( join_icon_png          , sizeof(join_icon_png)  ) );
    m_func_tabs->SetPageBitmap( PAGE_JOIN    , GetTabIcon( broom_tab_icon_png     , sizeof(broom_tab_icon_png) ) );
    m_func_tabs->SetPageBitmap( PAGE_SINGLE  , GetTabIcon( single_player_icon_png , sizeof (single_player_icon_png) ) );
    m_func_tabs->SetPageBitmap( PAGE_TORRENT , GetTabIcon( downloads_icon_png     , sizeof (downloads_icon_png) ) );
    m_func_tabs->SetPageBitmap( PAGE_REPLAY  , GetTabIcon( replay_icon_png        , sizeof (replay_icon_png) ) );
    //m_func_tabs->SetPageBitmap( ??         , GetTabIcon( floppy_icon_png        , sizeof (floppy_icon_png) ) );
    //m_func_tabs->SetPageBitmap( ??         , GetTabIcon( options_icon_png       , sizeof (options_icon_png) ) );
    Refresh();
}

void MainWindow::forceSettingsFrameClose()
{
}

void MainWindow::SetLogWin( wxLogWindow* log, wxLogChain* logchain  )
{
    m_log_win = log;
    m_log_chain = logchain;
    if ( m_log_win )
        m_log_win->GetFrame()->SetParent( this );
}

MainWindow::~MainWindow()
{
	SetEvtHandlerEnabled( false );
	if ( m_opts_dialog )
	{
		m_opts_dialog->Show(false);
		m_opts_dialog->Destroy();
	}
}

void MainWindow::OnClose( wxCloseEvent& /*unused*/ )
{
	GlobalEvent::Send(GlobalEvent::OnQuit);
//	GlobalEvent::Disconnect(MainWindow::OnClose, GlobalEvent::)
//	SetEvtHandlerEnabled(false);
	{
		wxWindowUpdateLocker lock( this );
		SavePerspectives();
		wxAuiManager* manager=GetAui().manager;
		if(manager){
			GetAui().manager=NULL;
			manager->UnInit();
			delete manager;
		}

		ui().Quit();
		forceSettingsFrameClose();
		freeStaticBox();

		delete m_autojoin_dialog;
		m_autojoin_dialog = 0;

		if ( m_log_win ) {
			m_log_win->GetFrame()->Destroy();
			if ( m_log_chain ) // if logwin was created, it's the current "top" log
				m_log_chain->DetachOldLog();  //so we need to tellwx not to delete it on its own
				//since we absolutely need to destroy the logwin here, set a fallback for the time until app cleanup
			#if(wxUSE_STD_IOSTREAM)
				wxLog::SetActiveTarget( new wxLogStream( &std::cout ) );
			#endif
		}
	}
	Destroy();
}

void MainWindow::OnSetFocus(wxFocusEvent&)
{
	m_has_focus = true;
	UpdateMainAppHasFocus(m_has_focus);
}

void MainWindow::OnKillFocus(wxFocusEvent&)
{
	m_has_focus = false;
    UpdateMainAppHasFocus(m_has_focus);
}

void MainWindow::OnMessage(wxCommandEvent &event)
{
    customMessageBoxNoModal(SL_MAIN_ICON, event.GetString());
}

void MainWindow::AddMessageEvent(const wxString& message)
{
    wxCommandEvent evt(MainwindowMessageEvent, wxNewId());
    evt.SetString(message);
    AddPendingEvent(evt);
}

/*
bool MainWindow::HasFocus() const
{
	return m_has_focus;
}
*/
/*
void DrawBmpOnBmp( wxBitmap& canvas, wxBitmap& object, int x, int y )
{
  wxMemoryDC dc;
  dc.SelectObject( canvas );
  dc.DrawBitmap( object, x, y, true );
  dc.SelectObject( wxNullBitmap );
}
*/

//void MainWindow::DrawTxtOnBmp( wxBitmap& canvas, wxString text, int x, int y )
//{
//  wxMemoryDC dc;
//  dc.SelectObject( canvas );
//
//  dc.DrawText( text, x, y);
//  dc.SelectObject( wxNullBitmap );
//}


/*
//! @brief Get the ChatPanel dedicated to server output and input
ChatPanel& servwin()
{
  return ui().mw().GetChatTab().ServerChat();
}
*/

//! @brief Returns the curent MainChatTab object
MainChatTab& MainWindow::GetChatTab()
{
  ASSERT_EXCEPTION( m_chat_tab != 0, _T("m_chat_tab = 0") );
  return *m_chat_tab;
}

//! @brief Returns the curent BattleListTab object
BattleListTab& MainWindow::GetBattleListTab()
{
	ASSERT_EXCEPTION( m_list_tab != 0, _T( "m_list_tab = 0" ) );
	return *m_list_tab;
}

MainJoinBattleTab& MainWindow::GetJoinTab()
{
  ASSERT_EXCEPTION( m_join_tab != 0, _T("m_join_tab = 0") );
  return *m_join_tab;
}


MainSinglePlayerTab& MainWindow::GetSPTab()
{
  ASSERT_EXCEPTION( m_sp_tab != 0, _T("m_sp_tab = 0") );
  return *m_sp_tab;
}

MainWindow::ReplayTab& MainWindow::GetReplayTab()
{
    ASSERT_EXCEPTION( m_replay_tab != 0, _T("m_replay_tab = 0") );
    return *m_replay_tab;
}

//MainWindow::SavegameTab& MainWindow::GetSavegameTab()
//{
//    ASSERT_EXCEPTION( m_replay_tab != 0, _T("m_replay_tab = 0") );
//    return *m_savegame_tab;
//}

MainDownloadTab& MainWindow::GetDownloadTab()
{
  ASSERT_EXCEPTION( m_torrent_tab  != 0, _T("m_torrent_tab = 0") );
  return *m_torrent_tab ;
}

ChatPanel* MainWindow::GetActiveChatPanel()
{
  unsigned int index = m_func_tabs->GetSelection();
  if ( index == PAGE_CHAT ) return m_chat_tab->GetActiveChatPanel();
  //! TODO (koshi) this doesn't work when in broom an and sending "/help (ShowMessage() )
  if ( index == PAGE_JOIN ) return m_join_tab->GetActiveChatPanel();
  return 0;
}

ChatPanel* MainWindow::GetChannelChatPanel( const wxString& channel )
{
  return m_chat_tab->GetChannelChatPanel( channel );
}

//! @brief Open a new chat tab with a channel chat
//!
//! @param channel The channel name
//! @note This does NOT join the chatt.
//! @sa Server::JoinChannel OpenPrivateChat
void MainWindow::OpenChannelChat( Channel& channel, bool doFocus )
{
    ASSERT_LOGIC( m_chat_tab != 0, _T("m_chat_tab") );
    if ( doFocus )
        m_func_tabs->SetSelection( PAGE_CHAT );
	m_chat_tab->AddChatPanel( channel, doFocus );
}


//! @brief Open a new chat tab with a private chat
//!
//! @param nick The user to whom the chatwindow should be opened to
void MainWindow::OpenPrivateChat( const User& user, bool doFocus )
{
  ASSERT_LOGIC( m_chat_tab != 0, _T("m_chat_tab") );
  m_func_tabs->SetSelection( PAGE_CHAT );
  ChatPanel* cp = m_chat_tab->AddChatPanel( user );
  if ( doFocus )
    cp->FocusInputBox();

}

//! @brief Displays the lobby singleplayer tab.
void MainWindow::ShowSingleplayer()
{
    ShowTab( PAGE_SINGLE );
}

void MainWindow::ShowTab( const unsigned int idx )
{
    if ( idx < m_tab_names.GetCount() )
        m_func_tabs->SetSelection( idx );
    else
        wxLogError( _T("tab selection oob: %d"), idx );
}

//! @brief Displays the lobby configuration.
void MainWindow::ShowConfigure( const unsigned int page )
{
	OptionsDialog* opts = new OptionsDialog( this );
	//possibly out of bounds is captured by m_opts_tab itslef
	opts->SetSelection( page );
	opts->Show();
}

void MainWindow::ShowChannelChooser()
{
	if ( (m_channel_chooser == NULL) || (m_channel_chooser && m_channel_chooser->IsShown()) )
		return;

	if ( !ui().IsConnected() ) {
		customMessageBox( SL_MAIN_ICON, _("You need to be connected to a server to view the channel list"), _("Not connected") );
	} else {
		m_channel_chooser->ClearChannels();
		serverSelector().GetServer().RequestChannels();
		m_channel_chooser->Show( true );
	}
}

//! @brief Called when join channel menuitem is clicked
void MainWindow::OnMenuJoin( wxCommandEvent& /*unused*/ )
{

  if ( !ui().IsConnected() ) return;
  wxString answer;
  if ( ui().AskText( _("Join channel..."), _("Name of channel to join"), answer ) ) {
    ui().JoinChannel( answer, wxEmptyString );
  }

}


void MainWindow::OnMenuChat( wxCommandEvent& /*unused*/ )
{

  if ( !ui().IsConnected() ) return;
  wxString answer;
  if ( ui().AskText( _("Open Private Chat..."), _("Name of user"), answer ) ) {
	if (serverSelector().GetServer().UserExists( answer ) ) {
        //true puts focus on new tab
	  OpenPrivateChat( serverSelector().GetServer().GetUser( answer ), true  );
    }
  }

}

void MainWindow::OnMenuAbout( wxCommandEvent& /*unused*/ )
{
	aboutbox().Show();
}

void MainWindow::OnMenuConnect( wxCommandEvent& /*unused*/ )
{
  ui().ShowConnectWindow();
}


void MainWindow::OnMenuDisconnect( wxCommandEvent& /*unused*/ )
{
  ui().Disconnect();
}

void MainWindow::OnMenuServerTab( wxCommandEvent& /*unused*/ )
{
  ui().ReopenServerTab();
}

void MainWindow::OnMenuSaveOptions( wxCommandEvent& /*unused*/ )
{
  sett().SaveSettings();
}

void MainWindow::OnMenuQuit( wxCommandEvent& /*unused*/ )
{
  Close();
}

void MainWindow::OnMenuVersion( wxCommandEvent& /*unused*/ )
{
	ui().CheckForUpdates();
}

void MainWindow::OnUnitSyncReload( wxCommandEvent& /*unused*/ )
{
	LSL::usync().ReloadUnitSyncLib();
	GlobalEvent::Send(GlobalEvent::OnUnitsyncReloaded);
}

void MainWindow::MainWindow::OnShowWriteableDir( wxCommandEvent& /*unused*/ )
{
	BrowseFolder(TowxString(SlPaths::GetDataDir()));
}


void MainWindow::OnReportBug( wxCommandEvent& /*unused*/ )
{
	aboutbox().openNewTicket();
}


void MainWindow::OnShowDocs( wxCommandEvent& /*unused*/ )
{
	aboutbox().showDocs();
}

void MainWindow::OnTabsChanged( wxAuiNotebookEvent& event )
{
  int newsel = event.GetSelection();

  if ( newsel == 0 || newsel == 1 )
  {
	if ( !ui().IsConnected() && ui().IsMainWindowCreated() ) ui().Connect();
  }

	ChatPanel* panel = ui().GetActiveChatPanel(); //set input focus to edit field on tab change
	if (panel!=NULL) {
		panel->SetFocus();
	}
}

void MainWindow::OnShowSettingsPP( wxCommandEvent&  )
{
	if ( se_frame && se_frame_active ) {
		se_frame->updateAllControls();
		se_frame->Raise();
		return;
	}

	se_frame = new settings_frame(this,wxT("SpringSettings"));
	se_frame_active = true;
	se_frame->updateAllControls();
	se_frame->Show();
}

void MainWindow::OnMenuAutojoinChannels( wxCommandEvent& /*unused*/ )
{
    m_autojoin_dialog = new AutojoinChannelDialog (this);
    m_autojoin_dialog->Show();
}

void MainWindow::OnMenuSelectLocale( wxCommandEvent& /*unused*/ )
{
    if ( wxGetApp().SelectLanguage() ) {
		customMessageBoxNoModal( SL_MAIN_ICON,
								 IdentityString( _("You need to restart %s for the language change to take effect.") ),
								 _("Restart required"),
								 wxICON_EXCLAMATION | wxOK );
    }
}

void MainWindow::OnShowChannelChooser( wxCommandEvent& /*unused*/ )
{
    ShowChannelChooser();
}

void MainWindow::OnChannelList( const wxString& channel, const int& numusers, const wxString& topic )
{
    m_channel_chooser->AddChannel( channel, numusers, topic );
}

void MainWindow::OnChannelListStart( )
{
    m_channel_chooser->ClearChannels();
}

wxString MainWindow::AddPerspectivePostfix( const wxString& pers_name ) const
{
    wxString perspective_name  = pers_name.IsEmpty() ? sett().GetLastPerspectiveName() : pers_name;
    if ( m_join_tab &&  m_join_tab->UseBattlePerspective() )
        perspective_name += BattlePostfix;
    return perspective_name;
}


void MainWindow::OnMenuResetLayout( wxCommandEvent& /*event*/ )
{
	cfg().Write(_T( "/ResetLayout" ), true);
	sett().SaveSettings();
	customMessageBoxNoModal( SL_MAIN_ICON, IdentityString( _("Please restart %s now") ), wxEmptyString );
}

const MainWindow::TabNames& MainWindow::GetTabNames()
{
    return m_tab_names;
}

#ifdef __WXMSW__
	#include <hosting/battleroomtab.h>
#endif

void MainWindow::LoadPerspectives( const wxString& pers_name )
{
    sett().SetLastPerspectiveName( pers_name );
    wxString perspective_name = AddPerspectivePostfix( pers_name );

    LoadNotebookPerspective( m_func_tabs, perspective_name );
    m_sp_tab->LoadPerspective( perspective_name );
    m_join_tab->LoadPerspective( perspective_name );
    wxWindow* active_chat_tab = static_cast<wxWindow*> ( m_chat_tab->GetActiveChatPanel() );
    if ( active_chat_tab )
        active_chat_tab->Refresh();
    //chat tab saving won't work w/o further work
//    m_chat_tab->LoadPerspective( perspective_name );
}

void MainWindow::SavePerspectives( const wxString& pers_name )
{
    sett().SetLastPerspectiveName( pers_name );
    wxString perspective_name = AddPerspectivePostfix( pers_name );

    m_sp_tab->SavePerspective( perspective_name );
    m_join_tab->SavePerspective( perspective_name );
//    m_chat_tab->SavePerspective( perspective_name );
    SaveNotebookPerspective( m_func_tabs, perspective_name );
}

void MainWindow::FocusBattleRoomTab()
{
	m_func_tabs->SetSelection( PAGE_JOIN );
	GetJoinTab().FocusBattleRoomTab();
}

void MainWindow::OnMenuPreferences( wxCommandEvent& /*event*/ )
{
	assert(wxThread::IsMain());
	m_opts_dialog = new OptionsDialog( this );
	m_opts_dialog->Show();
}

void MainWindow::OnMenuFirstStart( wxCommandEvent& /*event*/ )
{
	OpenWebBrowser( _T("https://github.com/springlobby/springlobby/wiki/Userdoc") );
}

void MainWindow::OnMenuPathInfo( wxCommandEvent& /*event*/ )
{
	InfoDialog( this ).ShowModal();
}

void MainWindow::OnMenuDownload( wxCommandEvent& /*event*/ )
{
	wxString lines;
	if ( !ui().AskText( _( "Which Archives to download? Put each archive on a single line, for example \ngame:ba:stable\nmap:The Rock Final" ), _( "Download Archives" ), lines, true ) ) return;
	size_t start = 0;
	size_t pos = 0;
	do {
		pos = lines.find('\n', start);
		wxString line = lines.substr(start, pos-start);
		line.Trim(true).Trim(false);
		const std::string category = STD_STRING(line.BeforeFirst(':'));
		const std::string archive = STD_STRING(line.AfterFirst(':'));
		if (!category.empty() && !archive.empty()) {
			prDownloader().GetDownload(category, archive);
		}
		start = pos+1;
	} while (pos != wxNOT_FOUND);
}

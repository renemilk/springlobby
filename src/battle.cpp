/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */
//
// Class: Battle
//
#include "battle.h"
#include "ui.h"
#include "server.h"
#include "user.h"
#include "utils/misc.h"
#include "utils/debug.h"
#include "utils/conversion.h"
#include <lslutils/misc.h>
#include "utils/uievents.h"
#include "utils/battleevents.h"
#include "uiutils.h"
#include "settings.h"
#include "useractions.h"
#include "utils/customdialogs.h"
#include <lslutils/conversion.h>
#include "iconimagelist.h"
#include "spring.h"

#include <wx/timer.h>
#include <wx/image.h>
#include <wx/string.h>
#include <wx/log.h>
#include <wx/filename.h>

const unsigned int TIMER_INTERVAL = 1000;
const unsigned int TIMER_ID = 101;

BEGIN_EVENT_TABLE(Battle, wxEvtHandler)
EVT_TIMER(TIMER_ID, Battle::OnTimer)
END_EVENT_TABLE()

Battle::Battle(Server& serv, int id)
  : IBattle()
  , m_serv(serv)
  , m_ah(*this)
  , m_autolock_on_start(false)
  , m_id(id) {
  ConnectGlobalEvent(this, GlobalEvent::OnUnitsyncReloaded, wxObjectEventFunction(&Battle::OnUnitsyncReloaded));
  m_opts.battleid = m_id;
}

Battle::~Battle() {}

void Battle::SendHostInfo(HostInfo update) { m_serv.SendHostInfo(update); }

void Battle::SendHostInfo(const wxString& Tag) { m_serv.SendHostInfo(Tag); }

void Battle::Update() {
  BattleEvents::GetBattleEventSender(BattleEvents::BattleInfoUpdate).SendEvent(std::make_pair(this, wxString()));
}

void Battle::Update(const wxString& Tag) {
  BattleEvents::GetBattleEventSender(BattleEvents::BattleInfoUpdate).SendEvent(std::make_pair(this, Tag));
}

void Battle::Join(const wxString& password) {
  m_serv.JoinBattle(m_opts.battleid, password);
  m_is_self_in = true;
}

void Battle::Leave() { m_serv.LeaveBattle(m_opts.battleid); }

void Battle::OnRequestBattleStatus() {
  UserBattleStatus& bs = m_serv.GetMe().BattleStatus();
  bs.team = GetFreeTeam(true);
  bs.ally = GetFreeAlly(true);
  bs.spectator = false;
  bs.colour = sett().GetBattleLastColour();
  bs.side = sett().GetBattleLastSideSel(GetHostModName());
  // theres some highly annoying bug with color changes on player join/leave.
  if (!bs.colour.IsOk())
    bs.colour = GetFreeColour(GetMe());

  SendMyBattleStatus();
}

void Battle::SendMyBattleStatus() {
  UserBattleStatus& bs = m_serv.GetMe().BattleStatus();
  if (IsSynced())
    bs.sync = SYNC_SYNCED;
  else
    bs.sync = SYNC_UNSYNCED;
  m_serv.SendMyBattleStatus(bs);
}

void Battle::SetImReady(bool ready) {
  UserBattleStatus& bs = m_serv.GetMe().BattleStatus();

  bs.ready = ready;

  // m_serv.GetMe().SetBattleStatus( bs );
  SendMyBattleStatus();
}

/*bool Battle::HasMod()
{
  return LSL::usync().ModExists( m_opts.modname );
}*/

void Battle::Say(const wxString& msg) { m_serv.SayBattle(m_opts.battleid, msg); }

void Battle::DoAction(const wxString& msg) { m_serv.DoActionBattle(m_opts.battleid, msg); }

void Battle::SetLocalMap(const LSL::UnitsyncMap& map) {
  IBattle::SetLocalMap(map);
  if (IsFounderMe())
    LoadMapDefaults(TowxString(map.name));
}

User& Battle::GetMe() { return m_serv.GetMe(); }

const User& Battle::GetMe() const { return m_serv.GetMe(); }

void Battle::SaveMapDefaults() {
  // save map preset
  const wxString mapname = TowxString(LoadMap().name);
  const std::string startpostype =
      CustomBattleOptions().getSingleValue("startpostype", LSL::OptionsWrapper::EngineOption);
  sett().SetMapLastStartPosType(mapname, TowxString(startpostype));
  std::vector<Settings::SettStartBox> rects;
  for (unsigned int i = 0; i <= GetLastRectIdx(); ++i) {
    BattleStartRect rect = GetStartRect(i);
    if (rect.IsOk()) {
      Settings::SettStartBox box;
      box.ally = rect.ally;
      box.topx = rect.left;
      box.topy = rect.top;
      box.bottomx = rect.right;
      box.bottomy = rect.bottom;
      rects.push_back(box);
    }
  }
  sett().SetMapLastRectPreset(mapname, rects);
}

void Battle::LoadMapDefaults(const wxString& mapname) {
  CustomBattleOptions().setSingleOption("startpostype", STD_STRING(sett().GetMapLastStartPosType(mapname)),
                                        LSL::OptionsWrapper::EngineOption);
  SendHostInfo(wxFormat(_T("%d_startpostype")) % LSL::OptionsWrapper::EngineOption);

  for (unsigned int i = 0; i <= GetLastRectIdx(); ++i)
    if (GetStartRect(i).IsOk())
      RemoveStartRect(i); // remove all rects
  SendHostInfo(IBattle::HI_StartRects);

  std::vector<Settings::SettStartBox> savedrects = sett().GetMapLastRectPreset(mapname);
  for (std::vector<Settings::SettStartBox>::const_iterator itor = savedrects.begin(); itor != savedrects.end();
       ++itor) {
    AddStartRect(itor->ally, itor->topx, itor->topy, itor->bottomx, itor->bottomy);
  }
  SendHostInfo(IBattle::HI_StartRects);
}

User& Battle::OnUserAdded(User& user) {
  user = IBattle::OnUserAdded(user);
  if (&user == &GetMe() && (m_timer == NULL)) {
    m_timer = new wxTimer(this, TIMER_ID);
    m_timer->Start(TIMER_INTERVAL);
  }
  user.SetBattle(this);
  user.BattleStatus().isfromdemo = false;

  if (IsFounderMe()) {
    if (CheckBan(user))
      return user;

    if ((&user != &GetMe()) && !user.BattleStatus().IsBot() && (m_opts.rankneeded != UserStatus::RANK_1) &&
        !user.BattleStatus().spectator) {
      if (m_opts.rankneeded > UserStatus::RANK_1 && user.GetStatus().rank < m_opts.rankneeded) {
        DoAction(_T("Rank limit autospec: ") + user.GetNick());
        ForceSpectator(user, true);
      } else if (m_opts.rankneeded < UserStatus::RANK_1 && user.GetStatus().rank > (-m_opts.rankneeded - 1)) {
        DoAction(_T("Rank limit autospec: ") + user.GetNick());
        ForceSpectator(user, true);
      }
    }

    m_ah.OnUserAdded(user);
    if (!user.BattleStatus().IsBot() && sett().GetBattleLastAutoAnnounceDescription())
      DoAction(m_opts.description);
  }
  // any code here may be skipped if the user was autokicked
  return user;
}

void Battle::OnUserBattleStatusUpdated(User& user, UserBattleStatus status) {
  if (IsFounderMe()) {
    if ((&user != &GetMe()) && !status.IsBot() && (m_opts.rankneeded != UserStatus::RANK_1) && !status.spectator) {
      if (m_opts.rankneeded > UserStatus::RANK_1 && user.GetStatus().rank < m_opts.rankneeded) {
        DoAction(_T("Rank limit autospec: ") + user.GetNick());
        ForceSpectator(user, true);
      } else if (m_opts.rankneeded < UserStatus::RANK_1 && user.GetStatus().rank > (-m_opts.rankneeded - 1)) {
        DoAction(_T("Rank limit autospec: ") + user.GetNick());
        ForceSpectator(user, true);
      }
    }
    UserBattleStatus previousstatus = user.BattleStatus();
    if (m_opts.lockexternalbalancechanges) {
      if (previousstatus.team != status.team) {
        ForceTeam(user, previousstatus.team);
        status.team = previousstatus.team;
      }
      if (previousstatus.ally != status.ally) {
        ForceAlly(user, previousstatus.ally);
        status.ally = previousstatus.ally;
      }
    }
  }
  IBattle::OnUserBattleStatusUpdated(user, status);
  if (status.handicap != 0) {
    UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent).SendEvent(UiEvents::OnBattleActionData(
        wxString(_T(" ")), (_T("Warning: user ") + user.GetNick() + _T(" got bonus ")) << status.handicap));
  }
  if (IsFounderMe()) {
    if (ShouldAutoStart()) {
      if (sett().GetBattleLastAutoStartState()) {
        if (!spring().IsRunning())
          StartHostedBattle();
      }
    }
  }
  ShouldAutoUnspec();
  ui().OnUserBattleStatus(*this, user);
}

void Battle::OnUserRemoved(User& user) {
  m_ah.OnUserRemoved(user);
  IBattle::OnUserRemoved(user);
  ShouldAutoUnspec();
}

void Battle::RingNotReadyPlayers() {
  for (user_map_t::size_type i = 0; i < GetNumUsers(); i++) {
    User& u = GetUser(i);
    UserBattleStatus& bs = u.BattleStatus();
    if (bs.IsBot())
      continue;
    if (!bs.ready && !bs.spectator)
      m_serv.Ring(u.GetNick());
  }
}

void Battle::RingNotSyncedPlayers() {
  for (user_map_t::size_type i = 0; i < GetNumUsers(); i++) {
    User& u = GetUser(i);
    UserBattleStatus& bs = u.BattleStatus();
    if (bs.IsBot())
      continue;
    if (!bs.sync && !bs.spectator)
      m_serv.Ring(u.GetNick());
  }
}

void Battle::RingNotSyncedAndNotReadyPlayers() {
  for (user_map_t::size_type i = 0; i < GetNumUsers(); i++) {
    User& u = GetUser(i);
    UserBattleStatus& bs = u.BattleStatus();
    if (bs.IsBot())
      continue;
    if ((!bs.sync || !bs.ready) && !bs.spectator)
      m_serv.Ring(u.GetNick());
  }
}

void Battle::RingPlayer(const User& u) {
  if (u.BattleStatus().IsBot())
    return;
  m_serv.Ring(u.GetNick());
}

bool Battle::ExecuteSayCommand(const wxString& cmd) {
  wxString cmd_name = cmd.BeforeFirst(' ').Lower();
  if (cmd_name == _T("/me")) {
    m_serv.DoActionBattle(m_opts.battleid, cmd.AfterFirst(' '));
    return true;
  }
  if (cmd_name == _T("/replacehostip")) {
    wxString ip = cmd.AfterFirst(' ');
    if (ip.IsEmpty())
      return false;
    m_opts.ip = ip;
    return true;
  }
  //< quick hotfix for bans
  if (IsFounderMe()) {
    if (cmd_name == _T("/ban")) {
      wxString nick = cmd.AfterFirst(' ');
      m_banned_users.insert(nick);
      try {
        User& user = GetUser(nick);
        m_serv.BattleKickPlayer(m_opts.battleid, user);
      }
      catch (assert_exception) {
      }
      UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
          .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), nick + _T(" banned")));

      // m_serv.DoActionBattle( m_opts.battleid, cmd.AfterFirst(' ') );
      return true;
    }
    if (cmd_name == _T("/unban")) {
      wxString nick = cmd.AfterFirst(' ');
      m_banned_users.erase(nick);
      UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
          .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), nick + _T(" unbanned")));
      // m_serv.DoActionBattle( m_opts.battleid, cmd.AfterFirst(' ') );
      return true;
    }
    if (cmd_name == _T("/banlist")) {
      UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
          .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), _T("banlist:")));

      for (std::set<wxString>::const_iterator i = m_banned_users.begin(); i != m_banned_users.end(); ++i) {
        UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
            .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), *i));
      }
      for (std::set<wxString>::iterator i = m_banned_ips.begin(); i != m_banned_ips.end(); ++i) {
        UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
            .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), *i));
      }
      return true;
    }
    if (cmd_name == _T("/unban")) {
      wxString nick = cmd.AfterFirst(' ');
      m_banned_users.erase(nick);
      m_banned_ips.erase(nick);
      UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
          .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), nick + _T(" unbanned")));

      // m_serv.DoActionBattle( m_opts.battleid, cmd.AfterFirst(' ') );
      return true;
    }
    if (cmd_name == _T("/ipban")) {
      wxString nick = cmd.AfterFirst(' ');
      m_banned_users.insert(nick);
      UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
          .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), nick + _T(" banned")));

      if (UserExists(nick)) {
        User& user = GetUser(nick);
        if (!user.BattleStatus().ip.empty()) {
          m_banned_ips.insert(user.BattleStatus().ip);
          UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
              .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), user.BattleStatus().ip + _T(" banned")));
        }
        m_serv.BattleKickPlayer(m_opts.battleid, user);
      }
      // m_banned_ips.erase(nick);

      // m_serv.DoActionBattle( m_opts.battleid, cmd.AfterFirst(' ') );
      return true;
    }
  }
  //>
  return false;
}
///< quick hotfix for bans
/// returns true if user is banned (and hence has been kicked)
bool Battle::CheckBan(User& user) {
  if (IsFounderMe()) {
    if (m_banned_users.count(user.GetNick()) > 0 ||
        useractions().DoActionOnUser(UserActions::ActAutokick, user.GetNick())) {
      KickPlayer(user);
      UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent)
          .SendEvent(UiEvents::OnBattleActionData(wxString(_T(" ")), user.GetNick() + _T(" is banned, kicking")));
      return true;
    } else if (m_banned_ips.count(user.BattleStatus().ip) > 0) {
      UiEvents::GetUiEventSender(UiEvents::OnBattleActionEvent).SendEvent(
          UiEvents::OnBattleActionData(wxString(_T(" ")), user.BattleStatus().ip + _T(" is banned, kicking")));
      KickPlayer(user);
      return true;
    }
  }
  return false;
}
///>

void Battle::SetAutoLockOnStart(bool value) { m_autolock_on_start = value; }

bool Battle::GetAutoLockOnStart() { return m_autolock_on_start; }

void Battle::SetLockExternalBalanceChanges(bool value) {
  if (value)
    DoAction(_T("has locked player balance changes"));
  else
    DoAction(_T("has unlocked player balance changes"));
  m_opts.lockexternalbalancechanges = value;
}

bool Battle::GetLockExternalBalanceChanges() { return m_opts.lockexternalbalancechanges; }

void Battle::AddBot(const wxString& nick, UserBattleStatus status) { m_serv.AddBot(m_opts.battleid, nick, status); }

void Battle::ForceSide(User& user, int side) { m_serv.ForceSide(m_opts.battleid, user, side); }

void Battle::ForceTeam(User& user, int team) {
  IBattle::ForceTeam(user, team);
  m_serv.ForceTeam(m_opts.battleid, user, team);
}

void Battle::ForceAlly(User& user, int ally) {
  IBattle::ForceAlly(user, ally);
  m_serv.ForceAlly(m_opts.battleid, user, ally);
}

void Battle::ForceColour(User& user, const wxColour& col) {
  IBattle::ForceColour(user, col);
  m_serv.ForceColour(m_opts.battleid, user, col);
}

void Battle::ForceSpectator(User& user, bool spectator) { m_serv.ForceSpectator(m_opts.battleid, user, spectator); }

void Battle::KickPlayer(User& user) { m_serv.BattleKickPlayer(m_opts.battleid, user); }

void Battle::SetHandicap(User& user, int handicap) { m_serv.SetHandicap(m_opts.battleid, user, handicap); }

void Battle::ForceUnsyncedToSpectate() {
  size_t numusers = GetNumUsers();
  for (size_t i = 0; i < numusers; ++i) {
    User& user = GetUser(i);
    UserBattleStatus& bs = user.BattleStatus();
    if (bs.IsBot())
      continue;
    if (!bs.spectator && !bs.sync)
      ForceSpectator(user, true);
  }
}

void Battle::ForceUnReadyToSpectate() {
  size_t numusers = GetNumUsers();
  for (size_t i = 0; i < numusers; ++i) {
    User& user = GetUser(i);
    UserBattleStatus& bs = user.BattleStatus();
    if (bs.IsBot())
      continue;
    if (!bs.spectator && !bs.ready)
      ForceSpectator(user, true);
  }
}

void Battle::ForceUnsyncedAndUnreadyToSpectate() {
  size_t numusers = GetNumUsers();
  for (size_t i = 0; i < numusers; ++i) {
    User& user = GetUser(i);
    UserBattleStatus& bs = user.BattleStatus();
    if (bs.IsBot())
      continue;
    if (!bs.spectator && (!bs.sync || !bs.ready))
      ForceSpectator(user, true);
  }
}

void Battle::UserPositionChanged(const User& user) { m_serv.SendUserPosition(user); }

void Battle::SendScriptToClients() { m_serv.SendScriptToClients(GetScript()); }

void Battle::StartHostedBattle() {
  if (UserExists(GetMe().GetNick())) {
    if (IsFounderMe()) {
      if (sett().GetBattleLastAutoControlState()) {
        FixTeamIDs((IBattle::BalanceType)sett().GetFixIDMethod(), sett().GetFixIDClans(), sett().GetFixIDStrongClans(),
                   sett().GetFixIDGrouping());
        Autobalance((IBattle::BalanceType)sett().GetBalanceMethod(), sett().GetBalanceClans(),
                    sett().GetBalanceStrongClans(), sett().GetBalanceGrouping());
        FixColours();
      }
      if (IsProxy()) {
        if (UserExists(GetProxy()) && !GetUser(GetProxy()).Status().in_game) {
          // DON'T set m_generating_script here, it will trick the script generating code to think we're the host
          wxString hostscript = spring().WriteScriptTxt(*this);
          try {
            wxString path =
                sett().GetCurrentUsedDataDir() + wxFileName::GetPathSeparator() + _T("relayhost_script.txt");
            if (!wxFile::Access(path, wxFile::write)) {
              wxLogError(_T("Access denied to script.txt."));
            }

            wxFile f(path, wxFile::write);
            f.Write(hostscript);
            f.Close();
          }
          catch (...) {
          }
          m_serv.SendScriptToProxy(hostscript);
        }
      }
      if (GetAutoLockOnStart()) {
        SetIsLocked(true);
        SendHostInfo(IBattle::HI_Locked);
      }
      sett().SetLastHostMap(GetServer().GetCurrentBattle()->GetHostMapName());
      sett().SaveSettings();
      if (!IsProxy())
        GetServer().StartHostedBattle();
      else if (UserExists(GetProxy()) &&
               GetUser(GetProxy()).Status().in_game) // relayhost is already ingame, let's try to join it
      {
        StartSpring();
      }
    }
  }
}

void Battle::StartSpring() {
  if (UserExists(GetMe().GetNick()) && !GetMe().Status().in_game) {
    GetMe().BattleStatus().ready = false;
    SendMyBattleStatus();
    // set m_generating_script, this will make the script.txt writer realize we're just clients even if using a
    // relayhost
    m_generating_script = true;
    GetMe().Status().in_game = spring().Run(*this);
    m_generating_script = false;
    GetMe().SendMyUserStatus();
  }
  ui().OnBattleStarted(*this);
}

void Battle::OnTimer(wxTimerEvent&) {
  if (!IsFounderMe())
    return;
  if (m_ingame)
    return;
  int autospect_trigger_time = sett().GetBattleLastAutoSpectTime();
  if (autospect_trigger_time == 0)
    return;
  time_t now = time(0);
  for (unsigned int i = 0; i < GetNumUsers(); i++) {
    User& usr = GetUser(i);
    UserBattleStatus& status = usr.BattleStatus();
    if (status.IsBot() || status.spectator)
      continue;
    if (status.sync && status.ready)
      continue;
    if (&usr == &GetMe())
      continue;
    std::map<wxString, time_t>::const_iterator itor = m_ready_up_map.find(usr.GetNick());
    if (itor != m_ready_up_map.end()) {
      if ((now - itor->second) > autospect_trigger_time) {
        ForceSpectator(usr, true);
      }
    }
  }
}

void Battle::SetInGame(bool value) {
  time_t now = time(0);
  if (m_ingame && !value) {
    for (int i = 0; i < long(GetNumUsers()); i++) {
      User& user = GetUser(i);
      UserBattleStatus& status = user.BattleStatus();
      if (status.IsBot() || status.spectator)
        continue;
      if (status.ready && status.sync)
        continue;
      m_ready_up_map[user.GetNick()] = now;
    }
  }
  IBattle::SetInGame(value);
}

void Battle::FixColours() {
  if (!IsFounderMe())
    return;
  std::vector<wxColour>& palette = GetFixColoursPalette(m_teams_sizes.size() + 1);
  std::vector<int> palette_use(palette.size(), 0);

  wxColour my_col = GetMe().BattleStatus().colour; // Never changes color of founder (me) :-)
  int my_diff = 0;
  int my_col_i = GetClosestFixColour(my_col, palette_use, my_diff);
  palette_use[my_col_i]++;
  std::set<int> parsed_teams;

  for (user_map_t::size_type i = 0; i < GetNumUsers(); i++) {
    User& user = GetUser(i);
    if (&user == &GetMe())
      continue; // skip founder ( yourself )
    UserBattleStatus& status = user.BattleStatus();
    if (status.spectator)
      continue;
    if (parsed_teams.find(status.team) != parsed_teams.end())
      continue; // skip duplicates
    parsed_teams.insert(status.team);

    wxColour& user_col = status.colour;
    int user_col_i = GetClosestFixColour(user_col, palette_use, 60);
    palette_use[user_col_i]++;
    for (user_map_t::size_type j = 0; j < GetNumUsers(); j++) {
      User& usr = GetUser(j);
      if (usr.BattleStatus().team == status.team) {
        ForceColour(usr, palette[user_col_i]);
      }
    }
  }
}

bool PlayerRankCompareFunction(User* a,
                               User* b) // should never operate on nulls. Hence, ASSERT_LOGIC is appropriate here.
{
  ASSERT_LOGIC(a, _T("fail in Autobalance, NULL player"));
  ASSERT_LOGIC(b, _T("fail in Autobalance, NULL player"));
  return (a->GetBalanceRank() > b->GetBalanceRank());
}

bool PlayerTeamCompareFunction(User* a,
                               User* b) // should never operate on nulls. Hence, ASSERT_LOGIC is appropriate here.
{
  ASSERT_LOGIC(a, _T("fail in Autobalance, NULL player"));
  ASSERT_LOGIC(b, _T("fail in Autobalance, NULL player"));
  return (a->BattleStatus().team > b->BattleStatus().team);
}

struct Alliance {
  std::vector<User*> players;
  float ranksum;
  int allynum;
  Alliance()
    : ranksum(0)
    , allynum(-1) {}
  Alliance(int i)
    : ranksum(0)
    , allynum(i) {}
  void AddPlayer(User* player) {
    if (player) {
      players.push_back(player);
      ranksum += player->GetBalanceRank();
    }
  }
  void AddAlliance(Alliance& other) {
    for (std::vector<User*>::const_iterator i = other.players.begin(); i != other.players.end(); ++i)
      AddPlayer(*i);
  }
  bool operator<(const Alliance& other) const { return ranksum < other.ranksum; }
};

struct ControlTeam {
  std::vector<User*> players;
  float ranksum;
  int teamnum;
  ControlTeam()
    : ranksum(0)
    , teamnum(-1) {}
  ControlTeam(int i)
    : ranksum(0)
    , teamnum(i) {}
  void AddPlayer(User* player) {
    if (player) {
      players.push_back(player);
      ranksum += player->GetBalanceRank();
    }
  }
  void AddTeam(ControlTeam& other) {
    for (std::vector<User*>::const_iterator i = other.players.begin(); i != other.players.end(); ++i)
      AddPlayer(*i);
  }
  bool operator<(const ControlTeam& other) const { return ranksum < other.ranksum; }
};

int my_random(int range) { return rand() % range; }

void shuffle(std::vector<User*>& players) // proper shuffle.
{
  for (size_t i = 0; i < players.size(); ++i) // the players below i are shuffled, the players above arent
  {
    int rn = i + my_random(players.size() - i); // the top of shuffled part becomes random card from unshuffled part
    User* tmp = players[i];
    players[i] = players[rn];
    players[rn] = tmp;
  }
}

/*
bool ClanRemovalFunction(const std::map<wxString, Alliance>::value_type &v){
  return v.second.players.size()<2;
}
*/
/*
struct ClannersRemovalPredicate{
  std::map<wxString, Alliance> &clans;
  PlayerRemovalPredicate(std::map<wxString, Alliance> &clans_):clans(clans_)
  {
  }
  bool operator()(User *u) const{
    return clans.find(u->GetClan());
  }
}*/

void Battle::Autobalance(BalanceType balance_type, bool support_clans, bool strong_clans, int numallyteams) {
  wxLogMessage(_T("Autobalancing alliances, type=%d, clans=%d, strong_clans=%d, numallyteams=%d"), balance_type,
               support_clans, strong_clans, numallyteams);
  // size_t i;
  // int num_alliances;
  std::vector<Alliance> alliances;
  if (numallyteams == 0 || numallyteams == -1) // 0 or 1 -> use num start rects
  {
    int ally = 0;
    for (unsigned int i = 0; i < GetNumRects(); ++i) {
      BattleStartRect sr = GetStartRect(i);
      if (sr.IsOk()) {
        ally = i;
        alliances.push_back(Alliance(ally));
        ally++;
      }
    }
    // make at least two alliances
    while (alliances.size() < 2) {
      alliances.push_back(Alliance(ally));
      ally++;
    }
  } else {
    for (int i = 0; i < numallyteams; i++)
      alliances.push_back(Alliance(i));
  }

  // for(i=0;i<alliances.size();++i)alliances[i].allynum=i;

  wxLogMessage(_T("number of alliances: %u"), alliances.size());

  std::vector<User*> players_sorted;
  players_sorted.reserve(GetNumUsers());

  for (size_t i = 0; i < GetNumUsers(); ++i) {
    User& usr = GetUser(i);
    if (!usr.BattleStatus().spectator) {
      players_sorted.push_back(&usr);
    }
  }

  // remove players in the same team so only one remains
  std::map<int, User*> dedupe_teams;
  for (std::vector<User*>::const_iterator it = players_sorted.begin(); it != players_sorted.end(); ++it) {
    dedupe_teams[(*it)->BattleStatus().team] = *it;
  }
  players_sorted.clear();
  players_sorted.reserve(dedupe_teams.size());
  for (std::map<int, User*>::const_iterator it = dedupe_teams.begin(); it != dedupe_teams.end(); ++it) {
    players_sorted.push_back(it->second);
  }

  shuffle(players_sorted);

  std::map<wxString, Alliance> clan_alliances;
  if (support_clans) {
    for (size_t i = 0; i < players_sorted.size(); ++i) {
      wxString clan = players_sorted[i]->GetClan();
      if (!clan.empty()) {
        clan_alliances[clan].AddPlayer(players_sorted[i]);
      }
    }
  }
  ;

  if (balance_type != balance_random)
    std::sort(players_sorted.begin(), players_sorted.end(), PlayerRankCompareFunction);

  if (support_clans) {
    std::map<wxString, Alliance>::iterator clan_it = clan_alliances.begin();
    while (clan_it != clan_alliances.end()) {
      Alliance& clan = (*clan_it).second;
      // if clan is too small (only 1 clan member in battle) or too big, dont count it as clan
      if ((clan.players.size() < 2) ||
          (!strong_clans &&
           (clan.players.size() > ((players_sorted.size() + alliances.size() - 1) / alliances.size())))) {
        wxLogMessage(_T("removing clan %s"), (*clan_it).first.c_str());
        std::map<wxString, Alliance>::iterator next = clan_it;
        ++next;
        clan_alliances.erase(clan_it);
        clan_it = next;
        continue;
      }
      wxLogMessage(_T("Inserting clan %s"), (*clan_it).first.c_str());
      std::sort(alliances.begin(), alliances.end());
      float lowestrank = alliances[0].ranksum;
      int rnd_k = 1; // number of alliances with rank equal to lowestrank
      while (size_t(rnd_k) < alliances.size()) {
        if (fabs(alliances[rnd_k].ranksum - lowestrank) > 0.01)
          break;
        rnd_k++;
      }
      wxLogMessage(_T("number of lowestrank alliances with same rank=%d"), rnd_k);
      alliances[my_random(rnd_k)].AddAlliance(clan);
      ++clan_it;
    }
  }

  for (size_t i = 0; i < players_sorted.size(); ++i) {
    // skip clanners, those have been added already.
    if (clan_alliances.count(players_sorted[i]->GetClan()) > 0) {
      wxLogMessage(_T("clanner already added, nick=%s"), players_sorted[i]->GetNick().c_str());
      continue;
    }

    // find alliances with lowest ranksum
    // insert current user into random one out of them
    // since performance doesnt matter here, i simply sort alliances,
    // then find how many alliances in beginning have lowest ranksum
    // note that balance player ranks range from 1 to 1.1 now
    // i.e. them are quasi equal
    // so we're essentially adding to alliance with smallest number of players,
    // the one with smallest ranksum.

    std::sort(alliances.begin(), alliances.end());
    float lowestrank = alliances[0].ranksum;
    int rnd_k = 1; // number of alliances with rank equal to lowestrank
    while (size_t(rnd_k) < alliances.size()) {
      if (fabs(alliances[rnd_k].ranksum - lowestrank) > 0.01)
        break;
      rnd_k++;
    }
    wxLogMessage(_T("number of lowestrank alliances with same rank=%d"), rnd_k);
    alliances[my_random(rnd_k)].AddPlayer(players_sorted[i]);
  }

  UserList::user_map_t::size_type totalplayers = GetNumUsers();
  for (size_t i = 0; i < alliances.size(); ++i) {
    for (size_t j = 0; j < alliances[i].players.size(); ++j) {
      ASSERT_LOGIC(alliances[i].players[j], _T("fail in Autobalance, NULL player"));
      int balanceteam = alliances[i].players[j]->BattleStatus().team;
      wxLogMessage(_T("setting team %d to alliance %d"), balanceteam, i);
      for (size_t h = 0; h < totalplayers; h++) // change ally num of all players in the team
      {
        User& usr = GetUser(h);
        if (usr.BattleStatus().team == balanceteam)
          ForceAlly(usr, alliances[i].allynum);
      }
    }
  }
}

void Battle::FixTeamIDs(BalanceType balance_type, bool support_clans, bool strong_clans, int numcontrolteams) {
  wxLogMessage(_T("Autobalancing teams, type=%d, clans=%d, strong_clans=%d, numcontrolteams=%d"), balance_type,
               support_clans, strong_clans, numcontrolteams);
  // size_t i;
  // int num_alliances;
  std::vector<ControlTeam> control_teams;

  if (numcontrolteams == 0 || numcontrolteams == -1)
    numcontrolteams = GetNumUsers() -
                      GetSpectators(); // 0 or -1 -> use num players, will use comshare only if no available team slots
  IBattle::StartType position_type = (IBattle::StartType)LSL::Util::FromString<long>(
      CustomBattleOptions().getSingleValue("startpostype", LSL::OptionsWrapper::EngineOption));
  if ((position_type == ST_Fixed) ||
      (position_type == ST_Random)) // if fixed start pos type or random, use max teams = start pos count
  {
    try {
      int mapposcount = LoadMap().info.positions.size();
      numcontrolteams = std::min(numcontrolteams, mapposcount);
    }
    catch (assert_exception) {
    }
  }

  if (numcontrolteams >= (int)(GetNumUsers() - GetSpectators())) // autobalance behaves weird when trying to put one
                                                                 // player per team and i CBA to fix it, so i'll reuse
                                                                 // the old code :P
  {
    // apparently tasserver doesnt like when i fix/force ids of everyone.
    std::set<int> allteams;
    size_t numusers = GetNumUsers();
    for (size_t i = 0; i < numusers; ++i) {
      User& user = GetUser(i);
      if (!user.BattleStatus().spectator)
        allteams.insert(user.BattleStatus().team);
    }
    std::set<int> teams;
    int t = 0;
    for (size_t i = 0; i < GetNumUsers(); ++i) {
      User& user = GetUser(i);
      if (!user.BattleStatus().spectator) {
        if (teams.count(user.BattleStatus().team)) {
          while (allteams.count(t) || teams.count(t))
            t++;
          ForceTeam(GetUser(i), t);
          teams.insert(t);
        } else {
          teams.insert(user.BattleStatus().team);
        }
      }
    }
    return;
  }
  for (int i = 0; i < numcontrolteams; i++)
    control_teams.push_back(ControlTeam(i));

  wxLogMessage(_T("number of teams: %u"), control_teams.size());

  std::vector<User*> players_sorted;
  players_sorted.reserve(GetNumUsers());

  int player_team_counter = 0;

  for (size_t i = 0; i < GetNumUsers(); ++i) // don't count spectators
  {
    if (!GetUser(i).BattleStatus().spectator) {
      players_sorted.push_back(&GetUser(i));
      // -- server fail? it doesnt work right.
      // ForceTeam(GetUser(i),player_team_counter);
      player_team_counter++;
    }
  }

  shuffle(players_sorted);

  std::map<wxString, ControlTeam> clan_teams;
  if (support_clans) {
    for (size_t i = 0; i < players_sorted.size(); ++i) {
      wxString clan = players_sorted[i]->GetClan();
      if (!clan.empty()) {
        clan_teams[clan].AddPlayer(players_sorted[i]);
      }
    }
  }
  ;

  if (balance_type != balance_random)
    std::sort(players_sorted.begin(), players_sorted.end(), PlayerRankCompareFunction);

  if (support_clans) {
    std::map<wxString, ControlTeam>::iterator clan_it = clan_teams.begin();
    while (clan_it != clan_teams.end()) {
      ControlTeam& clan = (*clan_it).second;
      // if clan is too small (only 1 clan member in battle) or too big, dont count it as clan
      if ((clan.players.size() < 2) ||
          (!strong_clans &&
           (clan.players.size() > ((players_sorted.size() + control_teams.size() - 1) / control_teams.size())))) {
        wxLogMessage(_T("removing clan %s"), (*clan_it).first.c_str());
        std::map<wxString, ControlTeam>::iterator next = clan_it;
        ++next;
        clan_teams.erase(clan_it);
        clan_it = next;
        continue;
      }
      wxLogMessage(_T("Inserting clan %s"), (*clan_it).first.c_str());
      std::sort(control_teams.begin(), control_teams.end());
      float lowestrank = control_teams[0].ranksum;
      int rnd_k = 1; // number of alliances with rank equal to lowestrank
      while (size_t(rnd_k) < control_teams.size()) {
        if (fabs(control_teams[rnd_k].ranksum - lowestrank) > 0.01)
          break;
        rnd_k++;
      }
      wxLogMessage(_T("number of lowestrank teams with same rank=%d"), rnd_k);
      control_teams[my_random(rnd_k)].AddTeam(clan);
      ++clan_it;
    }
  }

  for (size_t i = 0; i < players_sorted.size(); ++i) {
    // skip clanners, those have been added already.
    if (clan_teams.count(players_sorted[i]->GetClan()) > 0) {
      wxLogMessage(_T("clanner already added, nick=%s"), players_sorted[i]->GetNick().c_str());
      continue;
    }

    // find teams with lowest ranksum
    // insert current user into random one out of them
    // since performance doesnt matter here, i simply sort teams,
    // then find how many teams in beginning have lowest ranksum
    // note that balance player ranks range from 1 to 1.1 now
    // i.e. them are quasi equal
    // so we're essentially adding to teams with smallest number of players,
    // the one with smallest ranksum.

    std::sort(control_teams.begin(), control_teams.end());
    float lowestrank = control_teams[0].ranksum;
    int rnd_k = 1; // number of alliances with rank equal to lowestrank
    while (size_t(rnd_k) < control_teams.size()) {
      if (fabs(control_teams[rnd_k].ranksum - lowestrank) > 0.01)
        break;
      rnd_k++;
    }
    wxLogMessage(_T("number of lowestrank teams with same rank=%d"), rnd_k);
    control_teams[my_random(rnd_k)].AddPlayer(players_sorted[i]);
  }

  for (size_t i = 0; i < control_teams.size(); ++i) {
    for (size_t j = 0; j < control_teams[i].players.size(); ++j) {
      ASSERT_LOGIC(control_teams[i].players[j], _T("fail in Autobalance teams, NULL player"));
      wxString msg = wxFormat(_T("setting player %s to team and ally %d")) % control_teams[i].players[j]->GetNick() % i;
      wxLogMessage(_T("%s"), msg.c_str());
      ForceTeam(*control_teams[i].players[j], control_teams[i].teamnum);
      ForceAlly(*control_teams[i].players[j], control_teams[i].teamnum);
    }
  }
}

void Battle::OnUnitsyncReloaded(wxEvent& data) {
  IBattle::OnUnitsyncReloaded(data);
  if (m_is_self_in)
    SendMyBattleStatus();
}

void Battle::ShouldAutoUnspec() {
  if (m_auto_unspec && !IsLocked() && GetMe().BattleStatus().spectator) {
    unsigned int numplayers = 0;
    std::map<int, int> allysizes = GetAllySizes();
    for (std::map<int, int>::const_iterator itor = allysizes.begin(); itor != allysizes.end(); ++itor) {
      numplayers += itor->second;
    }
    if (numplayers < m_auto_unspec_num_players) {
      ForceSpectator(GetMe(), false);
    }
  }
}

void Battle::SetAutoUnspec(bool value) {
  m_auto_unspec = value;
  m_auto_unspec_num_players = GetNumActivePlayers();
  ShouldAutoUnspec();
}

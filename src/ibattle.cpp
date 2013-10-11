/* Copyright (C) 2007 The SpringLobby Team. All rights reserved. */

/**
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

DO NOT CHANGE THIS FILE!

this file is deprecated and will be replaced with

lsl/battle/ibattle.cpp

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**/

#include <wx/tokenzr.h>
#include <wx/image.h>
#include <sstream>
#include <wx/timer.h>

#include "ibattle.h"
#include "utils/debug.h"
#include "utils/conversion.h"
#include <lslutils/misc.h>
#include "uiutils.h"
#include "settings.h"
#include "ui.h" //only required for preset stuff
#include "spring.h"
#include <lslutils/conversion.h>
#include "springlobbyapp.h"

#include <list>
#include <algorithm>
#include <cmath>
#include <set>

const unsigned int TIMER_ID = 102;

IBattle::IBattle()
  : wxEvtHandler()
  , m_map_loaded(false)
  , m_mod_loaded(false)
  , m_map_exists(false)
  , m_mod_exists(false)
  , m_previous_local_mod_name(wxEmptyString)
  , m_ingame(false)
  , m_auto_unspec(false)
  , m_auto_unspec_num_players(0)
  , m_generating_script(false)
  , m_players_ready(0)
  , m_players_sync(0)
  , m_players_ok(0)
  , m_is_self_in(false)
  , m_timer(0)
  , m_start_time(0) {
  ConnectGlobalEvent(this, GlobalEvent::OnUnitsyncReloaded, wxObjectEventFunction(&IBattle::OnUnitsyncReloaded));
}

IBattle::~IBattle() {
  if (m_is_self_in)
    LSL::usync().UnSetCurrentMod();
  if (m_timer)
    m_timer->Stop();
  delete m_timer;
}

bool IBattle::IsSynced() {
  LoadMod();
  LoadMap();
  bool synced = true;
  if (!m_host_map.hash.empty() && m_host_map.hash != "0" && m_host_map.hash != m_local_map.hash)
    synced = false;
  else if (!m_host_map.name.empty() && m_local_map.name != m_host_map.name)
    synced = false;
  else if (!m_host_mod.hash.empty() && m_host_mod.hash != "0" && m_host_mod.hash != m_local_mod.hash)
    synced = false;
  else if (!m_host_mod.name.empty() && m_local_mod.name != m_host_mod.name)
    synced = false;
  return synced;
}

std::vector<wxColour>& IBattle::GetFixColoursPalette(int numteams) const { return GetBigFixColoursPalette(numteams); }

wxColour IBattle::GetFixColour(int i) const {
  int size = m_teams_sizes.size();
  std::vector<wxColour> palette = GetFixColoursPalette(size);
  return palette[i];
}

int IBattle::GetPlayerNum(const User& user) const {
  for (user_map_t::size_type i = 0; i < GetNumUsers(); i++) {
    if (&GetUser(i) == &user)
      return i;
  }
  ASSERT_EXCEPTION(false, _T("The player is not in this game."));
  return -1;
}

#include <algorithm>
class DismissColor {
protected:
  typedef std::vector<wxColour> ColorVec;
  const ColorVec& m_other;

public:
  DismissColor(const ColorVec& other) : m_other(other) {}

  bool operator()(wxColor to_check) { return std::find(m_other.begin(), m_other.end(), to_check) != m_other.end(); }
};

class AreColoursSimilarProxy {
  int m_mindiff;

public:
  AreColoursSimilarProxy(int mindiff) : m_mindiff(mindiff) {}

  bool operator()(wxColor a, wxColor b) { return AreColoursSimilar(a, b, m_mindiff); }
};

wxColour IBattle::GetFreeColour(User*) const {
  typedef std::vector<wxColour> ColorVec;

  ColorVec current_used_colors;
  for (user_map_t::size_type i = 0; i < GetNumUsers(); ++i) {
    UserBattleStatus& bs = GetUser(i).BattleStatus();
    current_used_colors.push_back(bs.colour);
  }

  int inc = 1;
  while (true) {
    ColorVec fixcolourspalette = GetFixColoursPalette(m_teams_sizes.size() + inc++);

    ColorVec::iterator fixcolourspalette_new_end =
        std::unique(fixcolourspalette.begin(), fixcolourspalette.end(), AreColoursSimilarProxy(20));

    fixcolourspalette_new_end =
        std::remove_if(fixcolourspalette.begin(), fixcolourspalette.end(), DismissColor(current_used_colors));

    if (fixcolourspalette_new_end != fixcolourspalette.begin())
      return (*fixcolourspalette.begin());
  }
}

wxColour IBattle::GetFreeColour(User& for_whom) const { return GetFreeColour(&for_whom); }

wxColour IBattle::GetNewColour() const { return GetFreeColour(); }

int IBattle::ColourDifference(const wxColour& a, const wxColour& b) const // returns max difference of r,g,b.
{
  return std::max(abs(a.Red() - b.Red()), std::max(abs(a.Green() - b.Green()), abs(a.Blue() - b.Blue())));
}

int IBattle::GetFreeTeam(bool excludeme) const {
  int lowest = 0;
  bool changed = true;
  while (changed) {
    changed = false;
    for (user_map_t::size_type i = 0; i < GetNumUsers(); i++) {
      User& user = GetUser(i);
      if ((&user == &GetMe()) && excludeme)
        continue;
      if (user.BattleStatus().spectator)
        continue;
      if (user.BattleStatus().team == lowest) {
        lowest++;
        changed = true;
      }
    }
  }
  return lowest;
}

int IBattle::GetClosestFixColour(const wxColour& col, const std::vector<int>& excludes, int difference) const {
  std::vector<wxColour> palette = GetFixColoursPalette(m_teams_sizes.size() + 1);
  int result = 0;
  int t1 = palette.size();
  int t2 = excludes.size();
  wxLogMessage(_T("GetClosestFixColour %d %d"), t1, t2);
  for (size_t i = 0; i < palette.size(); ++i) {
    if ((i >= excludes.size()) || (!excludes[i])) {
      if (AreColoursSimilar(palette[i], col, difference)) {
        return i;
      }
    }
  }
  return result;
}

void IBattle::SendHostInfo(HostInfo /*unused*/) {}

void IBattle::SendHostInfo(const wxString& /*unused*/) {}

void IBattle::Update(const wxString& /*unused*/) {}

User& IBattle::OnUserAdded(User& user) {
  UserList::AddUser(user);
  UserBattleStatus& bs = user.BattleStatus();
  bs.spectator = false;
  bs.ready = false;
  bs.sync = SYNC_UNKNOWN;
  if (!bs.IsBot() && IsFounderMe() && GetBattleType() == BT_Played) {
    bs.team = GetFreeTeam(&user == &GetMe());
    bs.ally = GetFreeAlly(&user == &GetMe());
    bs.colour = GetFreeColour(user);
  }
  if (IsFounderMe() && ((bs.pos.x < 0) || (bs.pos.y < 0))) {
    UserPosition& pos = bs.pos;
    pos = GetFreePosition();
    UserPositionChanged(user);
  }
  if (!bs.spectator) {
    PlayerJoinedAlly(bs.ally);
    PlayerJoinedTeam(bs.team);
  }
  if (bs.spectator && IsFounderMe())
    m_opts.spectators++;
  if (!bs.spectator && !bs.IsBot()) {
    if (bs.ready)
      m_players_ready++;
    if (bs.sync)
      m_players_sync++;
    if (!bs.ready || !bs.sync)
      m_ready_up_map[user.GetNick()] = time(0);
    if (bs.ready && bs.sync)
      m_players_ok++;
  }
  return user;
}

User& IBattle::OnBotAdded(const wxString& nick, const UserBattleStatus& bs) {
  m_internal_bot_list[nick] = User(nick);
  User& user = m_internal_bot_list[nick];
  user.UpdateBattleStatus(bs);
  User& usr = OnUserAdded(user);
  return usr;
}

unsigned int IBattle::GetNumBots() const { return m_internal_bot_list.size(); }

unsigned int IBattle::GetNumPlayers() const { return GetNumUsers() - GetNumBots(); }

unsigned int IBattle::GetNumActivePlayers() const { return GetNumPlayers() - m_opts.spectators; }

void IBattle::OnUserBattleStatusUpdated(User& user, UserBattleStatus status) {

  UserBattleStatus previousstatus = user.BattleStatus();

  user.UpdateBattleStatus(status);
  unsigned int oldspeccount = m_opts.spectators;
  m_opts.spectators = 0;
  m_players_sync = 0;
  m_players_ready = 0;
  m_players_ok = 0;
  m_teams_sizes.clear();
  m_ally_sizes.clear();
  for (unsigned int i = 0; i < GetNumUsers(); i++) {
    User& loopuser = GetUser(i);
    UserBattleStatus& loopstatus = loopuser.BattleStatus();
    if (loopstatus.spectator)
      m_opts.spectators++;
    if (!loopstatus.IsBot()) {
      if (!loopstatus.spectator) {
        if (loopstatus.ready && loopstatus.spectator)
          m_players_ready++;
        if (loopstatus.sync)
          m_players_sync++;
        if (loopstatus.ready && loopstatus.sync)
          m_players_ok++;
        PlayerJoinedTeam(loopstatus.team);
        PlayerJoinedAlly(loopstatus.ally);
      }
    }
  }
  if (oldspeccount != m_opts.spectators) {
    if (IsFounderMe())
      SendHostInfo(HI_Spectators);
  }
  if (!status.IsBot()) {
    if ((status.ready && status.sync) || status.spectator) {
      std::map<wxString, time_t>::iterator itor = m_ready_up_map.find(user.GetNick());
      if (itor != m_ready_up_map.end()) {
        m_ready_up_map.erase(itor);
      }
    }
    if ((!status.ready || !status.sync) && !status.spectator) {
      std::map<wxString, time_t>::iterator itor = m_ready_up_map.find(user.GetNick());
      if (itor == m_ready_up_map.end()) {
        m_ready_up_map[user.GetNick()] = time(0);
      }
    }
  }
}

bool IBattle::ShouldAutoStart() const {
  if (GetInGame())
    return false;
  if (!IsLocked() && (GetNumActivePlayers() < m_opts.maxplayers))
    return false; // proceed checking for ready & symc players only if the battle is full or locked
  if (!IsEveryoneReady())
    return false;
  return true;
}

void IBattle::OnUserRemoved(User& user) {
  UserBattleStatus& bs = user.BattleStatus();
  if (!bs.spectator) {
    PlayerLeftTeam(bs.team);
    PlayerLeftAlly(bs.ally);
  }
  if (!bs.spectator && !bs.IsBot()) {
    if (bs.ready)
      m_players_ready--;
    if (bs.sync)
      m_players_sync--;
    if (bs.ready && bs.sync)
      m_players_ok--;
  }
  if (IsFounderMe() && bs.spectator) {
    m_opts.spectators--;
    SendHostInfo(HI_Spectators);
  }
  if (&user == &GetMe()) {
    OnSelfLeftBattle();
  }
  UserList::RemoveUser(user.GetNick());
  if (!bs.IsBot())
    user.SetBattle(0);
  else {
    UserVecIter itor = m_internal_bot_list.find(user.GetNick());
    if (itor != m_internal_bot_list.end()) {
      m_internal_bot_list.erase(itor);
    }
  }
}

bool IBattle::IsEveryoneReady() const {
  for (unsigned int i = 0; i < GetNumPlayers(); i++) {
    User& usr = GetUser(i);
    UserBattleStatus& status = usr.BattleStatus();
    if (status.IsBot())
      continue;
    if (status.spectator)
      continue;
    if (&usr == &GetMe())
      continue;
    if (!status.ready)
      return false;
    if (!status.sync)
      return false;
  }
  return true;
}

void IBattle::AddStartRect(unsigned int allyno, unsigned int left, unsigned int top, unsigned int right,
                           unsigned int bottom) {
  BattleStartRect sr;

  sr.ally = allyno;
  sr.left = left;
  sr.top = top;
  sr.right = right;
  sr.bottom = bottom;
  sr.toadd = true;
  sr.todelete = false;
  sr.toresize = false;
  sr.exist = true;

  m_rects[allyno] = sr;
}

void IBattle::RemoveStartRect(unsigned int allyno) {
  std::map<unsigned int, BattleStartRect>::iterator rect_it = m_rects.find(allyno);
  if (rect_it == m_rects.end())
    return;

  rect_it->second.todelete = true;
  // BattleStartRect sr = m_rects[allyno];
  // sr.todelete = true;
  // m_rects[allyno] = sr;
}

void IBattle::ResizeStartRect(unsigned int allyno) {
  std::map<unsigned int, BattleStartRect>::iterator rect_it = m_rects.find(allyno);
  if (rect_it == m_rects.end())
    return;

  rect_it->second.toresize = true;
  // BattleStartRect sr = m_rects[allyno];
  //&&sr.toresize = true;
  // m_rects[allyno] = sr;
}

void IBattle::StartRectRemoved(unsigned int allyno) {
  std::map<unsigned int, BattleStartRect>::const_iterator rect_it = m_rects.find(allyno);
  if (rect_it == m_rects.end())
    return;

  if (rect_it->second.todelete)
    m_rects.erase(allyno);
}

void IBattle::StartRectResized(unsigned int allyno) {
  std::map<unsigned int, BattleStartRect>::iterator rect_it = m_rects.find(allyno);
  if (rect_it == m_rects.end())
    return;

  rect_it->second.toresize = false;
  // BattleStartRect sr = m_rects[allyno];
  // sr.toresize = false;
  // m_rects[allyno] = sr;
}

void IBattle::StartRectAdded(unsigned int allyno) {
  std::map<unsigned int, BattleStartRect>::iterator rect_it = m_rects.find(allyno);
  if (rect_it == m_rects.end())
    return;

  rect_it->second.toadd = false;
  // BattleStartRect sr = m_rects[allyno];
  // sr.toadd = false;
  // m_rects[allyno] = sr;
}

BattleStartRect IBattle::GetStartRect(unsigned int allyno) const {
  std::map<unsigned int, BattleStartRect>::const_iterator rect_it = m_rects.find(allyno);
  if (rect_it != m_rects.end())
    return (*rect_it).second;
  return BattleStartRect();
}

// total number of start rects
unsigned int IBattle::GetNumRects() const { return m_rects.size(); }

// key of last start rect in the map
unsigned int IBattle::GetLastRectIdx() const {
  if (GetNumRects() > 0)
    return m_rects.rbegin()->first;

  return 0;
}

// return  the lowest currently unused key in the map of rects.
unsigned int IBattle::GetNextFreeRectIdx() const {
  // check for unused allyno keys
  for (unsigned int i = 0; i <= GetLastRectIdx(); i++) {
    if (!GetStartRect(i).IsOk())
      return i;
  }
  return GetNumRects(); // if all rects are in use, or no elements exist, return first possible available allyno.
}

void IBattle::ClearStartRects() { m_rects.clear(); }

void IBattle::ForceSide(User& user, int side) {
  if (IsFounderMe() || user.BattleStatus().IsBot()) {
    user.BattleStatus().side = side;
  }
}

void IBattle::ForceTeam(User& user, int team) {
  if (IsFounderMe() || user.BattleStatus().IsBot()) {
    if (!user.BattleStatus().spectator) {
      PlayerLeftTeam(user.BattleStatus().team);
      PlayerJoinedTeam(team);
    }
    user.BattleStatus().team = team;
  }
}

void IBattle::ForceAlly(User& user, int ally) {

  if (IsFounderMe() || user.BattleStatus().IsBot()) {
    if (!user.BattleStatus().spectator) {
      PlayerLeftAlly(user.BattleStatus().ally);
      PlayerJoinedAlly(ally);
    }
    user.BattleStatus().ally = ally;
  }
}

void IBattle::ForceColour(User& user, const wxColour& col) {
  if (IsFounderMe() || user.BattleStatus().IsBot()) {
    user.BattleStatus().colour = col;
  }
}

void IBattle::PlayerJoinedTeam(int team) {
  std::map<int, int>::const_iterator itor = m_teams_sizes.find(team);
  if (itor == m_teams_sizes.end())
    m_teams_sizes[team] = 1;
  else
    m_teams_sizes[team] = m_teams_sizes[team] + 1;
}

void IBattle::PlayerJoinedAlly(int ally) {
  std::map<int, int>::const_iterator iter = m_ally_sizes.find(ally);
  if (iter == m_ally_sizes.end())
    m_ally_sizes[ally] = 1;
  else
    m_ally_sizes[ally] = m_ally_sizes[ally] + 1;
}

void IBattle::PlayerLeftTeam(int team) {
  std::map<int, int>::iterator itor = m_teams_sizes.find(team);
  if (itor != m_teams_sizes.end()) {
    itor->second = itor->second - 1;
    if (itor->second == 0) {
      m_teams_sizes.erase(itor);
    }
  }
}

void IBattle::PlayerLeftAlly(int ally) {
  std::map<int, int>::iterator iter = m_ally_sizes.find(ally);
  if (iter != m_ally_sizes.end()) {
    iter->second = iter->second - 1;
    if (iter->second == 0) {
      m_ally_sizes.erase(iter);
    }
  }
}

void IBattle::ForceSpectator(User& user, bool spectator) {
  if (IsFounderMe() || user.BattleStatus().IsBot()) {
    UserBattleStatus& status = user.BattleStatus();

    if (!status.spectator) // leaving spectator status
    {
      PlayerJoinedTeam(status.team);
      PlayerJoinedAlly(status.ally);
      if (status.ready && !status.IsBot())
        m_players_ready++;
    }

    if (spectator) // entering spectator status
    {
      PlayerLeftTeam(status.team);
      PlayerLeftAlly(status.ally);
      if (status.ready && !status.IsBot())
        m_players_ready--;
    }

    if (IsFounderMe()) {
      if (status.spectator != spectator) {
        if (spectator) {
          m_opts.spectators++;
        } else {
          m_opts.spectators--;
        }
        SendHostInfo(HI_Spectators);
      }
    }
    user.BattleStatus().spectator = spectator;
  }
}

void IBattle::SetHandicap(User& user, int handicap) {
  if (IsFounderMe() || user.BattleStatus().IsBot()) {
    user.BattleStatus().handicap = handicap;
  }
}

void IBattle::KickPlayer(User& user) {
  if (IsFounderMe() || user.BattleStatus().IsBot()) {
    OnUserRemoved(user);
  }
}

int IBattle::GetFreeAlly(bool excludeme) const {
  int lowest = 0;
  bool changed = true;
  while (changed) {
    changed = false;
    for (unsigned int i = 0; i < GetNumUsers(); i++) {
      User& user = GetUser(i);
      if ((&user == &GetMe()) && excludeme)
        continue;
      if (user.BattleStatus().spectator)
        continue;
      if (user.BattleStatus().ally == lowest) {
        lowest++;
        changed = true;
      }
    }
  }
  return lowest;
}

UserPosition IBattle::GetFreePosition() {
  UserPosition ret;
  LSL::UnitsyncMap map = LoadMap();
  for (int i = 0; i < int(map.info.positions.size()); i++) {
    bool taken = false;
    for (unsigned int bi = 0; bi < GetNumUsers(); bi++) {
      User& user = GetUser(bi);
      UserBattleStatus& status = user.BattleStatus();
      if (status.spectator)
        continue;
      if ((map.info.positions[i].x == status.pos.x) && (map.info.positions[i].y == status.pos.y)) {
        taken = true;
        break;
      }
    }
    if (!taken) {
      ret.x = LSL::Util::Clamp(map.info.positions[i].x, 0, map.info.width);
      ret.y = LSL::Util::Clamp(map.info.positions[i].y, 0, map.info.height);
      return ret;
    }
  }
  ret.x = map.info.width / 2;
  ret.y = map.info.height / 2;
  return ret;
}

void IBattle::SetHostMap(const wxString& _mapname, const wxString& _hash) {
  assert(!_mapname.empty());
  const std::string mapname(STD_STRING(_mapname));
  const std::string hash(STD_STRING(_hash));
  if (mapname != m_host_map.name || hash != m_host_map.hash) {
    m_map_loaded = false;
    m_host_map.name = mapname;
    m_host_map.hash = hash;
    if (!m_host_map.hash.empty() && m_host_map.hash != "0")
      m_map_exists = LSL::usync().MapExists(m_host_map.name, m_host_map.hash);
    else
      m_map_exists = LSL::usync().MapExists(m_host_map.name);
#ifndef __WXMSW__ //!TODO why not on win?
    if (m_map_exists && !spring().IsRunning())
      LSL::usync().PrefetchMap(m_host_map.name);
#endif
  }
}

void IBattle::SetLocalMap(const LSL::UnitsyncMap& map) {
  assert(!map.name.empty());
  if (map.name != m_local_map.name || map.hash != m_local_map.hash) {
    m_local_map = map;
    m_map_loaded = true;
    if (!m_host_map.hash.empty() && m_host_map.hash != "0")
      m_map_exists = LSL::usync().MapExists(m_host_map.name, m_host_map.hash);
    else
      m_map_exists = LSL::usync().MapExists(m_host_map.name);
#ifndef __WXMSW__
    if (m_map_exists && !spring().IsRunning())
      LSL::usync().PrefetchMap(m_host_map.name);
#endif
    if (IsFounderMe()) // save all rects infos
    {
    }
  }
}

const LSL::UnitsyncMap& IBattle::LoadMap() {
  if ((!m_map_loaded) && (!m_host_map.name.empty())) {
    try {
      ASSERT_EXCEPTION(m_map_exists, _T("Map does not exist: ") + TowxString(m_host_map.name));
      m_local_map = LSL::usync().GetMapEx(m_host_map.name);
      bool options_loaded = CustomBattleOptions().loadOptions(LSL::OptionsWrapper::MapOption, m_host_map.name);
      ASSERT_EXCEPTION(options_loaded, _T("couldn't load the map options"));
      m_map_loaded = true;
    }
    catch (...) {
    }
  }
  return m_local_map;
}

wxString IBattle::GetHostMapName() const { return TowxString(m_host_map.name); }

wxString IBattle::GetHostMapHash() const { return TowxString(m_host_map.hash); }

void IBattle::SetHostMod(const wxString& _modname, const wxString& _hash) {
  const std::string modname(STD_STRING(_modname));
  const std::string hash(STD_STRING(_hash));
  if (m_host_mod.name != modname || m_host_mod.hash != hash) {
    m_mod_loaded = false;
    m_host_mod.name = modname;
    m_host_mod.hash = hash;
    if (!m_host_mod.hash.empty() && m_host_mod.hash != "0")
      m_mod_exists = LSL::usync().ModExists(m_host_mod.name, m_host_mod.hash);
    else
      m_mod_exists = LSL::usync().ModExists(m_host_mod.name);
  }
}

void IBattle::SetLocalMod(const LSL::UnitsyncMod& mod) {
  if (mod.name != m_local_mod.name || mod.hash != m_local_mod.hash) {
    m_previous_local_mod_name = TowxString(m_local_mod.name);
    m_local_mod = mod;
    m_mod_loaded = true;
    if (!m_host_mod.hash.empty() && m_host_mod.hash != "0")
      m_mod_exists = LSL::usync().ModExists(m_host_mod.name, m_host_mod.hash);
    else
      m_mod_exists = LSL::usync().ModExists(m_host_mod.name);
  }
}

const LSL::UnitsyncMod& IBattle::LoadMod() {
  assert(!m_host_mod.name.empty());
  if (!m_mod_loaded) {
    try {
      ASSERT_EXCEPTION(m_mod_exists, _T("Mod does not exist."));
      m_local_mod = LSL::usync().GetMod(m_host_mod.name);
      bool options_loaded = CustomBattleOptions().loadOptions(LSL::OptionsWrapper::ModOption, m_host_mod.name);
      ASSERT_EXCEPTION(options_loaded, _T("couldn't load the mod options"));
      m_mod_loaded = true;
    }
    catch (...) {
    }
  }
  return m_local_mod;
}

wxString IBattle::GetHostModName() const { return TowxString(m_host_mod.name); }

wxString IBattle::GetHostModHash() const { return TowxString(m_host_mod.hash); }

bool IBattle::MapExists() const {
  return m_map_exists;
  //  return LSL::usync().MapExists( m_map.name, m_map.hash );
}

bool IBattle::ModExists() const {
  return m_mod_exists;
  //  return LSL::usync().ModExists( m_host_mod.name, m_host_mod.hash );
}

void IBattle::RestrictUnit(const wxString& unitname, int count) { m_restricted_units[unitname] = count; }

void IBattle::UnrestrictUnit(const wxString& unitname) {
  std::map<wxString, int>::iterator pos = m_restricted_units.find(unitname);
  if (pos == m_restricted_units.end())
    return;
  m_restricted_units.erase(pos);
}

void IBattle::UnrestrictAllUnits() { m_restricted_units.clear(); }

std::map<wxString, int> IBattle::RestrictedUnits() const { return m_restricted_units; }

void IBattle::OnSelfLeftBattle() {
  GetMe().BattleStatus().spectator = false; // always reset back yourself to player when rejoining
  if (m_timer)
    m_timer->Stop();
  delete m_timer;
  m_timer = 0;
  m_is_self_in = false;
  for (size_t j = 0; j < GetNumUsers(); ++j) {
    User& u = GetUser(j);
    if (u.GetBattleStatus().IsBot()) {
      OnUserRemoved(u);
      ui().OnUserLeftBattle(*this, u, true);
      j--;
    }
  }
  ClearStartRects();
  m_teams_sizes.clear();
  m_ally_sizes.clear();
  m_players_ready = 0;
  m_players_sync = 0;
  m_players_ok = 0;
  LSL::usync().UnSetCurrentMod(); // left battle
}

void IBattle::OnUnitsyncReloaded(wxEvent& /*data*/) {
  if (!m_host_mod.hash.empty() && m_host_mod.hash != "0")
    m_mod_exists = LSL::usync().ModExists(m_host_mod.name, m_host_mod.hash);
  else
    m_mod_exists = LSL::usync().ModExists(m_host_mod.name);
  if (!m_host_map.hash.empty() && m_host_map.hash != "0")
    m_map_exists = LSL::usync().MapExists(m_host_map.name, m_host_map.hash);
  else
    m_map_exists = LSL::usync().MapExists(m_host_map.name);
}

static wxString FixPresetName(const wxString& name) {
  // look name up case-insensitively
  const wxArrayString& presetList = sett().GetPresetList();
  int index = presetList.Index(name, false /*case insensitive*/);
  if (index == -1)
    return _T("");

  // set preset to the actual name, with correct case
  return presetList[index];
}

bool IBattle::LoadOptionsPreset(const wxString& name) {
  wxString preset = FixPresetName(name);
  if (preset == _T(""))
    return false; // preset not found
  m_preset = preset;

  for (unsigned int i = 0; i < LSL::OptionsWrapper::LastOption; i++) {
    std::map<wxString, wxString> options = sett().GetHostingPreset(m_preset, i);
    if ((LSL::OptionsWrapper::GameOption)i != LSL::OptionsWrapper::PrivateOptions) {
      for (std::map<wxString, wxString>::const_iterator itor = options.begin(); itor != options.end(); ++itor) {
        wxLogWarning(itor->first + _T(" ::: ") + itor->second);
        CustomBattleOptions().setSingleOption(STD_STRING(itor->first), STD_STRING(itor->second),
                                              (LSL::OptionsWrapper::GameOption)i);
      }
    } else {
      if (!options[_T("mapname")].IsEmpty()) {
        if (LSL::usync().MapExists(STD_STRING(options[_T("mapname")]))) {
          LSL::UnitsyncMap map = LSL::usync().GetMapEx(STD_STRING(options[_T("mapname")]));
          SetLocalMap(map);
          SendHostInfo(HI_Map);
        } else if (!ui().OnPresetRequiringMap(options[_T("mapname")])) {
          // user didn't want to download the missing map, so set to empty to not have it tried to be loaded again
          options[_T("mapname")] = _T("");
          sett().SetHostingPreset(m_preset, i, options);
        }
      }

      for (unsigned int j = 0; j <= GetLastRectIdx(); ++j) {
        if (GetStartRect(j).IsOk())
          RemoveStartRect(j); // remove all rects that might come from map presets
      }
      SendHostInfo(IBattle::HI_StartRects);

      unsigned int rectcount = s2l(options[_T("numrects")]);
      for (unsigned int loadrect = 0; loadrect < rectcount; loadrect++) {
        int ally = s2l(options[_T("rect_") + TowxString(loadrect) + _T("_ally")]);
        if (ally == 0)
          continue;
        AddStartRect(ally - 1, s2l(options[_T("rect_") + TowxString(loadrect) + _T("_left")]),
                     s2l(options[_T("rect_") + TowxString(loadrect) + _T("_top")]),
                     s2l(options[_T("rect_") + TowxString(loadrect) + _T("_right")]),
                     s2l(options[_T("rect_") + TowxString(loadrect) + _T("_bottom")]));
      }
      SendHostInfo(HI_StartRects);

      wxStringTokenizer tkr(options[_T("restrictions")], _T('\t'));
      m_restricted_units.clear();
      while (tkr.HasMoreTokens()) {
        wxString unitinfo = tkr.GetNextToken();
        RestrictUnit(unitinfo.BeforeLast(_T('=')), s2l(unitinfo.AfterLast(_T('='))));
      }
      SendHostInfo(HI_Restrictions);
      Update(wxFormat(_T("%d_restrictions")) % LSL::OptionsWrapper::PrivateOptions);
    }
  }
  SendHostInfo(HI_Send_All_opts);
  ui().ReloadPresetList();
  return true;
}

void IBattle::SaveOptionsPreset(const wxString& name) {
  m_preset = FixPresetName(name);
  if (m_preset == _T(""))
    m_preset = name; // new preset

  for (int i = 0; i < (int)LSL::OptionsWrapper::LastOption; i++) {
    if ((LSL::OptionsWrapper::GameOption)i != LSL::OptionsWrapper::PrivateOptions) {
      const auto opts = CustomBattleOptions().getOptionsMap((LSL::OptionsWrapper::GameOption)i);
      std::map<wxString, wxString> wopts;
      for (const auto pair : opts)
        wopts.insert(std::make_pair(TowxString(pair.first), TowxString(pair.second)));
      sett().SetHostingPreset(m_preset, (LSL::OptionsWrapper::GameOption)i, wopts);
    } else {
      std::map<wxString, wxString> opts;
      opts[_T("mapname")] = GetHostMapName();
      unsigned int validrectcount = 0;
      if (LSL::Util::FromString<long>(
              CustomBattleOptions().getSingleValue("startpostype", LSL::OptionsWrapper::EngineOption)) == ST_Choose) {
        unsigned int boxcount = GetLastRectIdx();
        for (unsigned int boxnum = 0; boxnum <= boxcount; boxnum++) {
          BattleStartRect rect = GetStartRect(boxnum);
          if (rect.IsOk()) {
            opts[_T("rect_") + TowxString(validrectcount) + _T("_ally")] = TowxString(rect.ally + 1);
            opts[_T("rect_") + TowxString(validrectcount) + _T("_left")] = TowxString(rect.left);
            opts[_T("rect_") + TowxString(validrectcount) + _T("_top")] = TowxString(rect.top);
            opts[_T("rect_") + TowxString(validrectcount) + _T("_bottom")] = TowxString(rect.bottom);
            opts[_T("rect_") + TowxString(validrectcount) + _T("_right")] = TowxString(rect.right);
            validrectcount++;
          }
        }
      }
      opts[_T("numrects")] = TowxString(validrectcount);

      wxString restrictionsstring;
      for (std::map<wxString, int>::const_iterator itor = m_restricted_units.begin(); itor != m_restricted_units.end();
           ++itor) {
        restrictionsstring << itor->first << _T('=') << TowxString(itor->second) << _T('\t');
      }
      opts[_T("restrictions")] = restrictionsstring;

      sett().SetHostingPreset(m_preset, (LSL::OptionsWrapper::GameOption)i, opts);
    }
  }
  sett().SaveSettings();
  ui().ReloadPresetList();
}

wxString IBattle::GetCurrentPreset() { return m_preset; }

void IBattle::DeletePreset(const wxString& name) {
  wxString preset = FixPresetName(name);
  if (m_preset == preset)
    m_preset = _T("");
  sett().DeletePreset(preset);
  ui().ReloadPresetList();
}

wxArrayString IBattle::GetPresetList() { return sett().GetPresetList(); }

void IBattle::UserPositionChanged(const User& /*unused*/) {}

void IBattle::AddUserFromDemo(User& user) {
  user.BattleStatus().isfromdemo = true;
  m_internal_user_list[user.GetNick()] = user;
  UserList::AddUser(m_internal_user_list[user.GetNick()]);
}

void IBattle::SetProxy(const wxString& value) { m_opts.proxyhost = value; }

bool IBattle::IsProxy() const { return !m_opts.proxyhost.IsEmpty(); }

wxString IBattle::GetProxy() const { return m_opts.proxyhost; }

bool IBattle::IsFounderMe() const {
  return ((m_opts.founder == GetMe().GetNick()) || (IsProxy() && !m_generating_script));
}

bool IBattle::IsFounder(const User& user) const {
  if (UserExists(m_opts.founder)) {
    try {
      return &GetFounder() == &user;
    }
    catch (...) {
      return false;
    }
  } else
    return false;
}

int IBattle::GetMyPlayerNum() const { return GetPlayerNum(GetMe()); }

void IBattle::LoadScriptMMOpts(const wxString& sectionname, const SL::PDataList& node) {
  if (!node.ok())
    return;
  SL::PDataList section(node->Find(sectionname));
  if (!section.ok())
    return;
  LSL::OptionsWrapper& opts = CustomBattleOptions();
  for (SL::PNode n = section->First(); n != section->Last(); n = section->Next(n)) {
    if (!n.ok())
      continue;
    opts.setSingleOption(STD_STRING(n->Name()), STD_STRING(section->GetString(n->Name())));
  }
}

void IBattle::LoadScriptMMOpts(const SL::PDataList& node) {
  if (!node.ok())
    return;
  LSL::OptionsWrapper& opts = CustomBattleOptions();
  auto options = opts.getOptionsMap(LSL::OptionsWrapper::EngineOption);
  for (const auto i : options) {
    opts.setSingleOption(i.first, STD_STRING(node->GetString(TowxString(i.first), TowxString(i.second))));
  }
}

//! (koshi) don't delete commented things please, they might be need in the future and i'm lazy
void IBattle::GetBattleFromScript(bool loadmapmod) {

  BattleOptions opts;
  std::stringstream ss((const char*)GetScript().mb_str(
      wxConvUTF8)); // no need to convert wxstring-->std::string-->std::stringstream, convert directly.
  SL::PDataList script(ParseTDF(ss));

  SL::PDataList replayNode(script->Find(_T("GAME")));
  if (replayNode.ok()) {

    wxString modname = replayNode->GetString(_T("GameType"));
    wxString modhash = replayNode->GetString(_T("ModHash"));
    if (!modhash.IsEmpty())
      modhash = MakeHashUnsigned(modhash);
    SetHostMod(modname, modhash);

    // don't have the maphash, what to do?
    // ui download function works with mapname if hash is empty, so works for now
    wxString mapname = replayNode->GetString(_T("MapName"));
    wxString maphash = replayNode->GetString(_T("MapHash"));
    if (!maphash.IsEmpty())
      maphash = MakeHashUnsigned(maphash);
    SetHostMap(mapname, maphash);

    //        opts.ip         = replayNode->GetString( _T("HostIP") );
    //        opts.port       = replayNode->GetInt  ( _T("HostPort"), DEFAULT_EXTERNAL_UDP_SOURCE_PORT );
    opts.spectators = 0;

    int playernum = replayNode->GetInt(_T("NumPlayers"), 0);
    int usersnum = replayNode->GetInt(_T("NumUsers"), 0);
    if (usersnum > 0)
      playernum = usersnum;
    //        int allynum = replayNode->GetInt  ( _T("NumAllyTeams"), 1);
    //        int teamnum = replayNode->GetInt  ( _T("NumTeams"), 1);

    LSL::StringVector sides;
    if (loadmapmod) {
      sides = LSL::usync().GetSides(STD_STRING(modname));
    }

    IBattle::TeamVec parsed_teams = GetParsedTeamsVec();
    IBattle::AllyVec parsed_allies = GetParsedAlliesVec();

    //[PLAYERX] sections
    for (int i = 0; i < playernum; ++i) {
      SL::PDataList player(replayNode->Find(_T("PLAYER") + TowxString(i)));
      SL::PDataList bot(replayNode->Find(_T("AI") + TowxString(i)));
      if (player.ok() || bot.ok()) {
        if (bot.ok())
          player = bot;
        User user(player->GetString(_T("Name")), (player->GetString(_T("CountryCode")).Upper()), 0);
        UserBattleStatus& status = user.BattleStatus();
        status.isfromdemo = true;
        status.spectator = player->GetInt(_T("Spectator"), 0);
        opts.spectators += user.BattleStatus().spectator;
        status.team = player->GetInt(_T("Team"));
        if (!status.spectator) {
          PlayerJoinedTeam(status.team);
        }
        status.sync = true;
        status.ready = true;
        if (status.spectator)
          m_opts.spectators++;
        else {
          if (!bot.ok()) {
            if (status.ready)
              m_players_ready++;
            if (status.sync)
              m_players_sync++;
            if (status.sync && status.ready)
              m_players_ok++;
          }
        }

        //! (koshi) changed this from ServerRankContainer to RankContainer
        user.Status().rank = (UserStatus::RankContainer)player->GetInt(_T("Rank"), -1);

        if (bot.ok()) {
          status.aishortname = bot->GetString(_T("ShortName" ));
          status.aiversion = bot->GetString(_T("Version" ));
          int ownerindex = bot->GetInt(_T("Host" ));
          SL::PDataList aiowner(replayNode->Find(_T("PLAYER") + TowxString(ownerindex)));
          if (aiowner.ok()) {
            status.owner = aiowner->GetString(_T("Name"));
          }
        }

        IBattle::TeamInfoContainer teaminfos = parsed_teams[user.BattleStatus().team];
        if (!teaminfos.exist) {
          SL::PDataList team(replayNode->Find(_T("TEAM") + TowxString(user.BattleStatus().team)));
          if (team.ok()) {
            teaminfos.exist = true;
            teaminfos.TeamLeader = team->GetInt(_T("TeamLeader"), 0);
            teaminfos.StartPosX = team->GetInt(_T("StartPosX"), -1);
            teaminfos.StartPosY = team->GetInt(_T("StartPosY"), -1);
            teaminfos.AllyTeam = team->GetInt(_T("AllyTeam"), 0);
            teaminfos.RGBColor = GetColorFromFloatStrng(team->GetString(_T("RGBColor")));
            teaminfos.SideName = team->GetString(_T("Side"), _T(""));
            teaminfos.Handicap = team->GetInt(_T("Handicap"), 0);
            const int sidepos = LSL::Util::IndexInSequence(sides, STD_STRING(teaminfos.SideName));
            teaminfos.SideNum = sidepos;
            parsed_teams[user.BattleStatus().team] = teaminfos;
          }
        }
        if (teaminfos.exist) {
          status.ally = teaminfos.AllyTeam;
          status.pos.x = teaminfos.StartPosX;
          status.pos.y = teaminfos.StartPosY;
          status.colour = teaminfos.RGBColor;
          status.handicap = teaminfos.Handicap;
          if (!status.spectator) {
            PlayerJoinedAlly(status.ally);
          }
          if (teaminfos.SideNum >= 0)
            status.side = teaminfos.SideNum;
          IBattle::AllyInfoContainer allyinfos = parsed_allies[user.BattleStatus().ally];
          if (!allyinfos.exist) {
            SL::PDataList ally(replayNode->Find(_T("ALLYTEAM") + TowxString(user.BattleStatus().ally)));
            if (ally.ok()) {
              allyinfos.exist = true;
              allyinfos.NumAllies = ally->GetInt(_T("NumAllies"), 0);
              allyinfos.StartRectLeft = ally->GetInt(_T("StartRectLeft"), 0);
              allyinfos.StartRectTop = ally->GetInt(_T("StartRectTop"), 0);
              allyinfos.StartRectRight = ally->GetInt(_T("StartRectRight"), 0);
              allyinfos.StartRectBottom = ally->GetInt(_T("StartRectBottom"), 0);
              parsed_allies[user.BattleStatus().ally] = allyinfos;
              AddStartRect(user.BattleStatus().ally, allyinfos.StartRectTop, allyinfos.StartRectTop,
                           allyinfos.StartRectRight, allyinfos.StartRectBottom);
            }
          }
        }

        AddUserFromDemo(user);
      }
    }
    SetParsedTeamsVec(parsed_teams);
    SetParsedAlliesVec(parsed_allies);

    // MMoptions, this'll fail unless loading map/mod into wrapper first
    if (loadmapmod) {
      LoadScriptMMOpts(_T("mapoptions"), replayNode);
      LoadScriptMMOpts(_T("modoptions"), replayNode);
    }

    opts.maxplayers = playernum;
  }
  SetBattleOptions(opts);
}

void IBattle::SetInGame(bool ingame) {
  m_ingame = ingame;
  if (m_ingame)
    m_start_time = wxGetUTCTime();
  else
    m_start_time = 0;
}

long IBattle::GetBattleRunningTime() const {
  if (!GetInGame())
    return 0;
  if (m_start_time == 0)
    return 0;
  return wxGetUTCTime() - m_start_time;
}

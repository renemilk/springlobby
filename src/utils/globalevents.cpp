#include "globalevents.h"
#include <wx/app.h>
#include <list>
#include <map>

const wxEventType GlobalEvent::OnDownloadComplete = wxNewEventType();
const wxEventType GlobalEvent::OnUnitsyncFirstTimeLoad = wxNewEventType();
const wxEventType GlobalEvent::OnUnitsyncReloaded = wxNewEventType();
const wxEventType GlobalEvent::OnSpringTerminated = wxNewEventType();
const wxEventType GlobalEvent::OnSpringStarted = wxNewEventType();
const wxEventType GlobalEvent::UpdateFinished = wxNewEventType();
const wxEventType GlobalEvent::OnQuit = wxNewEventType();
const wxEventType GlobalEvent::OnLogin = wxNewEventType();
const wxEventType GlobalEvent::PlasmaResourceListParsed = wxNewEventType();
const wxEventType GlobalEvent::PlasmaResourceListFailedDownload = wxNewEventType();
const wxEventType GlobalEvent::BattleSyncReload = wxNewEventType();
const wxEventType GlobalEvent::OnUpdateFinished = wxNewEventType();

// const wxEventType GlobalEvent::OnTimerUpdates = wxNewEventType();

// static wxEvtHandler* _evthandler=NULL;
static std::map<wxEventType, std::list<wxEvtHandler*>> evts;

GlobalEvent::GlobalEvent() : m_handler(NULL) {}

GlobalEvent::~GlobalEvent() {
  if (m_handler != NULL)
    _Disconnect(m_handler);
}

void GlobalEvent::Send(wxEventType type) {
  wxCommandEvent evt = wxCommandEvent(type);
  assert(evt.GetEventType() == type);
  Send(evt);
}

void GlobalEvent::Send(wxCommandEvent event) {
  std::list<wxEvtHandler*>& evtlist = evts[event.GetEventType()];
  //	printf("AddPendingEvent %lu %lu\n", evts.size(), evtlist.size());
  for (auto evt : evtlist) {
    //		printf("	AddPendingEvent %lu \n", evt);
    evt->AddPendingEvent(event);
  }
}

void GlobalEvent::_Connect(wxEvtHandler* evthandler, wxEventType id, wxObjectEventFunction func) {
  assert(evthandler != NULL);
  std::list<wxEvtHandler*>& evtlist = evts[id];
  for (auto evt : evtlist) {
    if (evt == evthandler) {
      //			printf("Double Evthandler\n");
      return;
    }
  }
  //	printf("connected event! %lu\n", evthandler);
  evthandler->Connect(id, func);
  evtlist.push_back(evthandler);
  assert(evtlist.size() > 0);
}

// static std::map<wxEventType, std::list<wxEvtHandler*>> evts;

void GlobalEvent::_Disconnect(wxEvtHandler* evthandler, wxEventType id) {
  std::map<wxEventType, std::list<wxEvtHandler*>>::iterator it;
  for (it = evts.begin(); it != evts.end(); ++it) {
    if ((id == 0) || (id == it->first)) {
      for (std::list<wxEvtHandler*>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
        if (*it2 == evthandler) {
          //					printf("Disconnect %lu\n", evthandler);
          if (id != 0) {
            evthandler->Disconnect(id);
          }
          it->second.erase(it2);
          //					printf("Deleted Handler\n");
          break;
        }
      }
    }
  }
}

void GlobalEvent::ConnectGlobalEvent(wxEvtHandler* evh, wxEventType id, wxObjectEventFunction func) {
  if (m_handler == NULL) {
    m_handler = evh;
  }
  assert(m_handler == evh); // assigning a different eventhandler from the same object shouldn't happen
  GlobalEvent::_Connect(evh, id, func);
}

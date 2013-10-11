#include "notificationmanager.h"

#include <lslutils/globalsmanager.h>
#include "../uiutils.h"
#include "../ui.h"
#include "../settings.h"
#include "../spring.h"
#include "../mainwindow.h"
#include "../utils/platform.h"
#include "../images/springlobby_64.png.h"
#include "inotification.h"

#include "sound/alsound.h"

#ifdef HAVE_LIBNOTIFY
#include "libnotify.h"
typedef LibnotifyNotification NotificationWrapperType;
//#elif defined(__WXMSW__) //this is currently completely screwed
//	#include "snarlnotification.h"
//	typedef SnarlNotification NotificationWrapperType;
#else
#include "toasternotification.h"
typedef ToasterNotification NotificationWrapperType;
#endif

const int this_timer_id = wxNewId();

BEGIN_EVENT_TABLE(NotificationManager, wxEvtHandler)
EVT_TIMER(this_timer_id, NotificationManager::OnTimer)
END_EVENT_TABLE()

NotificationManager& notificationManager() {
  static LSL::Util::LineInfo<NotificationManager> m(AT);
  static LSL::Util::GlobalObjectHolder<NotificationManager, LSL::Util::LineInfo<NotificationManager>> m_manager(m);
  return m_manager;
}

NotificationManager::NotificationManager()
  : wxEvtHandler()
  , m_notification_wrapper(new NotificationWrapperType(&ui().mw()))
  , m_rate_limit_timer(this, this_timer_id)
  , m_rate_limit_ms(2000)
  , m_showNotificationSink(this, &UiEvents::GetNotificationEventSender()) {
  ConnectGlobalEvent(this, GlobalEvent::OnQuit, wxObjectEventFunction(&NotificationManager::OnQuit));
}

NotificationManager::~NotificationManager() {
  m_rate_limit_timer.Stop();
  delete m_notification_wrapper;
  m_notification_wrapper = NULL;
}

void NotificationManager::OnQuit(wxCommandEvent& /*data*/) {
  delete m_notification_wrapper;
  m_notification_wrapper = NULL;
}

void NotificationManager::OnShowNotification(UiEvents::NotficationData data) {
  if (m_rate_limit_timer.IsRunning()) {
    m_eventDataBuffer.push_back(data);
    return;
  }
  ShowNotification(data);
  m_rate_limit_timer.Start(m_rate_limit_ms, wxTIMER_ONE_SHOT);
}

void NotificationManager::ShowNotification(const UiEvents::NotficationData& data) {
  if (sett().GetUseNotificationPopups()) {
    const bool disable_if_ingame = sett().Get<bool>(_T("/GUI/NotificationPopupDisableIngame"), true);
    if (m_notification_wrapper && !(disable_if_ingame && spring().IsRunning())) {
      //! \todo use image from customizations
      wxBitmap nmp(charArr2wxBitmap(springlobby_64_png, sizeof(springlobby_64_png)));
      m_notification_wrapper->Show(nmp, sett().GetNotificationPopupPosition(), data);
    }
  }
#ifndef DISABLE_SOUND
  if (sett().GetChatPMSoundNotificationEnabled())
    sound().pm();
#endif
}

void NotificationManager::OnTimer(wxTimerEvent& /*event*/) {
  UiEvents::NotficationData data;
  if (!m_eventDataBuffer.empty()) {
    if (m_eventDataBuffer.size() == 1)
      data = m_eventDataBuffer.front();
    else
      data.second = wxFormat(_("%d more events")) % m_eventDataBuffer.size();
    m_eventDataBuffer.clear();
    ShowNotification(data);
  }
}

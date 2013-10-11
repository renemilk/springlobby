#ifndef DOWNLOADSOBSERVER_H
#define DOWNLOADSOBSERVER_H
#include <list>
#include <map>
#include <wx/thread.h>
#include <wx/string.h>
#include "lib/src/Downloader/Download.h"
#include "lib/src/Downloader/IDownloadsObserver.h"

class wxMutex;
class wxString;

//! Strcuture which contain information about downloading file
class ObserverDownloadInfo {
public:
  ObserverDownloadInfo();

  //! Size of file in bytes
  unsigned int size;

  //! Downloaded part in bytes
  unsigned int progress;

  //! Category of file
  IDownload::category cat;

  //! Is file downloaded
  bool finished;

  //! Name of the file (map_1)
  wxString filename;

  //! Original name (Map 1)
  wxString name;

private:
  //! Get download infromations from IDownload
  ObserverDownloadInfo(IDownload* dl);

  friend class DownloadsObserver;
};

//! DownloadsObserver collect and control information about downloads
//! This class is thread-safe
class DownloadsObserver : public IDownloadsObserver {
public:
  DownloadsObserver();
  virtual ~DownloadsObserver();

  //! Add information about download
  //! This function called from IDownload::IDownload
  virtual void Add(IDownload* dl);

  //! Move information about download from m_dl_list to m_finished_list
  //! This function called from IDownload::~IDownload
  virtual void Remove(IDownload* dl);

  //! Fill out list with donwloads
  void GetList(std::list<ObserverDownloadInfo>& lst);

  //! Fill out map with downloads (Key: Original name)
  void GetMap(std::map<wxString, ObserverDownloadInfo>& map);

  //! Delete all informations about finished downloads
  void ClearFinished();

private:
  //! Creatre infromation about download
  ObserverDownloadInfo GetInfo(IDownload* dl);

  //! List with downloads which are in process
  std::list<IDownload*> m_dl_list;

  //! List with finished downloads
  std::list<ObserverDownloadInfo> m_finished_list;

  //! Mutex fir functions Add, Remove, GetList, GetMap
  wxMutex mutex;
};

DownloadsObserver& downloadsObserver();

#endif // DOWNLOADSOBSERVER_H

#ifndef WINDOWATTRIBUTESPICKLE_H
#define WINDOWATTRIBUTESPICKLE_H

#include <wx/string.h>
#include <wx/toplevel.h>
#include "../utils/mixins.hh"

class wxSize;
//! automagically load/save window size and position in ctor/dtor
class WindowAttributesPickle : public SL::NonCopyable {
public:
  WindowAttributesPickle(const wxString& name, wxTopLevelWindow* window, const wxSize& default_size);
  virtual ~WindowAttributesPickle();

protected:
  void SaveAttributes();
  void LoadAttributes();

private:
  const wxString m_name;
  wxTopLevelWindow* m_window;
  const wxSize m_default_size;
};

class WindowHintsPickle : public SL::NonCopyable {
public:
  WindowHintsPickle(const wxString& name, wxTopLevelWindow* window, const wxSize& default_size);
  virtual ~WindowHintsPickle();

protected:
  void SaveAttributes();
  void LoadAttributes();

private:
  const wxString m_name;
  wxTopLevelWindow* m_window;
  const wxSize m_default_size;
};

#endif // WINDOWATTRIBUTESPICKLE_H

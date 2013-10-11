/* Copyright (C) 2007-2011 The SpringLobby Team. All rights reserved. */
#include "tasutil.h"
#include "conversion.h"

#include <wx/regex.h>

wxString GetWordParam(wxString& params) { return GetParamByChar(params, _T(' ')); }

wxString GetSentenceParam(wxString& params) { return GetParamByChar(params, _T('\t')); }

long GetIntParam(wxString& params) { return s2l(GetParamByChar(params, _T(' '))); }

wxString GetParamByChar(wxString& params, const wxChar& sep) {
  int pos = params.Find(sep);
  wxString ret;
  if (pos != -1) {
    ret = params.Left(pos);
    params = params.Mid(pos + 1);
  } else {
    ret = params;
    params = _T("");
  }
  return ret;
}

bool GetBoolParam(wxString& params) { return (bool)GetIntParam(params); }

bool IsValidNickname(const wxString& _name) {
  wxString name = _name;
  // The Regex Container
  // wxRegEx regex( wxT("[:graph:]") );
  wxRegEx regex(wxT("[ \t\r\n\v\föäüß, .:<>\\!§$%&+-]"));

  // We need to escape all regular Expression Characters, that have a special Meaning
  name.Replace(_T("["), _T(""));
  name.Replace(_T("]"), _T(""));

  if (name.IsEmpty())
    return false;

  return !regex.Matches(name);
}

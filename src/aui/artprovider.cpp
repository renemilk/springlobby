#include "artprovider.h"
#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/aui/framemanager.h>
#include <wx/aui/dockart.h>
#include <wx/aui/floatpane.h>
#include "auiutils.h"
#include "../uiutils.h"

SLArtProvider::SLArtProvider() {
  m_normal_font = *wxNORMAL_FONT;
  m_selected_font = *wxNORMAL_FONT;
  m_selected_font.SetWeight(wxBOLD);
  m_measuring_font = m_selected_font;

  m_fixed_tab_width = 100;
  m_tab_ctrl_height = 0;

#if defined(__WXMAC__) && defined(__WXOSX_CARBON__)
  wxBrush toolbarbrush;
  toolbarbrush.MacSetTheme(kThemeBrushToolbarBackground);
  wxColour base_colour = toolbarbrush.GetColour();
#else
  wxColour base_colour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
#endif
  m_active_colour = base_colour;

  // the base_colour is too pale to use as our base colour,
  // so darken it a bit --
  if ((255 - base_colour.Red()) + (255 - base_colour.Green()) + (255 - base_colour.Blue()) < 60) {
    base_colour = wxAuiStepColour(base_colour, 92);
  }

  m_base_colour = base_colour;
  wxColour border_colour = wxAuiStepColour(base_colour, 75);

  m_border_pen = wxPen(border_colour);
  m_base_colour_pen = wxPen(m_base_colour);
  m_base_colour_brush = wxBrush(m_base_colour);

  m_active_close_bmp = wxAuiBitmapFromBits(close_bits, 16, 16, *wxBLACK);
  m_disabled_close_bmp = wxAuiBitmapFromBits(close_bits, 16, 16, wxColour(128, 128, 128));

  m_active_left_bmp = wxAuiBitmapFromBits(left_bits, 16, 16, *wxBLACK);
  m_disabled_left_bmp = wxAuiBitmapFromBits(left_bits, 16, 16, wxColour(128, 128, 128));

  m_active_right_bmp = wxAuiBitmapFromBits(right_bits, 16, 16, *wxBLACK);
  m_disabled_right_bmp = wxAuiBitmapFromBits(right_bits, 16, 16, wxColour(128, 128, 128));

  m_active_windowlist_bmp = wxAuiBitmapFromBits(list_bits, 16, 16, *wxBLACK);
  m_disabled_windowlist_bmp = wxAuiBitmapFromBits(list_bits, 16, 16, wxColour(128, 128, 128));

  m_flags = 0;
}

SLArtProvider::~SLArtProvider() {}

wxAuiTabArt* SLArtProvider::Clone() {
  SLArtProvider* art = new SLArtProvider;
  art->SetNormalFont(m_normal_font);
  art->SetSelectedFont(m_selected_font);
  art->SetMeasuringFont(m_measuring_font);

  return art;
}

void SLArtProvider::SetFlags(unsigned int flags) { m_flags = flags; }

void SLArtProvider::SetSizingInfo(const wxSize& tab_ctrl_size, size_t tab_count) {
  m_fixed_tab_width = 100;

  int tot_width = (int)tab_ctrl_size.x - GetIndentSize() - 4;

  if (m_flags & wxAUI_NB_CLOSE_BUTTON)
    tot_width -= m_active_close_bmp.GetWidth();
  if (m_flags & wxAUI_NB_WINDOWLIST_BUTTON)
    tot_width -= m_active_windowlist_bmp.GetWidth();

  if (tab_count > 0) {
    m_fixed_tab_width = tot_width / (int)tab_count;
  }

  if (m_fixed_tab_width < 100)
    m_fixed_tab_width = 100;

  if (m_fixed_tab_width > tot_width / 2)
    m_fixed_tab_width = tot_width / 2;

  if (m_fixed_tab_width > 220)
    m_fixed_tab_width = 220;

  m_tab_ctrl_height = tab_ctrl_size.y;
}

void SLArtProvider::DrawBackground(wxDC& dc, wxWindow* WXUNUSED(wnd), const wxRect& rect) {
  // draw background
  wxColour top_color = wxAuiStepColour(m_base_colour, 90);
  wxColour bottom_color = wxAuiStepColour(m_base_colour, 170);
  wxRect r;

  if (m_flags & wxAUI_NB_BOTTOM)
    r = wxRect(rect.x, rect.y, rect.width + 2, rect.height);
  // TODO: else if (m_flags &wxAUI_NB_LEFT) {}
  // TODO: else if (m_flags &wxAUI_NB_RIGHT) {}
  else // for wxAUI_NB_TOP
    r = wxRect(rect.x, rect.y, rect.width + 2, rect.height - 3);
  dc.GradientFillLinear(r, top_color, bottom_color, wxSOUTH);

  // draw base lines
  dc.SetPen(m_border_pen);
  int y = rect.GetHeight();
  int w = rect.GetWidth();

  if (m_flags & wxAUI_NB_BOTTOM) {
    dc.SetBrush(wxBrush(bottom_color));
    dc.DrawRectangle(-1, 0, w + 2, 4);
  }
      // TODO: else if (m_flags &wxAUI_NB_LEFT) {}
      // TODO: else if (m_flags &wxAUI_NB_RIGHT) {}
      else // for wxAUI_NB_TOP
  {
    dc.SetBrush(m_base_colour_brush);
    dc.DrawRectangle(-1, y - 4, w + 2, 4);
  }
}

// DrawTab() draws an individual tab.
//
// dc       - output dc
// in_rect  - rectangle the tab should be confined to
// caption  - tab's caption
// active   - whether or not the tab is active
// out_rect - actual output rectangle
// x_extent - the advance x; where the next tab should start

void SLArtProvider::DrawTab(wxDC& dc, wxWindow* wnd, const wxAuiNotebookPage& page, const wxRect& in_rect,
                            int close_button_state, wxRect* out_tab_rect, wxRect* out_button_rect, int* x_extent) {
  wxCoord normal_textx, normal_texty;
  wxCoord selected_textx, selected_texty;
  wxCoord texty;

  // if the caption is empty, measure some temporary text
  wxString caption = page.caption;
  if (caption.empty())
    caption = wxT("Xj");

  dc.SetFont(m_selected_font);
  dc.GetTextExtent(caption, &selected_textx, &selected_texty);

  dc.SetFont(m_normal_font);
  dc.GetTextExtent(caption, &normal_textx, &normal_texty);

  // figure out the size of the tab
  wxSize tab_size = GetTabSize(dc, wnd, page.caption, page.bitmap, page.active, close_button_state, x_extent);

  wxCoord tab_height = m_tab_ctrl_height - 3;
  wxCoord tab_width = tab_size.x;
  wxCoord tab_x = in_rect.x;
  wxCoord tab_y = in_rect.y + in_rect.height - tab_height;

  caption = page.caption;

  // select pen, brush and font for the tab to be drawn

  if (page.active) {
    dc.SetFont(m_selected_font);
    texty = selected_texty;
  } else {
    dc.SetFont(m_normal_font);
    texty = normal_texty;
  }

  // create points that will make the tab outline

  int clip_width = tab_width;
  if (tab_x + clip_width > in_rect.x + in_rect.width)
    clip_width = (in_rect.x + in_rect.width) - tab_x;

  /*
      wxPoint clip_points[6];
      clip_points[0] = wxPoint(tab_x,              tab_y+tab_height-3);
      clip_points[1] = wxPoint(tab_x,              tab_y+2);
      clip_points[2] = wxPoint(tab_x+2,            tab_y);
      clip_points[3] = wxPoint(tab_x+clip_width-1, tab_y);
      clip_points[4] = wxPoint(tab_x+clip_width+1, tab_y+2);
      clip_points[5] = wxPoint(tab_x+clip_width+1, tab_y+tab_height-3);

      // FIXME: these ports don't provide wxRegion ctor from array of points
  #if !defined(__WXDFB__) && !defined(__WXCOCOA__)
      // set the clipping region for the tab --
      wxRegion clipping_region(WXSIZEOF(clip_points), clip_points);
      dc.SetClippingRegion(clipping_region);
  #endif // !wxDFB && !wxCocoa
  */
  // since the above code above doesn't play well with WXDFB or WXCOCOA,
  // we'll just use a rectangle for the clipping region for now --
  dc.SetClippingRegion(tab_x, tab_y, clip_width + 1, tab_height - 3);

  wxPoint border_points[6];
  if (m_flags & wxAUI_NB_BOTTOM) {
    border_points[0] = wxPoint(tab_x, tab_y);
    border_points[1] = wxPoint(tab_x, tab_y + tab_height - 6);
    border_points[2] = wxPoint(tab_x + 2, tab_y + tab_height - 4);
    border_points[3] = wxPoint(tab_x + tab_width - 2, tab_y + tab_height - 4);
    border_points[4] = wxPoint(tab_x + tab_width, tab_y + tab_height - 6);
    border_points[5] = wxPoint(tab_x + tab_width, tab_y);
  } else // if (m_flags & wxAUI_NB_TOP) {}
  {
    border_points[0] = wxPoint(tab_x, tab_y + tab_height - 4);
    border_points[1] = wxPoint(tab_x, tab_y + 2);
    border_points[2] = wxPoint(tab_x + 2, tab_y);
    border_points[3] = wxPoint(tab_x + tab_width - 2, tab_y);
    border_points[4] = wxPoint(tab_x + tab_width, tab_y + 2);
    border_points[5] = wxPoint(tab_x + tab_width, tab_y + tab_height - 4);
  }
  // TODO: else if (m_flags &wxAUI_NB_LEFT) {}
  // TODO: else if (m_flags &wxAUI_NB_RIGHT) {}

  int drawn_tab_yoff = border_points[1].y;
  int drawn_tab_height = border_points[0].y - border_points[1].y;

  if (page.active) {
    // draw active tab

    // draw base background color
    wxRect r(tab_x, tab_y, tab_width, tab_height);
    dc.SetPen(m_base_colour_pen);
    dc.SetBrush(m_base_colour_brush);
    dc.DrawRectangle(r.x + 1, r.y + 1, r.width - 1, r.height - 4);

    // this white helps fill out the gradient at the top of the tab
    dc.SetPen(*wxWHITE_PEN);
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.DrawRectangle(r.x + 2, r.y + 1, r.width - 3, r.height - 4);

    // these two points help the rounded corners appear more antialiased
    dc.SetPen(m_base_colour_pen);
    dc.DrawPoint(r.x + 2, r.y + 1);
    dc.DrawPoint(r.x + r.width - 2, r.y + 1);

    // set rectangle down a bit for gradient drawing
    r.SetHeight(r.GetHeight() / 2);
    r.x += 2;
    r.width -= 2;
    r.y += r.height;
    r.y -= 2;

    // draw gradient background
    wxColour top_color = *wxWHITE;
    wxColour bottom_color = m_base_colour;
    dc.GradientFillLinear(r, bottom_color, top_color, wxNORTH);
  } else {
    // draw inactive tab

    wxRect r(tab_x, tab_y + 1, tab_width, tab_height - 3);

    // start the gradent up a bit and leave the inside border inset
    // by a pixel for a 3D look.  Only the top half of the inactive
    // tab will have a slight gradient
    r.x += 3;
    r.y++;
    r.width -= 4;
    r.height /= 2;
    r.height--;

    // -- draw top gradient fill for glossy look
    wxColour top_color = m_base_colour;
    wxColour bottom_color = wxAuiStepColour(top_color, 160);
    dc.GradientFillLinear(r, bottom_color, top_color, wxNORTH);

    r.y += r.height;
    r.y--;

    // -- draw bottom fill for glossy look
    top_color = m_base_colour;
    bottom_color = m_base_colour;
    dc.GradientFillLinear(r, top_color, bottom_color, wxSOUTH);
  }

  // draw tab outline
  dc.SetPen(m_border_pen);
  dc.SetBrush(*wxTRANSPARENT_BRUSH);
  dc.DrawPolygon(WXSIZEOF(border_points), border_points);

  // there are two horizontal grey lines at the bottom of the tab control,
  // this gets rid of the top one of those lines in the tab control
  if (page.active) {
    if (m_flags & wxAUI_NB_BOTTOM)
      dc.SetPen(wxPen(wxColour(wxAuiStepColour(m_base_colour, 170))));
    // TODO: else if (m_flags &wxAUI_NB_LEFT) {}
    // TODO: else if (m_flags &wxAUI_NB_RIGHT) {}
    else // for wxAUI_NB_TOP
      dc.SetPen(m_base_colour_pen);
    dc.DrawLine(border_points[0].x + 1, border_points[0].y, border_points[5].x, border_points[5].y);
  }

  int text_offset = tab_x + 8;
  int close_button_width = 0;
  if (close_button_state != wxAUI_BUTTON_STATE_HIDDEN) {
    close_button_width = m_active_close_bmp.GetWidth();
  }

  int bitmap_offset = 0;
  if (page.bitmap.IsOk()) {
    bitmap_offset = tab_x + 8;

    // draw bitmap
    dc.DrawBitmap(page.bitmap, bitmap_offset, drawn_tab_yoff + (drawn_tab_height / 2) - (page.bitmap.GetHeight() / 2),
                  true);

    text_offset = bitmap_offset + page.bitmap.GetWidth();
    text_offset += 3; // bitmap padding
  } else {
    text_offset = tab_x + 8;
  }

  wxString draw_text = wxAuiChopText(dc, caption, tab_width - (text_offset - tab_x) - close_button_width);

  // draw tab text
  wxColour fg_color = dc.GetTextForeground();
  if (!page.active) {

    wxColour fg_color_new = fg_color;
    if (AreColoursSimilar(fg_color, dc.GetTextBackground(), 50)) {
      fg_color_new =
          wxColour((fg_color.Red() + 125) % 255, (fg_color.Green() + 125) % 255, (fg_color.Blue() + 125) % 255);
    }
    dc.SetTextForeground(wxAuiLightContrastColour(fg_color_new));
  }

  dc.DrawText(draw_text, text_offset, drawn_tab_yoff + (drawn_tab_height) / 2 - (texty / 2) - 1);
  dc.SetTextForeground((fg_color));

  // draw close button if necessary
  if (close_button_state != wxAUI_BUTTON_STATE_HIDDEN) {
    wxBitmap bmp = m_disabled_close_bmp;

    if (close_button_state == wxAUI_BUTTON_STATE_HOVER || close_button_state == wxAUI_BUTTON_STATE_PRESSED) {
      bmp = m_active_close_bmp;
    }

    wxRect rect(tab_x + tab_width - close_button_width - 1, tab_y + (tab_height / 2) - (bmp.GetHeight() / 2),
                close_button_width, tab_height);
    IndentPressedBitmap(&rect, close_button_state);
    dc.DrawBitmap(bmp, rect.x, rect.y, true);

    *out_button_rect = rect;
  }

  *out_tab_rect = wxRect(tab_x, tab_y, tab_width, tab_height);

#ifndef __WXMAC__
  // draw focus rectangle
  if (page.active && (wnd->FindFocus() == wnd)) {
    wxRect focusRectText(text_offset, (drawn_tab_yoff + (drawn_tab_height) / 2 - (texty / 2) - 1), selected_textx,
                         selected_texty);

    wxRect focusRect;
    wxRect focusRectBitmap;

    if (page.bitmap.IsOk())
      focusRectBitmap = wxRect(bitmap_offset, drawn_tab_yoff + (drawn_tab_height / 2) - (page.bitmap.GetHeight() / 2),
                               page.bitmap.GetWidth(), page.bitmap.GetHeight());

    if (page.bitmap.IsOk() && draw_text.IsEmpty())
      focusRect = focusRectBitmap;
    else if (!page.bitmap.IsOk() && !draw_text.IsEmpty())
      focusRect = focusRectText;
    else if (page.bitmap.IsOk() && !draw_text.IsEmpty())
      focusRect = focusRectText.Union(focusRectBitmap);

    focusRect.Inflate(2, 2);

    DrawFocusRect(wnd, dc, focusRect, 0);
  }
#endif

  dc.DestroyClippingRegion();
}

int SLArtProvider::GetIndentSize() { return 5; }

wxSize SLArtProvider::GetTabSize(wxDC& dc, wxWindow* WXUNUSED(wnd), const wxString& caption, const wxBitmap& bitmap,
                                 bool WXUNUSED(active), int close_button_state, int* x_extent) {
  wxCoord measured_textx, measured_texty, tmp;

  dc.SetFont(m_measuring_font);
  dc.GetTextExtent(caption, &measured_textx, &measured_texty);

  dc.GetTextExtent(wxT("ABCDEFXj"), &tmp, &measured_texty);

  // add padding around the text
  wxCoord tab_width = measured_textx;
  wxCoord tab_height = measured_texty;

  // if the close button is showing, add space for it
  if (close_button_state != wxAUI_BUTTON_STATE_HIDDEN)
    tab_width += m_active_close_bmp.GetWidth() + 3;

  // if there's a bitmap, add space for it
  if (bitmap.IsOk()) {
    tab_width += bitmap.GetWidth();
    tab_width += 3; // right side bitmap padding
    tab_height = wxMax(tab_height, bitmap.GetHeight());
  }

  // add padding
  tab_width += 16;
  tab_height += 10;

  if (m_flags & wxAUI_NB_TAB_FIXED_WIDTH) {
    tab_width = m_fixed_tab_width;
  }

  *x_extent = tab_width;

  return wxSize(tab_width, tab_height);
}

void SLArtProvider::DrawButton(wxDC& dc, wxWindow* WXUNUSED(wnd), const wxRect& in_rect, int bitmap_id,
                               int button_state, int orientation, wxRect* out_rect) {
  wxBitmap bmp;
  wxRect rect;

  switch (bitmap_id) {
    case wxAUI_BUTTON_CLOSE:
      if (button_state & wxAUI_BUTTON_STATE_DISABLED)
        bmp = m_disabled_close_bmp;
      else
        bmp = m_active_close_bmp;
      break;
    case wxAUI_BUTTON_LEFT:
      if (button_state & wxAUI_BUTTON_STATE_DISABLED)
        bmp = m_disabled_left_bmp;
      else
        bmp = m_active_left_bmp;
      break;
    case wxAUI_BUTTON_RIGHT:
      if (button_state & wxAUI_BUTTON_STATE_DISABLED)
        bmp = m_disabled_right_bmp;
      else
        bmp = m_active_right_bmp;
      break;
    case wxAUI_BUTTON_WINDOWLIST:
      if (button_state & wxAUI_BUTTON_STATE_DISABLED)
        bmp = m_disabled_windowlist_bmp;
      else
        bmp = m_active_windowlist_bmp;
      break;
    default:
      break;
  }

  if (!bmp.IsOk())
    return;

  rect = in_rect;

  if (orientation == wxLEFT) {
    rect.SetX(in_rect.x);
    rect.SetY(((in_rect.y + in_rect.height) / 2) - (bmp.GetHeight() / 2));
    rect.SetWidth(bmp.GetWidth());
    rect.SetHeight(bmp.GetHeight());
  } else {
    rect = wxRect(in_rect.x + in_rect.width - bmp.GetWidth(),
                  ((in_rect.y + in_rect.height) / 2) - (bmp.GetHeight() / 2), bmp.GetWidth(), bmp.GetHeight());
  }

  IndentPressedBitmap(&rect, button_state);
  dc.DrawBitmap(bmp, rect.x, rect.y, true);

  *out_rect = rect;
}

int SLArtProvider::ShowDropDown(wxWindow* wnd, const wxAuiNotebookPageArray& pages, int active_idx) {
  wxMenu menuPopup;

  size_t i, count = pages.GetCount();
  for (i = 0; i < count; ++i) {
    const wxAuiNotebookPage& page = pages.Item(i);
    wxString caption = page.caption;

    // if there is no caption, make it a space.  This will prevent
    // an assert in the menu code.
    if (caption.IsEmpty())
      caption = wxT(" ");

    menuPopup.AppendCheckItem(1000 + i, caption);
  }

  if (active_idx != -1) {
    menuPopup.Check(1000 + active_idx, true);
  }

  // find out where to put the popup menu of window items
  wxPoint pt = ::wxGetMousePosition();
  pt = wnd->ScreenToClient(pt);

  // find out the screen coordinate at the bottom of the tab ctrl
  wxRect cli_rect = wnd->GetClientRect();
  pt.y = cli_rect.y + cli_rect.height;

  wxAuiCommandCapture* cc = new wxAuiCommandCapture;
  wnd->PushEventHandler(cc);
  wnd->PopupMenu(&menuPopup, pt);
  int command = cc->GetCommandId();
  wnd->PopEventHandler(true);

  if (command >= 1000)
    return command - 1000;

  return -1;
}

int SLArtProvider::GetBestTabCtrlSize(wxWindow* wnd, const wxAuiNotebookPageArray& pages,
                                      const wxSize& required_bmp_size) {
  wxClientDC dc(wnd);
  dc.SetFont(m_measuring_font);

  // sometimes a standard bitmap size needs to be enforced, especially
  // if some tabs have bitmaps and others don't.  This is important because
  // it prevents the tab control from resizing when tabs are added.
  wxBitmap measure_bmp;
  if (required_bmp_size.IsFullySpecified()) {
    measure_bmp.Create(required_bmp_size.x, required_bmp_size.y);
  }

  int max_y = 0;
  size_t i, page_count = pages.GetCount();
  for (i = 0; i < page_count; ++i) {
    wxAuiNotebookPage& page = pages.Item(i);

    wxBitmap bmp;
    if (measure_bmp.IsOk())
      bmp = measure_bmp;
    else
      bmp = page.bitmap;

    // we don't use the caption text because we don't
    // want tab heights to be different in the case
    // of a very short piece of text on one tab and a very
    // tall piece of text on another tab
    int x_ext = 0;
    wxSize s = GetTabSize(dc, wnd, wxT("ABCDEFGHIj"), bmp, true, wxAUI_BUTTON_STATE_HIDDEN, &x_ext);

    max_y = wxMax(max_y, s.y);
  }

  return max_y + 2;
}

void SLArtProvider::SetNormalFont(const wxFont& font) { m_normal_font = font; }

void SLArtProvider::SetSelectedFont(const wxFont& font) { m_selected_font = font; }

void SLArtProvider::SetMeasuringFont(const wxFont& font) { m_measuring_font = font; }

void SLArtProvider::SetColour(const wxColour& colour) {
  m_base_colour = colour;
#ifdef HAVE_WX29
  m_border_pen = wxPen(m_base_colour.ChangeLightness(75));
#else
  //!todo
  m_border_pen = wxPen(m_base_colour);
#endif
  m_base_colour_pen = wxPen(m_base_colour);
  m_base_colour_brush = wxBrush(m_base_colour);
}

void SLArtProvider::SetActiveColour(const wxColour& colour) { m_active_colour = colour; }

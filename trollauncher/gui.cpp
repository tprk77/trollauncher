// Copyright (c) 2019 Tim Perkins

// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the “Software”), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
// to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.

// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "trollauncher/gui.hpp"

#include <functional>

#include <wx/filepicker.h>
#include <wx/wx.h>

#include "trollauncher/modpack_installer.hpp"
#include "trollauncher/utils.hpp"

namespace tl {

namespace {

namespace pl = std::placeholders;

class GuiApp : public wxApp {
 public:
  virtual bool OnInit() override;
};

class GuiFrame : public wxFrame {
 public:
  GuiFrame();

 private:
  void OnDoModpackInstall(wxCommandEvent& event);

  wxPanel* panel_ptr_;
  ModpackInstaller::Ptr mi_ptr_;
};

struct ModpackInstallData {
  std::string modpack_path;
  std::string profile_name;
  std::string profile_icon;
};

class GuiPanelModpackInstall : public wxPanel {
 public:
  GuiPanelModpackInstall(wxWindow* parent);

  ModpackInstallData GetData() const;

 private:
  wxFilePickerCtrl* path_picker_ptr_;
  wxTextCtrl* name_textbox_ptr_;
  wxChoice* icon_choice_ptr_;
};

constexpr int ID_DO_MODPACK_INSTALL = 1;

bool GuiApp::OnInit()
{
  wxFrame* frame_ptr = new GuiFrame();
  frame_ptr->Show(true);
  return true;
}

GuiFrame::GuiFrame() : wxFrame(NULL, wxID_ANY, "Trollauncher")
{
  wxFlexGridSizer* grid_ptr = new wxFlexGridSizer(1);
  panel_ptr_ = new GuiPanelModpackInstall(this);
  grid_ptr->Add(panel_ptr_);
  SetSizerAndFit(grid_ptr);
  // Lock down resizing
  SetMinSize(GetSize());
  SetMaxSize(GetSize());
  // Connect everything up
  Bind(wxEVT_BUTTON, &GuiFrame::OnDoModpackInstall, this, ID_DO_MODPACK_INSTALL);
  // Center the frame in the screen
  Center();
}

void GuiFrame::OnDoModpackInstall(wxCommandEvent&)
{
  const ModpackInstallData data = static_cast<GuiPanelModpackInstall*>(panel_ptr_)->GetData();
  if (data.modpack_path.empty()) {
    wxMessageBox("You must supply a modpack path.", "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  if (data.profile_name.empty()) {
    wxMessageBox("You must supply a profile name.", "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  if (data.profile_icon.empty()) {
    wxMessageBox("You must supply a profile icon.", "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  std::error_code ec;
  mi_ptr_ = ModpackInstaller::Create(data.modpack_path, &ec);
  if (mi_ptr_ == nullptr) {
    const auto text = wxString::Format("Cannot initialize installer!\n\n%s.", ec.message());
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  mi_ptr_->SetName(data.profile_name);
  mi_ptr_->SetIcon(data.profile_icon);
  if (!mi_ptr_->Install(&ec)) {
    const auto text = wxString::Format("Cannot install modpack!\n\n%s.", ec.message());
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  const auto text = wxString::Format("Modpack installed successfully.", ec.message());
  wxMessageBox(text, "Error", wxOK, this);
  Close();
}

GuiPanelModpackInstall::GuiPanelModpackInstall(wxWindow* parent) : wxPanel(parent)
{
  constexpr int MIN_CONTOL_WIDTH = 300;
  constexpr int BORDER_WIDTH = 10;
  wxStaticText* path_text_ptr = new wxStaticText(this, wxID_ANY, "Modpack Zip");
  wxStaticText* name_text_ptr = new wxStaticText(this, wxID_ANY, "Profile Name");
  wxStaticText* icon_text_ptr = new wxStaticText(this, wxID_ANY, "Profile Icon");
  path_picker_ptr_ = new wxFilePickerCtrl(this, wxID_ANY);
  path_picker_ptr_->SetMinSize(wxSize(MIN_CONTOL_WIDTH, path_picker_ptr_->GetSize().GetHeight()));
  // TODO Populate name with default unique name?
  name_textbox_ptr_ = new wxTextCtrl(this, wxID_ANY);
  name_textbox_ptr_->SetMinSize(wxSize(MIN_CONTOL_WIDTH, name_textbox_ptr_->GetSize().GetHeight()));
  icon_choice_ptr_ = new wxChoice(this, wxID_ANY);
  icon_choice_ptr_->SetMinSize(wxSize(MIN_CONTOL_WIDTH, icon_choice_ptr_->GetSize().GetHeight()));
  const std::vector<std::string> icons = GetDefaultLauncherIcons();
  const int random_index = std::rand() % icons.size();
  for (const auto& icon : icons) {
    icon_choice_ptr_->Append(icon);
  }
  icon_choice_ptr_->Select(random_index);
  wxButton* install_button_ptr = new wxButton(this, ID_DO_MODPACK_INSTALL, "Install Modpack");
  install_button_ptr->SetMinSize(
      wxSize(MIN_CONTOL_WIDTH, 2 * install_button_ptr->GetSize().GetHeight()));
  wxFlexGridSizer* grid_ptr = new wxFlexGridSizer(2);
  const auto text_flags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL, BORDER_WIDTH);
  const auto border_flags = wxSizerFlags().Border(wxALL, BORDER_WIDTH);
  grid_ptr->Add(path_text_ptr, text_flags);
  grid_ptr->Add(path_picker_ptr_, border_flags);
  grid_ptr->Add(name_text_ptr, text_flags);
  grid_ptr->Add(name_textbox_ptr_, border_flags);
  grid_ptr->Add(icon_text_ptr, text_flags);
  grid_ptr->Add(icon_choice_ptr_, border_flags);
  grid_ptr->Add(1, 1);
  grid_ptr->Add(install_button_ptr, border_flags);
  wxFlexGridSizer* padder_ptr = new wxFlexGridSizer(1);
  padder_ptr->Add(grid_ptr, border_flags);
  SetSizer(padder_ptr);
}

ModpackInstallData GuiPanelModpackInstall::GetData() const
{
  return ModpackInstallData{
      path_picker_ptr_->GetPath().ToStdString(),
      name_textbox_ptr_->GetValue().Strip(wxString::both).ToStdString(),
      icon_choice_ptr_->GetString(icon_choice_ptr_->GetSelection()).ToStdString(),
  };
}

wxIMPLEMENT_WX_THEME_SUPPORT;
wxIMPLEMENT_APP_NO_MAIN(GuiApp);

}  // namespace

int GuiMain(const int, const char* const[])
{
  wxInitializer initializer(0, static_cast<char**>(nullptr));
  if (!initializer.IsOk()) {
    return 1;
  }
  if (!wxGetApp().CallOnInit()) {
    return 1;
  }
  struct Exiter {
    ~Exiter()
    {
      wxGetApp().OnExit();
    }
  } exiter;
  return wxGetApp().OnRun();
}

}  // namespace tl

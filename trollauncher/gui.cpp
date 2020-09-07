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
#include <regex>

#include <wx/filepicker.h>
#include <wx/mimetype.h>
#include <wx/mstream.h>
#include <wx/progdlg.h>
#include <wx/utils.h>
#include <wx/wx.h>

#include "trollauncher/mc_process_detector.hpp"
#include "trollauncher/modpack_installer.hpp"
#include "trollauncher/utils.hpp"

namespace tl {

namespace {

namespace pl = std::placeholders;

class GuiApp : public wxApp {
 public:
  virtual bool OnInit() override;
};

wxIMPLEMENT_WX_THEME_SUPPORT wxIMPLEMENT_APP_NO_MAIN(GuiApp);

class GuiFrame : public wxFrame {
 public:
  GuiFrame();

 private:
  void OnSelectInstall(wxCommandEvent& event);
  void OnSelectUpdate(wxCommandEvent& event);
  void OnDoModpackInstall(wxCommandEvent& event);
  void OnDoModpackUpdate(wxCommandEvent& event);

  wxPanel* panel_ptr_;
  ModpackInstaller::Ptr mi_ptr_;
  ModpackUpdater::Ptr mu_ptr_;
};

struct ModpackInstallData {
  std::string modpack_path;
  std::string profile_name;
  std::string profile_icon;
};

constexpr int ID_SELECT_INSTALL = 1;
constexpr int ID_SELECT_UPDATE = 2;
constexpr int ID_DO_MODPACK_INSTALL = 3;
constexpr int ID_DO_MODPACK_UPDATE = 4;

class GuiPanelModeSelector : public wxPanel {
 public:
  GuiPanelModeSelector(wxWindow* parent);
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

struct ModpackUpdateData {
  std::string modpack_path;
  std::string profile_id;
};

class GuiPanelModpackUpdate : public wxPanel {
 public:
  GuiPanelModpackUpdate(wxWindow* parent);

  ModpackUpdateData GetData() const;

 private:
  std::vector<ProfileData> profile_datas_;
  wxFilePickerCtrl* path_picker_ptr_;
  wxChoice* profile_choice_ptr_;
};

class GuiDialogForgePromo : public wxDialog {
 public:
  GuiDialogForgePromo(wxWindow* parent);

  void OnGotoForgeSite(wxCommandEvent& event);
  void OnGotoForgePatreon(wxCommandEvent& event);

 private:
  static constexpr const char* FORGE_SITE_URL = "https://files.minecraftforge.net";
  static constexpr const char* FORGE_PATREON_URL = "https://www.patreon.com/LexManos";
};

constexpr int ID_GOTO_FORGE_SITE = 1;
constexpr int ID_GOTO_FORGE_PATREON = 2;

class GuiDialogForgeNotice : public wxDialog {
 public:
  GuiDialogForgeNotice(wxWindow* parent);
};

class GuiDialogProgress : public wxProgressDialog {
 public:
  GuiDialogProgress(wxWindow* parent);
};

std::string GetProcessRunningMessage(McProcessRunning process_running, bool can_continue);
void OpenUrlInBrowser(const char* const url);
std::vector<std::string> GetUniqueProfileNames(const std::vector<ProfileData>& profile_datas);
const wxBitmap& GetTrollfaceIcon16x16();

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

namespace {

bool GuiApp::OnInit()
{
  wxImage::AddHandler(new wxPNGHandler());
  wxFrame* frame_ptr = new GuiFrame();
  frame_ptr->Show(true);
  const McProcessRunning process_running = McProcessDetector::GetRunningMinecraft();
  if (process_running != McProcessRunning::NONE) {
    const auto text = wxString::Format("Detected running Minecraft processes!\n\n%s",
                                       GetProcessRunningMessage(process_running, false));
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, frame_ptr);
    return false;
  }
  return true;
}

GuiFrame::GuiFrame() : wxFrame(NULL, wxID_ANY, "Trollauncher")
{
  wxFlexGridSizer* grid_ptr = new wxFlexGridSizer(1);
  panel_ptr_ = new GuiPanelModeSelector(this);
  grid_ptr->Add(panel_ptr_);
  SetSizerAndFit(grid_ptr);
  // Lock down resizing
  SetMinSize(GetSize());
  SetMaxSize(GetSize());
  // Connect everything up
  Bind(wxEVT_BUTTON, &GuiFrame::OnSelectInstall, this, ID_SELECT_INSTALL);
  Bind(wxEVT_BUTTON, &GuiFrame::OnSelectUpdate, this, ID_SELECT_UPDATE);
  Bind(wxEVT_BUTTON, &GuiFrame::OnDoModpackInstall, this, ID_DO_MODPACK_INSTALL);
  Bind(wxEVT_BUTTON, &GuiFrame::OnDoModpackUpdate, this, ID_DO_MODPACK_UPDATE);
  // Set the icon
  wxIcon trollface_icon;
  trollface_icon.CopyFromBitmap(GetTrollfaceIcon16x16());
  SetIcon(trollface_icon);
  // Center the frame in the screen
  Center();
}

void GuiFrame::OnSelectInstall(wxCommandEvent&)
{
  // Unlock resizing
  SetMinSize(wxSize(-1, -1));
  SetMaxSize(wxSize(-1, -1));
  // Replace the previous pannel
  panel_ptr_->Destroy();
  wxFlexGridSizer* grid_ptr = new wxFlexGridSizer(1);
  panel_ptr_ = new GuiPanelModpackInstall(this);
  grid_ptr->Add(panel_ptr_);
  SetSizerAndFit(grid_ptr);
  // Lock down resizing
  SetMinSize(GetSize());
  SetMaxSize(GetSize());
}

void GuiFrame::OnSelectUpdate(wxCommandEvent&)
{
  // Unlock resizing
  SetMinSize(wxSize(-1, -1));
  SetMaxSize(wxSize(-1, -1));
  // Replace the previous pannel
  panel_ptr_->Destroy();
  wxFlexGridSizer* grid_ptr = new wxFlexGridSizer(1);
  panel_ptr_ = new GuiPanelModpackUpdate(this);
  grid_ptr->Add(panel_ptr_);
  SetSizerAndFit(grid_ptr);
  // Lock down resizing
  SetMinSize(GetSize());
  SetMaxSize(GetSize());
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
  const McProcessRunning process_running = McProcessDetector::GetRunningMinecraft();
  if (process_running != McProcessRunning::NONE) {
    const auto text = wxString::Format("Detected running Minecraft processes!\n\n%s",
                                       GetProcessRunningMessage(process_running, true));
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  std::error_code ec;
  mi_ptr_ = ModpackInstaller::Create(data.modpack_path, &ec);
  if (mi_ptr_ == nullptr) {
    const auto text = wxString::Format("Cannot initialize installer!\n\n%s.", ec.message());
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  if (!mi_ptr_->PrepInstaller(&ec)) {
    const auto text = wxString::Format("Cannot prepare installer!\n\n%s.", ec.message());
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  GuiDialogForgePromo forge_promo_dialog(this);
  if (forge_promo_dialog.ShowModal() != wxID_OK) {
    return;
  }
  if (!mi_ptr_->IsForgeInstalled().value()) {
    GuiDialogForgeNotice forge_notice_dialog(this);
    if (forge_notice_dialog.ShowModal() != wxID_OK) {
      return;
    }
  }
  GuiDialogProgress update_progress_dialog(this);
  auto progress_func = [&update_progress_dialog](std::size_t percent, const std::string& message) {
    update_progress_dialog.Update(percent, message);
  };
  if (!mi_ptr_->Install(data.profile_name, data.profile_icon, &ec, progress_func)) {
    const auto text = wxString::Format("Cannot install modpack!\n\n%s.", ec.message());
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  wxMessageBox("Modpack installed successfully.", "Success", wxOK, this);
  Close();
}

void GuiFrame::OnDoModpackUpdate(wxCommandEvent&)
{
  const ModpackUpdateData data = static_cast<GuiPanelModpackUpdate*>(panel_ptr_)->GetData();
  if (data.modpack_path.empty()) {
    wxMessageBox("You must supply a modpack path.", "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  if (data.profile_id.empty()) {
    wxMessageBox("You must supply a profile ID.", "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  const McProcessRunning process_running = McProcessDetector::GetRunningMinecraft();
  if (process_running != McProcessRunning::NONE) {
    const auto text = wxString::Format("Detected running Minecraft processes!\n\n%s",
                                       GetProcessRunningMessage(process_running, true));
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  std::error_code ec;
  mu_ptr_ = ModpackUpdater::Create(data.profile_id, data.modpack_path, &ec);
  if (mu_ptr_ == nullptr) {
    const auto text = wxString::Format("Cannot initialize updater!\n\n%s.", ec.message());
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  if (!mu_ptr_->PrepInstaller(&ec)) {
    const auto text = wxString::Format("Cannot prepare installer!\n\n%s.", ec.message());
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  GuiDialogForgePromo forge_promo_dialog(this);
  if (forge_promo_dialog.ShowModal() != wxID_OK) {
    return;
  }
  if (!mu_ptr_->IsForgeInstalled().value()) {
    GuiDialogForgeNotice forge_notice_dialog(this);
    if (forge_notice_dialog.ShowModal() != wxID_OK) {
      return;
    }
  }
  GuiDialogProgress update_progress_dialog(this);
  auto progress_func = [&update_progress_dialog](std::size_t percent, const std::string& message) {
    update_progress_dialog.Update(percent, message);
  };
  if (!mu_ptr_->Update(&ec, progress_func)) {
    const auto text = wxString::Format("Cannot update modpack!\n\n%s.", ec.message());
    wxMessageBox(text, "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  wxMessageBox("Modpack updated successfully.", "Success", wxOK, this);
  Close();
}

GuiPanelModeSelector::GuiPanelModeSelector(wxWindow* parent) : wxPanel(parent)
{
  constexpr int BORDER_WIDTH = 10;
  constexpr int MIN_BUTTON_WIDTH = 300;
  wxButton* install_button_ptr = new wxButton(this, ID_SELECT_INSTALL, "Install Modpack");
  install_button_ptr->SetMinSize(
      wxSize(MIN_BUTTON_WIDTH, 2 * install_button_ptr->GetSize().GetHeight()));
  wxButton* update_button_ptr = new wxButton(this, ID_SELECT_UPDATE, "Update Modpack");
  update_button_ptr->SetMinSize(
      wxSize(MIN_BUTTON_WIDTH, 2 * update_button_ptr->GetSize().GetHeight()));
  const auto control_flags = wxSizerFlags().Expand().Border(wxALL, BORDER_WIDTH);
  wxFlexGridSizer* button_grid_ptr = new wxFlexGridSizer(1);
  button_grid_ptr->AddGrowableCol(0);
  button_grid_ptr->Add(install_button_ptr, control_flags);
  button_grid_ptr->Add(update_button_ptr, control_flags);
  wxFlexGridSizer* padder_ptr = new wxFlexGridSizer(1);
  const auto padder_flags = wxSizerFlags().Expand().Border(wxALL, BORDER_WIDTH);
  padder_ptr->Add(button_grid_ptr, padder_flags);
  SetSizer(padder_ptr);
}

GuiPanelModpackInstall::GuiPanelModpackInstall(wxWindow* parent) : wxPanel(parent)
{
  constexpr int BORDER_WIDTH = 10;
  constexpr int MIN_BUTTON_WIDTH = 300;
  wxStaticText* header_text_ptr = new wxStaticText(
      this, wxID_ANY, "Use this utility to create Minecraft Launcher profiles for modpacks.");
  header_text_ptr->SetFont(header_text_ptr->GetFont().Bold());
  wxStaticText* info_text_ptr = new wxStaticText(
      this, wxID_ANY, "The new profile will be installed under the '.minecraft' directory.\n");
  wxStaticText* path_text_ptr = new wxStaticText(this, wxID_ANY, "Modpack Zip");
  wxStaticText* name_text_ptr = new wxStaticText(this, wxID_ANY, "Profile Name");
  wxStaticText* icon_text_ptr = new wxStaticText(this, wxID_ANY, "Profile Icon");
  path_picker_ptr_ = new wxFilePickerCtrl(this, wxID_ANY);
  // TODO Populate name with default unique name?
  name_textbox_ptr_ = new wxTextCtrl(this, wxID_ANY);
  icon_choice_ptr_ = new wxChoice(this, wxID_ANY);
  const std::vector<std::string> icons = GetDefaultLauncherIcons();
  const int random_index = std::rand() % icons.size();
  for (const auto& icon : icons) {
    icon_choice_ptr_->Append(icon);
  }
  icon_choice_ptr_->Select(random_index);
  wxButton* install_button_ptr = new wxButton(this, ID_DO_MODPACK_INSTALL, "Install Modpack");
  // Make the button double the height
  install_button_ptr->SetMinSize(
      wxSize(MIN_BUTTON_WIDTH, 2 * install_button_ptr->GetSize().GetHeight()));
  const auto header_flags = wxSizerFlags().Center().Border(wxALL, BORDER_WIDTH);
  const auto text_flags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL, BORDER_WIDTH);
  const auto control_flags = wxSizerFlags().Expand().Border(wxALL, BORDER_WIDTH);
  wxFlexGridSizer* info_grid_ptr = new wxFlexGridSizer(1);
  info_grid_ptr->AddGrowableCol(0);
  info_grid_ptr->Add(header_text_ptr, header_flags);
  info_grid_ptr->Add(info_text_ptr, header_flags);
  wxFlexGridSizer* field_grid_ptr = new wxFlexGridSizer(2);
  field_grid_ptr->AddGrowableCol(1);
  field_grid_ptr->Add(path_text_ptr, text_flags);
  field_grid_ptr->Add(path_picker_ptr_, control_flags);
  field_grid_ptr->Add(name_text_ptr, text_flags);
  field_grid_ptr->Add(name_textbox_ptr_, control_flags);
  field_grid_ptr->Add(icon_text_ptr, text_flags);
  field_grid_ptr->Add(icon_choice_ptr_, control_flags);
  wxFlexGridSizer* button_grid_ptr = new wxFlexGridSizer(1);
  button_grid_ptr->AddGrowableCol(0);
  button_grid_ptr->Add(install_button_ptr, control_flags);
  wxFlexGridSizer* padder_ptr = new wxFlexGridSizer(1);
  const auto top_flags = wxSizerFlags().Expand().Border(wxALL & ~wxBOTTOM, BORDER_WIDTH);
  const auto bottom_flags = wxSizerFlags().Expand().Border(wxALL & ~wxTOP, BORDER_WIDTH);
  padder_ptr->Add(info_grid_ptr, top_flags);
  padder_ptr->Add(field_grid_ptr, bottom_flags);
  padder_ptr->Add(button_grid_ptr, bottom_flags);
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

GuiPanelModpackUpdate::GuiPanelModpackUpdate(wxWindow* parent) : wxPanel(parent)
{
  constexpr int BORDER_WIDTH = 10;
  constexpr int MIN_BUTTON_WIDTH = 300;
  wxStaticText* header_text_ptr = new wxStaticText(
      this, wxID_ANY, "Use this utility to update Minecraft Launcher profiles for modpacks.");
  header_text_ptr->SetFont(header_text_ptr->GetFont().Bold());
  wxStaticText* info_text_ptr = new wxStaticText(
      this, wxID_ANY, "Mods and configs will be updated, saves will not be modified.\n");
  wxStaticText* path_text_ptr = new wxStaticText(this, wxID_ANY, "Modpack Zip");
  wxStaticText* profile_text_ptr = new wxStaticText(this, wxID_ANY, "Profile");
  path_picker_ptr_ = new wxFilePickerCtrl(this, wxID_ANY);
  profile_choice_ptr_ = new wxChoice(this, wxID_ANY);
  std::error_code ec;  // But actually just ignore any errors
  profile_datas_ = GetInstalledProfiles(&ec);
  const std::vector<std::string> profiles = GetUniqueProfileNames(profile_datas_);
  for (const auto& profile : profiles) {
    profile_choice_ptr_->Append(profile);
  }
  profile_choice_ptr_->Select(0);
  wxButton* update_button_ptr = new wxButton(this, ID_DO_MODPACK_UPDATE, "Update Modpack");
  // Make the button double the height
  update_button_ptr->SetMinSize(
      wxSize(MIN_BUTTON_WIDTH, 2 * update_button_ptr->GetSize().GetHeight()));
  const auto header_flags = wxSizerFlags().Center().Border(wxALL, BORDER_WIDTH);
  const auto text_flags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL, BORDER_WIDTH);
  const auto control_flags = wxSizerFlags().Expand().Border(wxALL, BORDER_WIDTH);
  wxFlexGridSizer* info_grid_ptr = new wxFlexGridSizer(1);
  info_grid_ptr->AddGrowableCol(0);
  info_grid_ptr->Add(header_text_ptr, header_flags);
  info_grid_ptr->Add(info_text_ptr, header_flags);
  wxFlexGridSizer* field_grid_ptr = new wxFlexGridSizer(2);
  field_grid_ptr->AddGrowableCol(1);
  field_grid_ptr->Add(path_text_ptr, text_flags);
  field_grid_ptr->Add(path_picker_ptr_, control_flags);
  field_grid_ptr->Add(profile_text_ptr, text_flags);
  field_grid_ptr->Add(profile_choice_ptr_, control_flags);
  wxFlexGridSizer* button_grid_ptr = new wxFlexGridSizer(1);
  button_grid_ptr->AddGrowableCol(0);
  button_grid_ptr->Add(update_button_ptr, control_flags);
  wxFlexGridSizer* padder_ptr = new wxFlexGridSizer(1);
  const auto top_flags = wxSizerFlags().Expand().Border(wxALL & ~wxBOTTOM, BORDER_WIDTH);
  const auto bottom_flags = wxSizerFlags().Expand().Border(wxALL & ~wxTOP, BORDER_WIDTH);
  padder_ptr->Add(info_grid_ptr, top_flags);
  padder_ptr->Add(field_grid_ptr, bottom_flags);
  padder_ptr->Add(button_grid_ptr, bottom_flags);
  SetSizer(padder_ptr);
}

ModpackUpdateData GuiPanelModpackUpdate::GetData() const
{
  return ModpackUpdateData{
      path_picker_ptr_->GetPath().ToStdString(),
      profile_datas_.at(profile_choice_ptr_->GetSelection()).id,
  };
}

GuiDialogForgePromo::GuiDialogForgePromo(wxWindow* parent) : wxDialog(parent, wxID_ANY, "Forge")
{
  constexpr int MIN_CONTOL_WIDTH = 300;
  constexpr int BORDER_WIDTH = 10;
  wxFlexGridSizer* grid_ptr = new wxFlexGridSizer(1);
  wxStaticText* header_text_ptr =
      new wxStaticText(this, wxID_ANY, "\nThis Modpack depends on Forge!");
  header_text_ptr->SetFont(header_text_ptr->GetFont().Bold());
  wxStaticText* body_text_ptr =
      new wxStaticText(this, wxID_ANY,
                       "Forge is supported by ads and donations.\n\nPlease consider helping Forge "
                       "by visiting the\nwebsite, or by donating to Lex's Patreon.\n");
  wxButton* site_button_ptr = new wxButton(this, ID_GOTO_FORGE_SITE, "Visit Forge's Website");
  site_button_ptr->SetMinSize(wxSize(MIN_CONTOL_WIDTH, 2 * site_button_ptr->GetSize().GetHeight()));
  wxButton* patreon_button_ptr = new wxButton(this, ID_GOTO_FORGE_PATREON, "Visit Lex's Patreon");
  patreon_button_ptr->SetMinSize(
      wxSize(MIN_CONTOL_WIDTH, 2 * patreon_button_ptr->GetSize().GetHeight()));
  wxButton* ok_button_ptr = new wxButton(this, wxID_OK);
  ok_button_ptr->SetMinSize(wxSize(MIN_CONTOL_WIDTH, 2 * ok_button_ptr->GetSize().GetHeight()));
  const auto border_flags = wxSizerFlags().Center().Border(wxALL, BORDER_WIDTH);
  grid_ptr->Add(header_text_ptr, border_flags);
  grid_ptr->Add(body_text_ptr, border_flags);
  grid_ptr->Add(site_button_ptr, border_flags);
  grid_ptr->Add(patreon_button_ptr, border_flags);
  grid_ptr->Add(ok_button_ptr, border_flags);
  wxFlexGridSizer* padder_ptr = new wxFlexGridSizer(1);
  padder_ptr->Add(grid_ptr, border_flags);
  SetSizerAndFit(padder_ptr);
  // Lock down resizing
  SetMinSize(GetSize());
  SetMaxSize(GetSize());
  // Connect everything up
  Bind(wxEVT_BUTTON, &GuiDialogForgePromo::OnGotoForgeSite, this, ID_GOTO_FORGE_SITE);
  Bind(wxEVT_BUTTON, &GuiDialogForgePromo::OnGotoForgePatreon, this, ID_GOTO_FORGE_PATREON);
  // Cancel on close, etc
  SetEscapeId(wxID_CANCEL);
}

void GuiDialogForgePromo::OnGotoForgeSite(wxCommandEvent&)
{
  OpenUrlInBrowser(FORGE_SITE_URL);
}

void GuiDialogForgePromo::OnGotoForgePatreon(wxCommandEvent&)
{
  OpenUrlInBrowser(FORGE_PATREON_URL);
}

GuiDialogForgeNotice::GuiDialogForgeNotice(wxWindow* parent) : wxDialog(parent, wxID_ANY, "Forge")
{
  constexpr int MIN_CONTOL_WIDTH = 300;
  constexpr int BORDER_WIDTH = 10;
  wxFlexGridSizer* grid_ptr = new wxFlexGridSizer(1);
  wxStaticText* header_text_ptr =
      new wxStaticText(this, wxID_ANY, "\nThe Forge Installer is about to run!");
  header_text_ptr->SetFont(header_text_ptr->GetFont().Bold());
  wxStaticText* body_text_ptr = new wxStaticText(
      this, wxID_ANY,
      "The Forge Installer is not currently automated.\nThis process must be completed "
      "manually.\nDon't worry, it's easy.\n");
  wxStaticText* steps_text_ptr =
      new wxStaticText(this, wxID_ANY, "Step 1:\tSelect \"Install client\"\n\nStep 2:\tPress OK\n");
  steps_text_ptr->SetFont(steps_text_ptr->GetFont().Bold());
  const wxSize orig_size = steps_text_ptr->GetSize();
  // This apparently fixes a weird alignment issue on Windows
  steps_text_ptr->SetMinSize(
      wxSize(static_cast<int>(1.1 * orig_size.GetWidth()), orig_size.GetHeight()));
  wxButton* ok_button_ptr = new wxButton(this, wxID_OK, "Continue");
  ok_button_ptr->SetMinSize(wxSize(MIN_CONTOL_WIDTH, 2 * ok_button_ptr->GetSize().GetHeight()));
  const auto border_flags = wxSizerFlags().Center().Border(wxALL, BORDER_WIDTH);
  grid_ptr->Add(header_text_ptr, border_flags);
  grid_ptr->Add(body_text_ptr, border_flags);
  grid_ptr->Add(steps_text_ptr, border_flags);
  grid_ptr->Add(ok_button_ptr, border_flags);
  wxFlexGridSizer* padder_ptr = new wxFlexGridSizer(1);
  padder_ptr->Add(grid_ptr, border_flags);
  SetSizerAndFit(padder_ptr);
  // Lock down resizing
  SetMinSize(GetSize());
  SetMaxSize(GetSize());
  // Cancel on close, etc
  SetEscapeId(wxID_CANCEL);
}

GuiDialogProgress::GuiDialogProgress(wxWindow* parent)
    : wxProgressDialog("Progress", "...", 100, parent)
{
  constexpr int MIN_PROGRESS_WIDTH = 500;
  SetSize(wxSize(MIN_PROGRESS_WIDTH, GetSize().GetHeight()));
  SetMinSize(GetSize());
  SetMaxSize(GetSize());
}

std::string GetProcessRunningMessage(McProcessRunning process_running, bool can_continue)
{
  const std::string continue_text = (can_continue ? "to continue." : "and restart this utility.");
  switch (process_running) {
  case McProcessRunning::LAUNCHER:
    return "The Minecraft Launcher appears to be running. Please close it " + continue_text;
  case McProcessRunning::GAME:
    return "Minecraft appears to be running. Please close it " + continue_text;
  case McProcessRunning::LAUNCHER_AND_GAME:
    return "The Minecraft Launcher and game both appear to be running. Please close them "
           + continue_text;
  default:
    // We should never actaully display this
    return "Durp! Durp! Durp!";
  }
}

void OpenUrlInBrowser(const char* const url)
{
  wxMimeTypesManager mime_types_manager;
  wxFileType* file_type_ptr = mime_types_manager.GetFileTypeFromExtension("html");
  wxString command = file_type_ptr->GetOpenCommand(url);
  delete file_type_ptr;
  wxExecute(command);
}

std::vector<std::string> GetUniqueProfileNames(const std::vector<ProfileData>& profile_datas)
{
  // TODO Make sure these strings are actually unique
  constexpr std::size_t MAX_NAME_LENGTH = 30;
  std::vector<std::string> profile_names;
  for (const ProfileData& profile_data : profile_datas) {
    const std::string profile_name = profile_data.name_opt.value_or("");
    const std::string name_part =                         //
        (!profile_name.empty()                            //
             ? (profile_name.length() <= MAX_NAME_LENGTH  //
                    ? profile_name                        //
                    : profile_name.substr(0, MAX_NAME_LENGTH) + "...")
             : "<Unnamed Profile>");
    // The "Furnace" is apparently the default when no icon is set
    const std::regex data_icon_regex("^data:");
    const std::string profile_icon = profile_data.icon_opt.value_or("");
    const std::string icon_part =                                  //
        (!profile_icon.empty()                                     //
             ? (!std::regex_search(profile_icon, data_icon_regex)  //
                    ? profile_icon                                 //
                    : "<Custom Icon>")
             : "<No Icon>");
    profile_names.push_back(name_part + " (" + icon_part + ")");
  }
  return profile_names;
}

const wxBitmap& GetTrollfaceIcon16x16()
{
  static wxMemoryInputStream trollface_mem(
      "\211PNG\15\12\32\12\0\0\0\15IHDR\0\0\0\20\0\0\0\20\10\4\0\0\0\265\3727\352\0\0\0\2bKGD\0\377"
      "\207\217\314\277\0\0\0\11pHYs\0\0\13\23\0\0\13\23\1\0\232\234\30\0\0\0\7tIME\7\344\1\31\0043"
      ";\23\31\346Y\0\0\0\242IDAT(\317\205\221\273\15\2A\14\5\347I\33 *\201\230\353\6J!\244\37z "
      "\366\306\224\363\10\354\333\273\3\4\226\354\265\326\343\217l\231\337\322fGo\244\265\1\344x"
      "\313\224\23is\370^\201\33p\5\36\205\264\14\237\200'"
      "p\4\340\0\354\211B\34N\301\230\225\27\306Fi "
      "g\24\36\266\3\23\15:F\5\231\13p."
      "\250\223\231U6\2524\243Yy9\305\362\231\272\313\227\6\226\34\325["
      "\270t\263IK\206\31b5\364\330\244\225\313\16\226\215\367o'H("
      "e\32i\37WX\237K\377\316\375\2\322\300v\243z\321\324\254\0\0\0\0IEND\256B`\202",
      273);
  static wxBitmap trollface_bitmap(wxImage(trollface_mem, wxBITMAP_TYPE_ANY), -1);
  return trollface_bitmap;
}

}  // namespace

}  // namespace tl

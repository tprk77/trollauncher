# meson.build

###########
# Project #
###########

project(
  'trollauncher',
  'cpp',
  version : '0.7.0',
  license : 'MIT',
  default_options : [
    'cpp_std=c++17',
    'buildtype=debugoptimized',
    'warning_level=3',
    'werror=true'
  ]
)

version = meson.project_version()
version_array = version.split('.')
major_version = version_array[0].to_int()
minor_version = version_array[1].to_int()
patch_version = version_array[2].to_int()

###################
# Windows Options #
###################

# Use static linking on Windows
msw_extra_src = []
msw_extra_deps = []
wxwidgets_mods = []
main_link_args = []
if build_machine.system() == 'windows'
  windows = import('windows')
  trollauncher_rc = configure_file(
    input : 'resource/trollauncher.rc.in',
    output : 'trollauncher.rc',
    configuration : {
      'MAJOR_VERSION' : major_version,
      'MINOR_VERSION' : minor_version,
      'PATCH_VERSION' : patch_version
    }
  )
  msw_extra_src = [
    windows.compile_resources(
      trollauncher_rc,
      # Unfortunately, this is hardcoded to MSYS2, and you may need to edit it
      include_directories : include_directories('resource', 'C:/msys64/mingw64/include/wx-3.0')
    )
  ]
  msw_extra_deps = [meson.get_compiler('cpp').find_library('ws2_32')]
  wxwidgets_mods = ['--static']
  main_link_args = ['-static']
endif

################
# Dependencies #
################

fs_dep = meson.get_compiler('cpp').find_library('stdc++fs')

boost_dep = dependency(
  'boost',
  version : '>=1.65.0',
  modules : ['system', 'filesystem', 'program_options'],
  include_type : 'system'
)

libzippp_dep = dependency(
  'libzippp',
  fallback : ['libzippp', 'libzippp_dep'],
  include_type : 'system'
)

nlohmann_json_dep = dependency(
  'nlohmann_json',
  fallback : ['nlohmann_json', 'nlohmann_json_dep'],
  include_type : 'system'
)

date_dep = dependency(
  'date',
  fallback : ['date', 'date_dep'],
  include_type : 'system'
)

wxwidgets_dep = dependency(
  'wxwidgets',
  version : '>=3.0.0',
  modules : wxwidgets_mods,
  include_type : 'system'
)

###############
# Application #
###############

trollauncher_srcs = [
    'trollauncher/cli.cpp',
    'trollauncher/error_codes.cpp',
    'trollauncher/forge_installer.cpp',
    'trollauncher/gui.cpp',
    'trollauncher/java_detector.cpp',
    'trollauncher/keeplist_processor.cpp',
    'trollauncher/launcher_profiles_editor.cpp',
    'trollauncher/main.cpp',
    'trollauncher/modpack_installer.cpp',
    'trollauncher/utils.cpp'
]

trollauncher_deps = [
    fs_dep, boost_dep,
    libzippp_dep,
    nlohmann_json_dep,
    date_dep,
    wxwidgets_dep
]

executable(
  'trollauncher', trollauncher_srcs + msw_extra_src,
  include_directories : [],
  dependencies : trollauncher_deps + msw_extra_deps,
  link_args : main_link_args
)

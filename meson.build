# meson.build

###########
# Project #
###########

project('trollauncher', 'cpp',
  version : '0.1.0',
  license : 'MIT',
  default_options : [
    'cpp_std=c++17',
    'buildtype=debugoptimized',
    'warning_level=3'
    # Apparently, we can't have nice things...
    # 'werror=true'
  ]
)

###################
# Windows Options #
###################

# Use static linking on Windows
wxwidgets_mods = []
main_link_args = []
if build_machine.system() == 'windows'
  wxwidgets_mods = ['--static']
  main_link_args = ['-static']
endif

################
# Dependencies #
################

fs_dep = meson.get_compiler('cpp').find_library('stdc++fs')
boost_dep = dependency('boost', version : '>=1.65.0', modules : ['system', 'filesystem'])
libzippp_dep = dependency('libzippp', fallback : ['libzippp', 'libzippp_dep'])
nlohmann_json_dep = dependency('nlohmann_json', fallback : ['nlohmann_json', 'nlohmann_json_dep'])
# wxwidgets_dep = dependency('wxwidgets', version : '>=3.0.0', modules : wxwidgets_mods)

###############
# Application #
###############

trollauncher_srcs = [
    'trollauncher/main.cpp'
]

executable(
  'trollauncher', trollauncher_srcs,
  include_directories : [],
  dependencies : [fs_dep, boost_dep, libzippp_dep, nlohmann_json_dep],
  link_args : main_link_args
)

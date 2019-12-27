# meson.build

###########
# Project #
###########

project('trollauncher', 'cpp',
  version : '0.1.0',
  license : 'MIT',
  default_options : [
    'cpp_std=c++11',
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

boost_dep = dependency('boost', version : '>=1.65.0', modules : ['system', 'filesystem'])
libzippp_dep = dependency('libzippp', fallback : ['libzippp', 'libzippp_dep'])
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
  dependencies : [boost_dep, libzippp_dep],
  link_args : main_link_args
)

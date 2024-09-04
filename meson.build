project('harmonics', 'cpp')

maya_dep = dependency('maya')
maya_name_suffix = maya_dep.get_variable('name_suffix')
maya_version = maya_dep.get_variable('maya_version')

source_files = [
  'src/lib/harmonicSolver.cpp',
  'src/maya/harmonicsNode.cpp',
  'src/maya/mapData.cpp',
  'src/maya/pluginMain.cpp',
]

outlib = shared_library(
  meson.project_name(),
  source_files,
  install: true,
  install_dir : meson.global_source_root() / 'output_Maya' + maya_version,
  include_directories : include_directories(['src/lib', 'src/maya']),
  dependencies : maya_dep,
  name_suffix : maya_name_suffix,
)

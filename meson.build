project('esextractor', 'c', 'cpp',
  default_options : [ 'buildtype=debug', 'cpp_std=c++11' ],
  version: '0.0.1'
)

_override_options = []
build_system = build_machine.system()
if build_system == 'windows'
  _override_options = ['cpp_std=vc++latest']
endif

pkgconfig = import('pkgconfig')
samples_folder =  join_paths (meson.source_root(), 'samples')

cdata = configuration_data()
cdata.set_quoted('ESE_LICENSE', 'Apache License')
cdata.set_quoted('ESE_SAMPLES_FOLDER', samples_folder)
configure_file(output : 'config.h', configuration : cdata)

subdir('lib')
subdir('tests')

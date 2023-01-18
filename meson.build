project('esextractor', 'c', 'cpp',
  default_options : [ 'buildtype=debug', 'cpp_std=c++17' ],
  version: '0.0.1'
)

_override_options = []
build_system = build_machine.system()
if build_system == 'windows'
  _override_options = ['cpp_std=vc++latest']
endif

pkgconfig = import('pkgconfig')

subdir('lib')
subdir('test')

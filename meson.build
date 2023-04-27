project('esextractor', 'c', 'cpp',
  default_options : [ 'buildtype=debug', 'cpp_std=c++11' ],
  version: '0.0.1'
)
python3 = import('python').find_installation()

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

clang_format = find_program('clang-format', required: false)
# Install clang-format pre-commit hook
if clang_format.found()
  run_command(python3, '-c', 'import shutil; shutil.copy("scripts/git-hooks/pre-commit", ".git/hooks/pre-commit")', check: false)
else
  message('clang-format not found, please install it with your package manager to enable the commit message hook')
endif
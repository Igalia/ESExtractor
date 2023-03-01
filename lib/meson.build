esextractor_sources = files(
  'esextractor.cpp',
  'esenalu.cpp',
)

esextractor_headers = files(
  'esextractor.h',
)

install_headers(esextractor_headers)

es_cpp_args = ['-DBUILDING_ES_EXTRACTOR']

if get_option('default_library') == 'static'
  es_cpp_args += ['-DES_STATIC_COMPILATION']
endif

esextractor = library(
  'esextractor',
  esextractor_sources,
  include_directories: include_directories('.'),
  cpp_args: es_cpp_args,
  install: true,
#  vs_module_defs: 'esextractor.def',
)

libesextractor_dep = declare_dependency(

  sources: 'esextractor.h',
  include_directories: include_directories('.'),
  link_with: esextractor,
)

pkgconfig.generate(esextractor,
  name : 'esextractor',
  description : 'helper to demux H26x elementary stream',
)
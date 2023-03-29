
inc_dirs = include_directories('../')

esextractortestbin = executable(
  'testesextractor',
  files('testbin.cpp', 'testese.cpp'),
  include_directories : [inc_dirs],
  override_options: _override_options,
  dependencies: [libesextractor_dep]
)

esextractortest = executable(
  'testesesuite',
  files('testsuite.cpp', 'testese.cpp'),
  include_directories : [inc_dirs],
  override_options: _override_options,
  dependencies: [libesextractor_dep]
)

h264sample = files(join_paths(samples_folder, 'Sample_10.avc'))
h265sample = files(join_paths(samples_folder, 'Sample_10.hevc'))
ivfsample = files(join_paths(samples_folder, 'clip-a.ivf'))

test('testsuite', esextractortest, suite: ['assert', 'esextractor'])
test('testbin', esextractortestbin, args: ['-f', h264sample, '-o', 'alignment:AU'], suite: ['h264-AU', 'esextractor'])
test('testbin', esextractortestbin, args: ['-f', h264sample, '-o', 'alignment:NAL'], suite: ['h264-NAL', 'esextractor'])
test('testbin', esextractortestbin, args: ['-f', h265sample, '-o', 'alignment:AU'], suite: ['h265-AU', 'esextractor'])
test('testbin', esextractortestbin, args: ['-f', h265sample, '-o', 'alignment:NAL'], suite: ['h265-NAL', 'esextractor'])
test('testbin', esextractortestbin, args: ['-f', ivfsample], suite: ['ivf', 'esextractor'])

source_root = meson.source_root()

esextractortest = executable(
  'testesextractor',
  files('test.cpp'),
  override_options: _override_options,
  dependencies: [libesextractor_dep]
)

h264sample = files(join_paths(source_root, 'samples', 'Sample_10.avc'))
h265sample = files(join_paths(source_root, 'samples', 'Sample_10.hevc'))

test('test', esextractortest, args: [ h264sample], suite: ['h264', 'esextractor'])
test('test', esextractortest, args: [ h265sample], suite: ['h265', 'esextractor'])

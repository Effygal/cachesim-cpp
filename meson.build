project('cachesim-cpp', 'cpp',
  version : '0.1.0',
  default_options : ['cpp_std=c++20']
)

src_files = files(
  'src/lru.cpp',
  'src/fifo.cpp',
  'src/clock.cpp'
)

library('cachesim', src_files,
  install : true
)

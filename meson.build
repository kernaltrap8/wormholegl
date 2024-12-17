project('wormholegl', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++14'])

executable('wormhole',
           'src/wormholegl.cpp',
           install : true,
           link_args : ['-lpulse-simple', '-lpulse'])

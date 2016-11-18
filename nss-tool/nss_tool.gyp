{
  'includes': [
    '../coreconf/config.gypi',
    # this must be included else the -I flag does not get set
    '../cmd/platlibs.gypi'
  ],
  'targets': [
    {
      'target_name': 'nss',
      'type': 'executable',
      'sources': [
        'main.cc',
        'argparse.cc',
        'nss_tool.cc',
        'util.cc'
      ],
      'dependencies': [
        '<(DEPTH)/exports.gyp:dbm_exports',
        '<(DEPTH)/exports.gyp:nss_exports'
      ]
    }
  ],
#  'include_dirs': [
#    '/home/stefan/UNI_Local/MWoS/nss-src-root/dist/Linux4.7_x86_64_cc_glibc_PTH_64_DBG.OBJ/include/nspr'
#    '<(nspr_include_dir)',
#    '<(nspr_include_dir)/../../../public/nss',
#  ],
#  'target_defaults': {
#    'defines': [
#      'NSPR20'
#    ]
#  },
#  'variables': {
#    'nss_dist_dir': '<!(<(python) <(DEPTH)/coreconf/pkg_config.py ../../.. --cflags nspr)'
#    'foo': '<!(export CC=/usr/bin/g++)'
#    'module': 'nss'
#  }
}

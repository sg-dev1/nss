#/* This Source Code Form is subject to the terms of the Mozilla Public
# * License, v. 2.0. If a copy of the MPL was not distributed with this
# * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
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
        'nss_tools.cc',
        'common/argparse.cc',
        'common/nss_common.cc',
        'common/util.cc',
        'db/dbtool.cc',
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

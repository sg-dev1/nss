# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
{
  'includes': [
    '../../coreconf/config.gypi',
    '../common/gtest.gypi',
  ],
  'targets': [
    {
      'target_name': 'nss_tool_gtest',
      'type': 'executable',
      'sources': [
        'dbtool_unittest.cc',
        'nss_tool_gtests.cc',
      ],
      'include_dirs': [
        '<(DEPTH)/nss-tool/common',
        '<(DEPTH)/nss-tool/db',
      ],
      'dependencies': [
        '<(DEPTH)/exports.gyp:nss_exports',
        '<(DEPTH)/gtests/google_test/google_test.gyp:gtest',
        '<(DEPTH)/nss-tool/nss_tool.gyp:nss_tool_static',
      ]
    }
  ],
  'variables': {
    'module': 'nss',
  }
}

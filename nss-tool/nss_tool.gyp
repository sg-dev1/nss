# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
{
  'includes' : [
    '../coreconf/config.gypi',
    #this must be included else the - I flag does not get set
    '../cmd/platlibs.gypi'
  ],
  'targets' : [
    {
      'target_name' : 'nss',
      'type' : 'executable',
      'sources' : [
        'nss_tool.cc',
        'common/argparse.cc',
        'db/dbtool.cc',
       ],
       'dependencies' : [
         '<(DEPTH)/exports.gyp:dbm_exports',
         '<(DEPTH)/exports.gyp:nss_exports'
       ]
    }
  ],
}

{
    'targets' : [
        {
            'target_name' : 'hdfs3_bindings',
            'sources' : [
                'src/addon.cc',
                'src/HDFileSystem.cc',
                'src/HDFile.cc'
            ],
            'xcode_settings': {
                'OTHER_CFLAGS': ['-Wno-unused-parameter', '-Wno-unused-result']
            },
            'cflags' : ['-Wall', '-Wextra', '-Wno-unused-parameter', '-Wno-unused-result'],
            'include_dirs': [
                "<!(node -e \"require('nan')\")"
            ],
            'conditions' : [
                [ 'OS == "linux"', { 'libraries' : [ '-lhdfs3' ], 'cflags' : [ '-g' ] }],
                [ 'OS == "mac"', { 'libraries' : [ '-lhdfs3' ] }
                ]
            ]
        }
    ]
}
{
    'targets' : [
        {
            'target_name' : 'hdfs3_bindings',
            'sources' : [
                'src/addon.cc',
                'src/HDFileSystem.cc',
                'src/HDFile.cc'
            ],
            'cflags' : ['-Wall', '-Wextra', '-Wno-unused-parameter'],
            'cflags!': [ '-fno-exceptions' ],
            'cflags_cc!': [ '-fno-exceptions' ],
            'include_dirs': [
                "<!(node -e \"require('nan')\")"
            ],
            'conditions' : [
                [ 'OS == "linux"', { 'libraries' : [ '-lhdfs3' ], 'cflags' : [ '-g' ] }],
                [ 'OS == "mac"', {
                    'libraries' : [ '-L/usr/local/lib', '-lhdfs3' ],
                    'xcode_settings': {
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                    }
                }]
            ]
        }
    ]
}
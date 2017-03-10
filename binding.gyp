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
            'include_dirs': [
                "<!(node -e \"require('nan')\")"
            ],
            'conditions' : [
                [ 'OS == "linux"', { 'libraries' : [ '-lhdfs3' ], 'cflags' : [ '-g' ] }],
                [ 'OS == "mac"', { 'libraries' : [ '-L/usr/local/lib', '-lhdfs3' ] }]
            ]
        }
    ]
}
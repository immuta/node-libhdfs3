/*
  Copyright (c) 2017 Immuta, Inc.
  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

var libhdfs3 = require('../lib/hdfs');
var fs = new libhdfs3();

var targetDir = '/user/cloudera';    // The target dir should contain files
var kerbTicketCachePath = undefined;                   // Set this to use kerberos

var options = {
    "nameNode": "hdfs://localhost:8020",
    "extra": {
        "dfs.client.use.datanode.hostname": "true"
    }
};

if (kerbTicketCachePath) {
    options.kerbTicketCachePath = kerbTicketCachePath;
    options.extra['hadoop.security.authentication'] = 'kerberos';
}

fs.connect(options, function(err, success) {
    if (err) {
        return console.log('Connection Error :: ', err);
    }

    fs.ls(targetDir, function(lsErr, dirContents) {
        if (lsErr) {
            return console.log('Error listing directory :: ', err);
        }

        for (var i = 0; i < dirContents.length; i++) {
            // Only read a "big" file
            if (dirContents[i].kind === 'FILE' && dirContents[i].size > 1024 * 64) {
                // Read and pipe a file to stdout
                fs.read(dirContents[i].path, function(readErr, stream) {
                    if (readErr) {
                        console.log("File Read Error:", readErr);
                    }
                    stream.on('error', function (err) {
                        console.log("Stream Error:", err);
                    });
                    stream.pipe(process.stdout);
                });
                break;
            }
        };
    });
});
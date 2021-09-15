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

const libhdfs3 = require('../lib/hdfs');
const fs = new libhdfs3();

const targetDir = '/user/cloudera';     // The target dir should contain files
const kerbTicketCachePath = undefined;  // Set this to use kerberos

const options = {
    nameNode: 'hdfs://localhost:8020',
    extra: {
        'dfs.client.use.datanode.hostname': 'true'
    },
    userName: 'immuta'
};

if (kerbTicketCachePath) {
    options.kerbTicketCachePath = kerbTicketCachePath;
    options.extra['hadoop.security.authentication'] = 'kerberos';
}

async function printDirectory(directory) {
    return new Promise((resolve, reject) => {
        fs.ls(directory, async (lsErr, dirContents) => {
            if (lsErr) {
                console.log('Error listing directory :: ', err);
                reject(err);
            }

            for (const file of dirContents) {
                console.log(`${file.path} (${file.kind})`)
                if (file.kind === 'DIRECTORY') {
                    await printDirectory(file.path);
                }
            };
            resolve();
        });
    });
}

fs.connect(options, async (err, success) => {
    if (err) {
        return console.log('Connection Error :: ', err);
    }
    console.log('Successfully Connected!');

    await printDirectory(targetDir);

    fs.ls(targetDir, (lsErr, dirContents) => {
        if (lsErr) {
            return console.log('Error listing directory :: ', err);
        }
        for (const file of dirContents) {
            console.log(`${file.path} (${file.kind})`)
            // Only read a "big" file
            if (file.kind === 'FILE' && file.size > 1024 * 64) {
                // Read and pipe a file to stdout
                fs.read(file.path, (readErr, stream) => {
                    if (readErr) {
                        console.log('File Read Error:', readErr);
                    }
                    stream.on('error', (err) => {
                        console.log('Stream Error:', err);
                    });
                    stream.pipe(process.stdout);
                });
                break;
            }
        };
    });
});
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

'use strict';

var bindings = require('bindings')('hdfs3_bindings');
var Readable = require('stream').Readable;
var NOT_CONNECTED_ERROR = new Error("No active HDFS connection. Must call connect() first.")

var libhdfs3 = function() {
    this.connected = false;
    this.fs = new bindings.FileSystem();
};

libhdfs3.prototype.connect = function(options, cb) {
    var self = this;
    this.fs.connect(options, function(err, data) {
        self.connected = true;
        cb(err, data);
    });
};

libhdfs3.prototype.disconnect = function(cb) {
    var self = this;
    if (!this.connected) {
        return cb(null, true);
    }

    this.fs.disconnect(function(err, data) {
        self.connected = false;
        cb(err, data);
    });
};

libhdfs3.prototype.ls = function(path, cb) {
    if (!this.connected) {
        return cb(NOT_CONNECTED_ERROR);
    }

    this.fs.ls(path, cb);
};

libhdfs3.prototype.info = function(path, cb) {
    if (!this.connected) {
        return cb(NOT_CONNECTED_ERROR);
    }

    this.fs.info(path, cb);
};

libhdfs3.prototype.xattrs = function(path, cb) {
    if (!this.connected) {
        return cb(NOT_CONNECTED_ERROR);
    }

    this.fs.xattrs(path, cb);
};

libhdfs3.prototype.read = function(path, cb) {
    var self = this;

    if (!this.connected) {
        throw new Error("No active HDFS connect. Must call connect() first.");
    }

    var file = new bindings.File(this.fs, path);
    var stream = new Readable({ highWaterMark: 64 * 1024 });
    var eof = false;
    stream._read = function () {
        file.read(function(err, data) {
            if (err) {
                return stream.emit('error', err);
            }
            if (eof) {
                return stream.push(null);
            }
            if (data.bytesRead > 0) {
                stream.push(data.data);
            }
            eof = data.eof;
        });
    };
    cb(undefined, stream);
};

module.exports = libhdfs3;

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

#ifndef NODE_LIBHDFS3_HDFILE
#define NODE_LIBHDFS3_HDFILE

#include <nan.h>
#include <hdfs/hdfs.h>

class HDFile : public Nan::ObjectWrap {
    public:
        static NAN_METHOD(Create);
    private:
        hdfsFile    handle;
        hdfsFS      fileSystem;
        char*       path;

        HDFile();
        static void buffer_delete_callback(char* data, void* vector);

    public:
        static NAN_METHOD(Read);
    protected:
        static void UV_Read(uv_work_t* work_req);
        static void UV_AfterRead(uv_work_t* work_req, int status);
};

struct read_work_data {
    Nan::Callback*      cb;
    HDFile*             file;
    tOffset             bytesRead;
    int                 error;
    char*               buf;
};

#endif
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

#ifndef NODE_LIBHDFS3_HDFILESYSTEM
#define NODE_LIBHDFS3_HDFILESYSTEM

#include <nan.h>
#include <hdfs/hdfs.h>

class HDFileSystem : public Nan::ObjectWrap {
    public:
        static NAN_METHOD(Create);
        hdfsFS fs;

    private:
        HDFileSystem();
        static v8::Local<v8::Object> createFileInfoObject(hdfsFileInfo info);
        static bool valueExists(v8::Local<v8::Object> obj, v8::Local<v8::String> key);
        static hdfsBuilder* builderFromOptions(v8::Local<v8::Value> options);

    public:
        static NAN_METHOD(Connect);
        static NAN_METHOD(ConnectSync);
    protected:
        static void UV_Connect(uv_work_t* work_req);
        static void UV_AfterConnect(uv_work_t* work_req, int status);

    public:
        static NAN_METHOD(Disconnect);
        static NAN_METHOD(DisconnectSync);
    protected:
        static void UV_Disconnect(uv_work_t* work_req);
        static void UV_AfterDisconnect(uv_work_t* work_req, int status);

    public:
        static NAN_METHOD(FileInfo);
        static NAN_METHOD(FileInfoSync);
    protected:
        static void UV_FileInfo(uv_work_t* work_req);
        static void UV_AfterFileInfo(uv_work_t* work_req, int status);

    public:
        static NAN_METHOD(FileXAttrs);
        static NAN_METHOD(FileXAttrsSync);
    protected:
        static void UV_FileXAttrs(uv_work_t* work_req);
        static void UV_AfterFileXAttrs(uv_work_t* work_req, int status);

    public:
        static NAN_METHOD(List);
        static NAN_METHOD(ListSync);
    protected:
        static void UV_List(uv_work_t* work_req);
        static void UV_AfterList(uv_work_t* work_req, int status);

};

struct list_work_data {
    Nan::Callback*  cb;
    HDFileSystem*   fileSystem;
    hdfsFileInfo*   contents;
    char*           targetDir;
    int             entryCount;
};

struct connect_work_data {
    Nan::Callback*  cb;
    HDFileSystem*   fileSystem;
    hdfsBuilder*    bld;
};

struct disconnect_work_data {
    Nan::Callback*  cb;
    HDFileSystem*   fileSystem;
};

struct fileInfo_work_data {
    Nan::Callback*  cb;
    HDFileSystem*   fileSystem;
    char*           path;
    hdfsFileInfo*   info;
};

struct fileXAttrs_work_data {
    Nan::Callback*  cb;
    HDFileSystem*   fileSystem;
    char*           path;
    hdfsXAttr*      xattrs;
    int             attrCount;
};

#endif
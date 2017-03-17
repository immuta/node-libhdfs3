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
#include <errno.h>

#include "addon.h"
#include "HDFileSystem.h"

HDFile::HDFile() {
    DEBUG("HDFile::Constructor");
    fileSystem = NULL;
    handle = NULL;
    path = NULL;
}

NAN_METHOD(HDFile::Create) {
    DEBUG("Building HDFile instance");
    Nan::MaybeLocal<v8::Object> hdFileSystem = Nan::To<v8::Object>(info[0]);

    HDFile*         file = new HDFile();
    HDFileSystem*   fsWrapper = Nan::ObjectWrap::Unwrap<HDFileSystem>(hdFileSystem.ToLocalChecked());
    file->fileSystem = fsWrapper->fs;
    file->path = NewCString(info[1]);

    file->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(HDFile::Read) {
    DEBUG("HDFile::Read");

    v8::Local<v8::Function> cb;
    HDFile*                 self = NODE_FILE();
    uv_work_t*              work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
    read_work_data*         data = (read_work_data *) (calloc(1, sizeof(read_work_data)));

    if (info.Length() != 1) {
        return Nan::ThrowTypeError("HDFile::read(): Requires 1 arguments. ");
    }
    if (!info[0]->IsFunction()) {
        return Nan::ThrowTypeError("Argument 0 must be a Function.");
    }

    cb = v8::Local<v8::Function>::Cast(info[0]);

    data->file = self;
    data->bytesRead = 0;
    data->buf = new char[FILE_READ_SIZE];
    data->cb = new Nan::Callback(cb);

    work_req->data = data;

    uv_queue_work(
        uv_default_loop(),
        work_req,
        UV_Read,
        (uv_after_work_cb)UV_AfterRead);

    self->Ref();
    info.GetReturnValue().Set(Nan::Undefined());
}

void HDFile::UV_Read(uv_work_t* req) {
    DEBUG("HDFile::UV_Read");

    read_work_data* data = (read_work_data *)(req->data);

    // If the file handle doesn't exist or the file isn't open for reading
    // then open it for reading
    if (!data->file->handle || !hdfsFileIsOpenForRead(data->file->handle)) {
        DEBUG("Opening file...");
        data->file->handle = hdfsOpenFile(data->file->fileSystem, data->file->path, O_RDONLY, 0, 0, 0);
        // Check for open error
        if (data->file->handle == NULL) {
            data->error = errno;
            return;
        }
    }
    DEBUG("Reading File...");
    std::vector<char> buf(FILE_READ_SIZE);

    data->bytesRead = hdfsRead(data->file->fileSystem,
                                data->file->handle,
                                &buf[0],
                                FILE_READ_SIZE);
    // Check for read error
    if (data->bytesRead == -1) {
        data->error = errno;
        return;
    }
    std::copy(buf.begin(), buf.end(), data->buf);

    // If we're at the end of the file then we can close it
    if (data->bytesRead < FILE_READ_SIZE) {
        DEBUG("Closing File...");
        hdfsCloseFile(data->file->fileSystem, data->file->handle);
        data->file->handle = NULL;
    }

}

void HDFile::UV_AfterRead(uv_work_t* req, int status) {
    DEBUG("HDFile::UV_AfterRead");
    Nan::HandleScope        scope;
    v8::Local<v8::Value>    info[2];
    v8::Local<v8::Object>   readInfo = Nan::New<v8::Object>();
    read_work_data*         data = (read_work_data *)(req->data);

    if (data->error) {
        info[0] = Nan::ErrnoException(data->error);
        info[1] = Nan::Null();
    } else {
        // Return data which includes bytesRead count, eof, and buffer
        readInfo->Set(V8_STRING("data"), Nan::NewBuffer(data->buf,
                                            data->bytesRead,
                                            buffer_delete_callback,
                                            &data->buf).ToLocalChecked());
        readInfo->Set(V8_STRING("bytesRead"), Nan::New<v8::Number>(data->bytesRead));
        readInfo->Set(V8_STRING("eof"), Nan::New(data->bytesRead != FILE_READ_SIZE));

        info[0] = Nan::Null();
        info[1] = readInfo;
    }

    data->file->Unref();

    data->cb->Call(2, info);

    delete data->cb;
    free(data);
    free(req);
}

void HDFile::buffer_delete_callback(char* data, void* buf) {
    delete[] data;
}
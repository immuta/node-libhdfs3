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

#include "addon.h"

// Initialize the node addon
NAN_MODULE_INIT(InitAddon) {

  v8::Local<v8::FunctionTemplate> fsTemplate = Nan::New<v8::FunctionTemplate>(HDFileSystem::Create);
  fsTemplate->SetClassName(Nan::New("FileSystem").ToLocalChecked());
  fsTemplate->InstanceTemplate()->SetInternalFieldCount(1);

  // File System Functions
  Nan::SetPrototypeMethod(fsTemplate, "connect", HDFileSystem::Connect);
  Nan::SetPrototypeMethod(fsTemplate, "connectSync", HDFileSystem::ConnectSync);
  Nan::SetPrototypeMethod(fsTemplate, "disconnect", HDFileSystem::Disconnect);
  Nan::SetPrototypeMethod(fsTemplate, "disconnectSync", HDFileSystem::DisconnectSync);
  Nan::SetPrototypeMethod(fsTemplate, "info", HDFileSystem::FileInfo);
  Nan::SetPrototypeMethod(fsTemplate, "infoSync", HDFileSystem::FileInfoSync);
  Nan::SetPrototypeMethod(fsTemplate, "ls", HDFileSystem::List);
  Nan::SetPrototypeMethod(fsTemplate, "lsSync", HDFileSystem::ListSync);
  Nan::SetPrototypeMethod(fsTemplate, "xattrs", HDFileSystem::FileXAttrs);
  Nan::SetPrototypeMethod(fsTemplate, "xattrsSync", HDFileSystem::FileXAttrsSync);

  Nan::Set(target,
      Nan::New("FileSystem").ToLocalChecked(), Nan::GetFunction(fsTemplate).ToLocalChecked());

  v8::Local<v8::FunctionTemplate> fileTemplate = Nan::New<v8::FunctionTemplate>(HDFile::Create);
  fileTemplate->SetClassName(Nan::New("File").ToLocalChecked());
  fileTemplate->InstanceTemplate()->SetInternalFieldCount(1);

  // File System Functions
  Nan::SetPrototypeMethod(fileTemplate, "read", HDFile::Read);

  Nan::Set(target,
      Nan::New("File").ToLocalChecked(), Nan::GetFunction(fileTemplate).ToLocalChecked());
}

NODE_MODULE(addon, InitAddon)

char* NewCString(v8::Local<v8::Value> val) {
    Nan::HandleScope scope;

    v8::Local<v8::String> str = Nan::To<v8::String>(val).ToLocalChecked();
    int len = str->Utf8Length() + 1;
    char* buffer = new char[len];
    str->WriteUtf8(buffer, len);
    return buffer;
}
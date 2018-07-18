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

#ifndef NODE_LIBHDFS3_ADDON
#define NODE_LIBHDFS3_ADDON

#include <nan.h>

#include "HDFileSystem.h"
#include "HDFile.h"

char* NewCString(v8::Local<v8::Value> val);

//#define DEBUG(msg);
#define DEBUG(msg) fprintf(stderr, "%s\n", msg);

#define NODE_FS() Nan::ObjectWrap::Unwrap<HDFileSystem>(info.This());
#define NODE_FILE() Nan::ObjectWrap::Unwrap<HDFile>(info.This());
#define V8_STRING(key) Nan::New<v8::String>(key).ToLocalChecked()
#define FILE_READ_SIZE 65536  // 1024 * 64
#endif
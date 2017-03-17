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

HDFileSystem::HDFileSystem() {
    DEBUG("HDFileSystem::Constructor");
    fs = NULL;
}

NAN_METHOD(HDFileSystem::Create) {
    DEBUG("Building new HDFileSystem");
    HDFileSystem* fileSystem = new HDFileSystem();

    fileSystem->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(HDFileSystem::Connect) {
    DEBUG("HDFileSystem::Connect");

    v8::Local<v8::Function> cb;
    HDFileSystem*           self = NODE_FS();
    uv_work_t*              work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
    connect_work_data*         data = (connect_work_data *) (calloc(1, sizeof(connect_work_data)));

    if (info.Length() != 2) {
        return Nan::ThrowTypeError("Invalid number of arguments. Connect requires 2 arguments.");
    }
    if (!info[0]->IsString() && !info[0]->IsObject()) {
        return Nan::ThrowTypeError("Argument 0 must be a String or an Object.");
    }
    if (!info[1]->IsFunction()) {
        return Nan::ThrowTypeError("Argument 1 must be a Function.");
    }

    cb = v8::Local<v8::Function>::Cast(info[1]);

    data->fileSystem = self;
    data->bld = builderFromOptions(info[0]);
    data->cb = new Nan::Callback(cb);

    work_req->data = data;

    uv_queue_work(
        uv_default_loop(),
        work_req,
        UV_Connect,
        (uv_after_work_cb)UV_AfterConnect);

    self->Ref();
    info.GetReturnValue().Set(Nan::Undefined());
}

void HDFileSystem::UV_Connect(uv_work_t* req) {
    DEBUG("HDFileSystem::UV_Connect");

    connect_work_data* data = (connect_work_data *)(req->data);
    data->fileSystem->fs = hdfsBuilderConnect(data->bld);
}

void HDFileSystem::UV_AfterConnect(uv_work_t* req, int status) {
    DEBUG("HDFileSystem::UV_AfterConnect");
    Nan::HandleScope scope;

    connect_work_data*      data = (connect_work_data *)(req->data);

    v8::Local<v8::Value> info[2];
    info[0] = Nan::Null(); // Error would go here if we had one...
    info[1] = Nan::True();

    data->fileSystem->Unref();

    data->cb->Call(2, info);

    delete data->cb;
    free(data);
    free(req);
}

hdfsBuilder* HDFileSystem::builderFromOptions(v8::Local<v8::Value> options) {
    hdfsBuilder* bld = hdfsNewBuilder();
    // Connect using the first param as the "connection string"
    if (options->IsString()) {
        char* connectionString = NewCString(options);
        hdfsBuilderSetNameNode(bld, connectionString);
        delete[] connectionString;
    } else if (options->IsObject()) {
        v8::Local<v8::Object> conf = options->ToObject();
        v8::Local<v8::String> nameNodeKey = V8_STRING("nameNode");
        v8::Local<v8::String> portKey = V8_STRING("port");
        v8::Local<v8::String> userNameKey = V8_STRING("userName");
        v8::Local<v8::String> tokenKey = V8_STRING("token");
        v8::Local<v8::String> ticketPathKey = V8_STRING("kerbTicketCachePath");
        v8::Local<v8::String> extraKey = V8_STRING("extra");
        v8::Local<v8::String> effectiveUserKey = V8_STRING("effectiveUser");

        if (valueExists(conf, nameNodeKey)) {
            char* nn = NewCString(Nan::Get(conf, nameNodeKey).ToLocalChecked());
            hdfsBuilderSetNameNode(bld, nn);
            delete[] nn;
        }
        if (valueExists(conf, portKey)) {
            tPort port = Nan::Get(conf, portKey).ToLocalChecked()->Uint32Value();
            hdfsBuilderSetNameNodePort(bld, port);
        }
        if (valueExists(conf, userNameKey)) {
            char* userName = NewCString(Nan::Get(conf, userNameKey).ToLocalChecked());
            hdfsBuilderSetUserName(bld, userName);
            delete[] userName;
        }
        if (valueExists(conf, effectiveUserKey)) {
            char* effectiveUser = NewCString(Nan::Get(conf, effectiveUserKey).ToLocalChecked());
            hdfsBuilderSetEffectiveUser(bld, effectiveUser);
            delete[] effectiveUser;
        }
        if (valueExists(conf, tokenKey)) {
            char* token = NewCString(Nan::Get(conf, tokenKey).ToLocalChecked());
            hdfsBuilderSetToken(bld, token);
            delete[] token;
        }
        if (valueExists(conf, ticketPathKey)) {
            char* ticketPath = NewCString(Nan::Get(conf, ticketPathKey).ToLocalChecked());
            hdfsBuilderSetKerbTicketCachePath(bld, ticketPath);
            delete[] ticketPath;
        }
        if (valueExists(conf, extraKey)) {
            v8::Local<v8::Object> additionalConfig = Nan::Get(conf, extraKey).ToLocalChecked()->ToObject();
            v8::Local<v8::Array> props = Nan::GetOwnPropertyNames(additionalConfig).ToLocalChecked();
            int propertyCount = props->Length();
            for (int i = 0; i < propertyCount; i++) {
                v8::Local<v8::String> propKey = Nan::To<v8::String>(Nan::Get(props, i).ToLocalChecked()).ToLocalChecked();
                if (hdfsBuilderConfSetStr(bld, NewCString(propKey),
                    NewCString(Nan::Get(additionalConfig, propKey).ToLocalChecked())) != 0) {
                    hdfsFreeBuilder(bld);
                    Nan::ThrowTypeError("Error setting HDFS config property.");
                    return NULL;
                }
            }
        }
    } else {
        hdfsFreeBuilder(bld);
        Nan::ThrowTypeError("Invalid argument type.");
        return NULL;
    }
    return bld;
}

NAN_METHOD(HDFileSystem::List) {
    DEBUG("HDFileSystem::List");

    v8::Local<v8::Function> cb;
    HDFileSystem*           self = NODE_FS();
    uv_work_t*              work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
    list_work_data*         data = (list_work_data *) (calloc(1, sizeof(list_work_data)));

    // hdfs.ls("<path>", function cb() {})
    if (info.Length() != 2) {
        return Nan::ThrowTypeError("ODBCConnection::ls(): Requires 2 arguments. ");
    }
    if (!info[0]->IsString()) {
        return Nan::ThrowTypeError("Argument 0 must be a String.");
    }
    if (!info[1]->IsFunction()) {
        return Nan::ThrowTypeError("Argument 1 must be a Function.");
    }

    cb = v8::Local<v8::Function>::Cast(info[1]);

    data->fileSystem = self;
    data->targetDir = NewCString(info[0]);
    data->entryCount = 0;
    data->cb = new Nan::Callback(cb);

    work_req->data = data;

    uv_queue_work(
        uv_default_loop(),
        work_req,
        UV_List,
        (uv_after_work_cb)UV_AfterList);

    self->Ref();
    info.GetReturnValue().Set(Nan::Undefined());
}

void HDFileSystem::UV_List(uv_work_t* req) {
    DEBUG("HDFileSystem::UV_List");

    list_work_data* data = (list_work_data *)(req->data);
    data->contents = hdfsListDirectory(data->fileSystem->fs, data->targetDir, &data->entryCount);
}

void HDFileSystem::UV_AfterList(uv_work_t* req, int status) {
    DEBUG("HDFileSystem::UV_AfterList");
    Nan::HandleScope scope;

    list_work_data*         data = (list_work_data *)(req->data);
    v8::Local<v8::Array>    directoryContents = Nan::New<v8::Array>();

    if (data->contents) {
        for (int i = 0; i < data->entryCount; i++) {
            directoryContents->Set(i, createFileInfoObject(data->contents[i]));
        }
        hdfsFreeFileInfo(data->contents, data->entryCount);
    }

    v8::Local<v8::Value> info[2];
    info[0] = Nan::Null(); // Error would go here if we had one...
    info[1] = directoryContents;

    data->fileSystem->Unref();

    data->cb->Call(2, info);

    delete data->cb;
    delete[] data->targetDir;
    free(data);
    free(req);
}

NAN_METHOD(HDFileSystem::FileInfo) {
    DEBUG("HDFileSystem::FileInfo");

    v8::Local<v8::Function> cb;
    HDFileSystem*           self = NODE_FS();
    uv_work_t*              work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
    fileInfo_work_data*         data = (fileInfo_work_data *) (calloc(1, sizeof(fileInfo_work_data)));

    if (info.Length() != 2) {
        return Nan::ThrowTypeError("Invalid number of arguments. FileInfo requires 2 arguments.");
    }
    if (!info[0]->IsString()) {
        return Nan::ThrowTypeError("Argument 0 must be a String.");
    }
    if (!info[1]->IsFunction()) {
        return Nan::ThrowTypeError("Argument 1 must be a Function.");
    }

    cb = v8::Local<v8::Function>::Cast(info[1]);

    data->fileSystem = self;
    data->path = NewCString(info[0]);
    data->cb = new Nan::Callback(cb);

    work_req->data = data;

    uv_queue_work(
        uv_default_loop(),
        work_req,
        UV_FileInfo,
        (uv_after_work_cb)UV_AfterFileInfo);

    self->Ref();
    info.GetReturnValue().Set(Nan::Undefined());
}

void HDFileSystem::UV_FileInfo(uv_work_t* req) {
    DEBUG("HDFileSystem::UV_FileInfo");

    fileInfo_work_data* data = (fileInfo_work_data *)(req->data);
    data->info = hdfsGetPathInfo(data->fileSystem->fs, data->path);
}

void HDFileSystem::UV_AfterFileInfo(uv_work_t* req, int status) {
    DEBUG("HDFileSystem::UV_AfterFileInfo");
    Nan::HandleScope scope;

    fileInfo_work_data*         data = (fileInfo_work_data *)(req->data);

    v8::Local<v8::Value> info[2];
    info[0] = Nan::Null(); // Error would go here if we had one...

    if (data->info) {
        info[1] = createFileInfoObject(*(data->info));
        hdfsFreeFileInfo(data->info, 1);
    } else {
        info[1] = Nan::Null();
    }

    data->fileSystem->Unref();

    data->cb->Call(2, info);

    delete data->cb;
    delete[] data->path;
    free(data);
    free(req);
}

NAN_METHOD(HDFileSystem::FileXAttrs) {
    DEBUG("HDFileSystem::FileXAttrs");

    v8::Local<v8::Function> cb;
    HDFileSystem*           self = NODE_FS();
    uv_work_t*              work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
    fileXAttrs_work_data*   data = (fileXAttrs_work_data *) (calloc(1, sizeof(fileXAttrs_work_data)));

    if (info.Length() != 2) {
        return Nan::ThrowTypeError("Invalid number of arguments. FileXAttrs requires 2 arguments.");
    }
    if (!info[0]->IsString()) {
        return Nan::ThrowTypeError("Argument 0 must be a String.");
    }
    if (!info[1]->IsFunction()) {
        return Nan::ThrowTypeError("Argument 1 must be a Function.");
    }

    cb = v8::Local<v8::Function>::Cast(info[1]);

    data->fileSystem = self;
    data->attrCount = 0;
    data->path = NewCString(info[0]);
    data->cb = new Nan::Callback(cb);

    work_req->data = data;

    uv_queue_work(
        uv_default_loop(),
        work_req,
        UV_FileXAttrs,
        (uv_after_work_cb)UV_AfterFileXAttrs);

    self->Ref();
    info.GetReturnValue().Set(Nan::Undefined());
}

void HDFileSystem::UV_FileXAttrs(uv_work_t* req) {
    DEBUG("HDFileSystem::UV_FileXAttrs");

    fileXAttrs_work_data* data = (fileXAttrs_work_data *)(req->data);
    data->xattrs = hdfsListXAttrs(data->fileSystem->fs, data->path, &data->attrCount);
}

void HDFileSystem::UV_AfterFileXAttrs(uv_work_t* req, int status) {
    DEBUG("HDFileSystem::UV_AfterFileXAttrs");
    Nan::HandleScope scope;

    fileXAttrs_work_data*   data = (fileXAttrs_work_data *)(req->data);
    v8::Local<v8::Object>   xattrs = Nan::New<v8::Object>();
    v8::Local<v8::Value>    info[2];

    info[0] = Nan::Null(); // Error would go here if we had one...

    if (data->attrCount > 0) {
        for (int i = 0; i < data->attrCount; i++) {
            xattrs->Set(V8_STRING(data->xattrs[i].name), V8_STRING(data->xattrs[i].value));
        }
        info[1] = xattrs;
        hdfsFreeXAttrs(data->xattrs, data->attrCount);
    } else {
        info[1] = Nan::Null();
    }

    data->fileSystem->Unref();

    data->cb->Call(2, info);

    delete data->cb;
    delete[] data->path;
    free(data);
    free(req);
}

NAN_METHOD(HDFileSystem::Disconnect) {
    DEBUG("HDFileSystem::Disconnect");

    v8::Local<v8::Function> cb;
    HDFileSystem*           self = NODE_FS();
    uv_work_t*              work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
    disconnect_work_data*   data = (disconnect_work_data *) (calloc(1, sizeof(disconnect_work_data)));

    if (info.Length() != 1) {
        return Nan::ThrowTypeError("Invalid number of arguments. Disconnect requires a single callback argument.");
    }
    if (!info[0]->IsFunction()) {
        return Nan::ThrowTypeError("Argument 0 must be a Function.");
    }

    cb = v8::Local<v8::Function>::Cast(info[0]);

    data->fileSystem = self;
    data->cb = new Nan::Callback(cb);

    work_req->data = data;

    uv_queue_work(
        uv_default_loop(),
        work_req,
        UV_Disconnect,
        (uv_after_work_cb)UV_AfterDisconnect);

    self->Ref();
    info.GetReturnValue().Set(Nan::Undefined());
}

void HDFileSystem::UV_Disconnect(uv_work_t* req) {
    DEBUG("HDFileSystem::UV_Disconnect");

    disconnect_work_data* data = (disconnect_work_data *)(req->data);
    hdfsDisconnect(data->fileSystem->fs);
}

void HDFileSystem::UV_AfterDisconnect(uv_work_t* req, int status) {
    DEBUG("HDFileSystem::UV_AfterDisconnect");
    Nan::HandleScope scope;

    disconnect_work_data*   data = (disconnect_work_data *)(req->data);
    v8::Local<v8::Value>    info[2];

    info[0] = Nan::Null(); // Error would go here if we had one...
    info[1] = Nan::True();

    data->fileSystem->Unref();

    data->cb->Call(2, info);

    delete data->cb;
    free(data);
    free(req);
}

v8::Local<v8::Object> HDFileSystem::createFileInfoObject(hdfsFileInfo info) {
    v8::Local<v8::Object> fileInfo = Nan::New<v8::Object>();
    fileInfo->Set(V8_STRING("path"), V8_STRING(info.mName));
    fileInfo->Set(V8_STRING("size"), Nan::New<v8::Number>(info.mSize));
    fileInfo->Set(V8_STRING("blockSize"), Nan::New<v8::Number>(info.mBlockSize));
    // Convert timestamps to ms since epoch
    fileInfo->Set(V8_STRING("lastModified"), Nan::New<v8::Number>(static_cast<long int> (info.mLastMod) * 1000));
    fileInfo->Set(V8_STRING("lastAccess"), Nan::New<v8::Number>(static_cast<long int> (info.mLastAccess) * 1000));
    fileInfo->Set(V8_STRING("replication"), Nan::New(info.mReplication));
    fileInfo->Set(V8_STRING("kind"), V8_STRING(kObjectKindFile == info.mKind ? "FILE" : "DIRECTORY"));
    fileInfo->Set(V8_STRING("owner"), V8_STRING(info.mOwner));
    fileInfo->Set(V8_STRING("group"), V8_STRING(info.mGroup));
    fileInfo->Set(V8_STRING("permissions"), Nan::New(info.mPermissions));
    return fileInfo;
}

bool HDFileSystem::valueExists(v8::Local<v8::Object> obj, v8::Local<v8::String> key) {
    Nan::Maybe<bool> val = Nan::Has(obj, key);
    return val.IsJust() && val.FromJust();
}
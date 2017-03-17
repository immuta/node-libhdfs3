node-libhdfs3
---------

An asynchronous interface for node.js to HDFS via the libhdfs3 library.

requirements
------------

* Immuta version of [libhdfs3](https://github.com/immuta/incubator-hawq) installed

install
-------

In order for node-libhdfs3 to build the libhdfs3 library must be installed. The
easiest way to do that is:

### git

```bash
git clone git@github.com:immuta/incubator-hawq.git
cd incubator-hawq/depends/libhdfs3/osx/
./install_homebrew_pkg.sh
```

After insuring that all requirements are installed you can install node-libhdfs3 via
one of the two following options:

### git

```bash
git clone git://github.com/immuta/node-libhdfs3.git
cd node-libhdfs3
npm install
```
### npm

```bash
npm install immuta/node-libhdfs3
```

quick example
-------------

```javascript
var libhdfs3 = require('./lib/hdfs');
var fs = new libhdfs3();

var options = {
    "nameNode": "hdfs://192.168.99.100:8020",
    "extra": {
        "dfs.client.use.datanode.hostname": "true"
    }
};

fs.connect(options, function(err, success) {
    fs.ls('/user/cloudera/hdfs-example/logs', function(err, dirContents) {
        if (err) {
            console.log('Error: ', err);
            return;
        }
        console.log(dirContents);
    });
});
```

api
---

### libhdfs3

Basic functionality is provided via the `libhdfs3` class. You can get an
instance via:

```javascript
var libhdfs3 = require('./lib/hdfs');
var fs = new libhdfs3();
```

#### .connect(connectionString, callback)

Open a connection to an HDFS cluster using the supplied connection string.

* **connectionString** - The HDFS connection string for the name node including:
    - protocol
    - hostname
    - port
* **callback** - `callback (err, success)`

```javascript
var libhdfs3 = require('./lib/hdfs');
var fs = new libhdfs3();

fs.connect('hdfs://192.168.99.100:8020', function(err, success) {
    if (err) {
        return console.log(err);
    }

    //we now have an open connection to hdfs
});
```
#### .connect(options, callback)

Open a connection to an HDFS cluster using the supplied configuration options. Supported
options include kerberos authentication, effective user setting (impersonation), and
any additional HDFS config settings.

* **options** - The configuration options to use when establishing the connection
* **callback** - `callback (err, success)`

```javascript
var options = {
    "nameNode": "hdfs://192.168.99.100", // <protocol>://<hostname or ip>:<port>
    "kerbTicketCachePath": "",  // Optional kerberos ticket cache path
    "port": 8020,  // Optional namenode port if not included with nameNode property
    "userName": "", // Optional username to use when connecting
    "effectiveUser": "", // Optional effective user to execute actions as
    "extra": {
        // Extra can contain any additional configuration paramaters that will be
        // set in the config object prior to connecting
        "dfs.client.use.datanode.hostname": "true"
    }
};
fs.connect(options, function(err, success) {
    if (err) {
        return console.log(err);
    }

    //we now have an open connection to the database
});
```

#### .disconnect(callback)

Closes the connection to HDFS if there is an active connection. Once the connection
is closed, all additional operations against the libhdfs3 instance will fail until
a new connection is opened.

* **callback** - `callback (err, success)`

```javascript
var libhdfs3 = require('./lib/hdfs');
var fs = new libhdfs3();

fs.connect('hdfs://192.168.99.100:8020', function(err, success) {
    if (err) {
        return console.log(err);
    }
    // we now have an open connection to hdfs
    fs.disconnect(function(disconnectErr, disconnected) {
        if (disconnectErr) {
            return console.log(disconnectErr)
        }
        // hdfs connection closed
    });
});
```

#### .ls(path, callback)

Perform an ls against the target directory and return an array of file info objects.
File objects have the following structure:

```javascript
{
    path: '/path/to/file',
    size: 5610221,
    blockSize: 134217728,
    lastModified: 1489083945000,    //ms since epoch
    lastAccess: 1489083945000,      //ms since epoch
    replication: 1,
    kind: 'FILE',                   // 'FILE' or 'DIRECTORY'
    owner: 'root',
    group: 'cloudera',
    permissions: 420
}
```

* **path** - The path to execute the ls against.
* **callback** - `callback (err, directoryContents)`

```javascript
var libhdfs3 = require('./lib/hdfs');
var fs = new libhdfs3();

fs.connect('hdfs://192.168.99.100:8020', function(err, success) {
    if (err) {
        return console.log(err);
    }
    fs.ls('/hdfs/path/to/dir', function(lsErr, dirContents) {
        if (lsErr) {
            return console.log(lsErr);
        }
        dirContents.forEach(function(entry) {
            console.log(entry);
        });
    });
    ...
});
```

#### .info(path, callback)

Gets the file info object for the file/directory at the specified path. The structure
is the same as the one returned by ls.

* **path** - The path to get info on.
* **callback** - `callback (err, info)`

```javascript
var libhdfs3 = require('./lib/hdfs');
var fs = new libhdfs3();

fs.connect('hdfs://192.168.99.100:8020', function(err, success) {
    if (err) {
        return console.log(err);
    }
    fs.info('/hdfs/path/to/file', function(infoErr, info) {
        if (infoErr) {
            return console.log(infoErr);
        }
        console.log(info);
    });
    ...
});
```

#### .xattrs(path, callback)

Gets the xattrs associated with the file at the specified path. Xattrs are returned
as key/value pairs in an object. If the file has no xattrs associated with it, `null`
is returned.

* **path** - The path to the file to retrieve the xattrs for.
* **callback** - `callback (err, info)`

```javascript
var libhdfs3 = require('./lib/hdfs');
var fs = new libhdfs3();

fs.connect('hdfs://192.168.99.100:8020', function(err, success) {
    if (err) {
        return console.log(err);
    }
    fs.xattrs('/hdfs/path/to/file', function(xattrsErr, attrs) {
        if (xattrsErr) {
            return console.log(xattrsErr);
        }
        console.log(attrs);
    });
    ...
});
```

#### .read(path, callback)

Opens the file at the specified path and creates a readable stream. At the moment
files are read in 64kb chunks.

* **path** - The path of the file to read.
* **callback** - `callback (err, stream)`

```javascript
var libhdfs3 = require('./lib/hdfs');
var fs = new libhdfs3();

fs.connect('hdfs://192.168.99.100:8020', function(err, success) {
    if (err) {
        return console.log(err);
    }
    fs.read('/hdfs/path/to/file', function(readErr, stream) {
        if (readErr) {
            return console.log(readErr);
        }
        stream.pipe(process.stdout);
    });
    ...
});
```

TODO
-------

- testing
- additional error handling
- testing
- additional options for read (block size/seek/etc)
- additional libhdfs API support
- testing
- publish to npm

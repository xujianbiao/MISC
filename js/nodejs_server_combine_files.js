/**
 * Created by arvin on 2016/6/11.
 */

var url = require('url');
var http = require('http');
var fs = require('fs');
var path = require('path');

function parseURL(root, url) {
    var filepathList = [];
    if (url.indexOf('??') === -1) {
        url = url.replace('/', '/??');
        console.log(url);
    }

    var urlGroup = url.split('??');

    // urlGroup.forEach(function (data) {
    //     console.log(data);
    // })

    var commonPath = urlGroup[0];
    var individualPathGrounp = urlGroup[1].split(',');

    individualPathGrounp.forEach(function (fsPath) {
        filepathList.push(path.join(root, commonPath, fsPath));
    });

    // filepathList.forEach(function (path) {
    //     console.log(path);
    // })

    return filepathList;
}

function combineFile(filepathList, callback) {
    var buf = [];
    (function next(i, length) {
        if (i < length) {
            fs.readFile(filepathList[i], function (err, data) {
                if (err) {
                    callback(err);
                }
                else {
                    buf.push(data);
                    buf.push(new Buffer('\n\n/**--------------------**/\n\n'));
                    next(i + 1, length);
                }
            });
        } else {
            callback(null, Buffer.concat(buf));
        }
    })(0, filepathList.length);

}

function validateFile(filepathList, callback) {
    (function next(i, length) {
        if (i < length) {
            fs.stat(filepathList[i], function (err, stats) {
                if (err) {
                    callback(err);
                } else if (!stats.isFile()) {
                    callback(new Error('not a file'));
                } else {
                    next(i + 1, length);
                }
            })
        } else {
            callback(null, filepathList);
        }
    })(0, filepathList.length);
}

function outputFile(filepathList, writer) {
    (function next(i, length) {
        if (i < length) {
            var reader = fs.createReadStream(filepathList[i]);
            reader.pipe(writer, {end: false});
            reader.on('end', function () {
                next(i + 1, length);
            })
        } else {
            writer.end();
        }
    })(0, filepathList.length);
}

function loadConfig(configPath) {
    var config;
    try {
        config = JSON.parse(fs.readFileSync(configPath, 'utf-8'));
    } catch (e) {
        console.log(e);
    }
    return {
        'root': config ? config.root : './',
        'port': config ? config.port : 80
    }
}

function main(argc) {
    var config = loadConfig('./config.json');
    http.createServer(function (request, response) {

        var filePathList = parseURL(config.root, request.url);

        validateFile(filePathList, function (err, filelistPath) {
            if (err) {
                response.writeHead(404);
                response.end(err.message);
            } else {
                response.writeHead(200,
                    {'Content-Type': 'text/plain'});
                outputFile(filelistPath, response);
            }
        })
    }).listen(config.port);
}

main(process.argv);
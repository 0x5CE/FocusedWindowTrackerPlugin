var addon = require('bindings')('hello');
var fs = require('fs'),
    PNG = require('pngjs').PNG;

var focuseedImageAndDetail = addon.getFocusedImageAndDetail();

console.log("Window path name: " + focuseedImageAndDetail.filename);

width = focuseedImageAndDetail.snapshotWidth;
height = focuseedImageAndDetail.snapshotHeight;

var png = new PNG({
    width,
    height,
    filterType: -1
});

// saving snapshot
png.data = focuseedImageAndDetail.snapshot
png.pack().pipe(fs.createWriteStream("sample.png"));

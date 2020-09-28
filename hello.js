var addon = require('bindings')('wintracker');
var fs = require('fs'),
    PNG = require('pngjs').PNG;

var focuseedImageAndDetail = addon.getFocusedImageAndDetail();

console.log("Window path name: " + focuseedImageAndDetail.filename);

width = focuseedImageAndDetail.snapshotWidth;
height = focuseedImageAndDetail.snapshotHeight;

var snapshotPng = new PNG({
    width,
    height,
    filterType: -1
});

// saving snapshot
console.log("Saving window snapshot...");
snapshotPng .data = focuseedImageAndDetail.snapshot
snapshotPng .pack().pipe(fs.createWriteStream("snapshot.png"));


// icon
width = focuseedImageAndDetail.iconWidth;
height = focuseedImageAndDetail.iconHeight;

var snapshotPng = new PNG({
    width,
    height,
    filterType: -1
});

// saving snapshot
console.log("Saving window icon...");
snapshotPng .data = focuseedImageAndDetail.icon
snapshotPng .pack().pipe(fs.createWriteStream("icon.png"));

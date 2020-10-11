var addon = require('bindings')('wintracker');
var fs = require('fs'),
    PNG = require('pngjs').PNG;

// invert Red and Blue positions in RGBA
function invertRandB(imgBuffer, width, height)
{
    for (var i = 0; i < height * width * 4; i += 4) {
        var tmp = imgBuffer[i];
        imgBuffer[i] = imgBuffer[i + 2];
        imgBuffer[i + 2] = tmp;
    }
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}
sleep(3000).then(() =>
{
    var focuseedImageAndDetail = addon.getFocusedImageAndDetail();

    console.log("Window path name: " + focuseedImageAndDetail.filename);

    width = focuseedImageAndDetail.snapshotWidth;
    height = focuseedImageAndDetail.snapshotHeight;

    var snapshotPng = new PNG({
        width,
        height,
        filterType: -1
    });

    // color correction for saving
    invertRandB(focuseedImageAndDetail.snapshot, width, height)

    // saving snapshot
    console.log("Saving window snapshot...");
    snapshotPng.data = focuseedImageAndDetail.snapshot
    snapshotPng.pack().pipe(fs.createWriteStream("snapshot.png"));


    // icon
    width = focuseedImageAndDetail.iconWidth;
    height = focuseedImageAndDetail.iconHeight;

    var snapshotPng = new PNG({
        width,
        height,
        filterType: -1
    });

    invertRandB(focuseedImageAndDetail.icon, width, height)

    // saving snapshot
    console.log("Saving window icon...");
    snapshotPng.data = focuseedImageAndDetail.icon
    snapshotPng.pack().pipe(fs.createWriteStream("icon.png"));

    console.log("User idle time: " + focuseedImageAndDetail.userIdleTime);

    if (focuseedImageAndDetail.browserURL)
        console.log('Browser URL: ' + focuseedImageAndDetail.browserURL)
    if (focuseedImageAndDetail.browserTitle)
        console.log('Browser Title: ' + focuseedImageAndDetail.browserTitle)

});


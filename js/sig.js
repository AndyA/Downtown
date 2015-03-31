"use strict";

var MM = require('../lib/MovieMaker.js');

function loadJSONL(file, done, error) {
  var LineByLineReader = require('line-by-line');
  var frameData = [];
  var firstFrame = null;

  error = error ||
  function(err) {
    console.log("Error: ", err);
  }

  var lr = new LineByLineReader(file);
  lr.on('error', function(err) {
    error(err);
  });

  lr.on('line', function(line) {
    var data = JSON.parse(line);

    // Renumber frames to start at zero...
    // ...which they probably do anyway
    var frame = data.frame;
    if (firstFrame === null) firstFrame = frame;
    frame -= firstFrame;

    frameData[frame] = data.planes;
  });

  lr.on('end', function() {
    done(frameData);
  });
}

var WIDTH = 1920;
var HEIGHT = 1080;

function scaleBox(bbox, scale, xc, yc) {
  var nbbox = {
    w: bbox.w * scale,
    h: bbox.h * scale
  };

  nbbox.x = bbox.x + (bbox.w - nbbox.w) * xc;
  nbbox.y = bbox.y + (bbox.h - nbbox.h) * yc;
  return nbbox;
}

function dp(x, n) {
  return Math.floor(x * n) / n;
}

function historyTrace(ctx, data, row, bbox, parms, depth, factor) {
  var opacity = parms.opacity * factor;
  var lineWidth = parms.lw * factor;

  if (row > 0 && depth < parms.depth && opacity >= 1 / 255 && lineWidth >= 0.05) {
    var nbbox = scaleBox(bbox, parms.zoom, parms.xc, parms.yc);
    historyTrace(ctx, data, row - 1, nbbox, parms, depth + 1, factor * parms.zoom);
  }

  var rowData = data[row];
  if (!rowData) return;

  for (var pl = 0; pl < rowData.length; pl++) {
    var plane = rowData[pl];

    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = 'rgba(120, 255, 180, ' + opacity + ')';

    ctx.beginPath();

    for (var i = 0; i < plane.length; i++) {
      var datum = plane[i];
      var px = i * bbox.w / plane.length + bbox.x;
      var py = bbox.h - datum * parms.scale * factor + bbox.y;
      if (i == 0) ctx.moveTo(px, py);
      else ctx.lineTo(px, py);
    }

    ctx.stroke();
  }

}

function drawTrace(ctx, data, row) {
  var HMARGIN = 40;
  var VMARGIN = 22;

  var bbox = {
    x: HMARGIN,
    y: VMARGIN,
    w: (WIDTH - HMARGIN * 2),
    h: (HEIGHT - VMARGIN * 2)
  };

  var parms = {
    zoom: 0.975,
    scale: bbox.h / 100,
    opacity: 0.7,
    lw: 2.5,
    xc: 0.5,
    yc: 0.2,
    depth: 100
  };

  ctx.save();

  historyTrace(ctx, data, row, bbox, parms, 0, 1);
  ctx.restore();
}

function usage() {
  var me = process.argv[0] + " " + process.argv[1];
  console.log("Usage: " + me + " <data.dat> <output.mjpeg>");
  process.exit(1);
}

function secondsToFrames(s) {
  return s * 25
}

function makeSigMovie(dataFile, mjpegFile, opts) {
  loadJSONL(dataFile, function(frameData) {
    console.log("Loaded frame data");

    var movie = new MM.SequenceClip();
    var frames = frameData.length;
    if (opts.t) {
      var frameLimit = secondsToFrames(opts.t);
      if (frameLimit < frames) frames = frameLimit;
      console.log("Stopping after " + frames + " frames");
    }

    movie.append(new MM.Clip(function(ctx, framenum) {
      drawTrace(ctx, frameData, framenum);
    },
    frames));

    new MM.MovieMaker(mjpegFile, WIDTH, HEIGHT, movie).render();
  });
}

var argv = require('minimist')(process.argv.slice(2), {
  string: ['t'],
  alias: {
    t: 'time'
  }
});
//console.log(argv);
var args = argv._;
if (args.length != 2) usage();

makeSigMovie(args[0], args[1], argv);

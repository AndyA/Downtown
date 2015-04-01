"use strict";

var MM = require('../lib/MovieMaker.js');
var DSF = require('../lib/Downtown/StandardFunctions.js');
var BitBox = require('../lib/Downtown/BitBox.js');

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

function splitString(str, chunk) {
  var out = [];
  while (str.length) {
    out.push(str.substr(0, chunk));
    str = str.substr(chunk);
  }
  return out;
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

  function plotLine(series) {
    ctx.beginPath();

    var started = 0;
    for (var i = parms.skip; i < series.length; i++) {
      var datum = series[i];
      var norm = parms.func(i);
      var px = i * bbox.w / series.length + bbox.x;
      var py = bbox.h - datum / norm * parms.scale * factor + bbox.y;
      if (!started++) ctx.moveTo(px, py);
      else ctx.lineTo(px, py);
    }

    ctx.stroke();
  }

  for (var pl = 0; pl < rowData.length; pl++) {
    var plane = rowData[pl].slice(parms.skip);

    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';

    var smoothed = DSF.smooth(plane);

    ctx.lineWidth = lineWidth / 2;
    ctx.strokeStyle = 'rgba(200, 80, 100, ' + opacity + ')';

    plotLine(smoothed);

    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = 'rgba(120, 255, 180, ' + opacity + ')';

    plotLine(plane);

  }

}

function drawTrace(ctx, data, row, bitbox) {
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
    scale: bbox.h / 10,
    opacity: 0.7,
    lw: 4,
    xc: 0.5,
    yc: 0.2,
    depth: 100,
    skip: 6,
    func: DSF.normalise
  };

  ctx.save();
  historyTrace(ctx, data, row, bbox, parms, 0, 1);

  var plane = data[row][0].slice(parms.skip);
  var sig = DSF.signature(plane);
  var bs = 256;
  bitbox.render(ctx, sig, (WIDTH - bs) * 0.5, (HEIGHT - bs) * 0.1, bs, bs);

  if (0) {
    var hmh = 32;
    var hmw = hmh * 16;
    bitbox.renderHeatChip(ctx, (WIDTH - hmw) * 0.5, (HEIGHT - hmh) * 0.5, hmw, hmh);
  }

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
    if (opts.s) {
      var s = secondsToFrames(opts.s);
      console.log("Seeking " + s + " frames");
      frameData = frameData.slice(s);
    }

    if (opts.t) {
      var t = secondsToFrames(opts.t);
      console.log("Stopping after " + t + " frames");
      frameData = frameData.slice(0, t);
    }

    var frames = frameData.length;
    console.log(frames + " frames to process");

    var bitbox = new BitBox(DSF.signatureBits(), 0.95);

    movie.append(new MM.Clip(function(ctx, framenum) {
      drawTrace(ctx, frameData, framenum, bitbox);
    },
    frames));

    new MM.MovieMaker(mjpegFile, WIDTH, HEIGHT, movie).render();
  });
}

var argv = require('minimist')(process.argv.slice(2), {
  string: ['t', 'ss']
});
var args = argv._;
if (args.length != 2) usage();

makeSigMovie(args[0], args[1], argv);

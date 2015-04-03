"use strict";

var MM = require('../lib/MovieMaker.js');
var DSF = require('../lib/Downtown/StandardFunctions.js');

var WIDTH = 1920;
var HEIGHT = 1080;

function itArray(ar) {
  return function() {
    if (ar.length) return ar.shift();
    return null;
  }
}

function itLoop(it) {
  var stash = [];
  var loop = null;;
  var pos = 0;

  return function() {
    if (loop) {
      if (pos == loop.length) pos = 0;
      return loop[pos++];
    }

    var next = it();
    if (null === next) {
      loop = stash;
      return loop[pos++];
    }
    stash.push(next);
    return next;
  }
}

var BarsClip = MM.util.defineClass(function(cols, widths, frames, params) {
  this.cols = cols;
  this.widths = widths;
  this.frames = frames;
  var defaults = {
    phase: 0,
    angle: 0,
  };
  this.bindParameters(params, defaults);
},
new MM.ClipBase(), {
  render: function(ctx, framenum) {
    var w = ctx.canvas.width / 2;
    var h = ctx.canvas.height / 2;
    var r = Math.sqrt(w * w + h * h);

    console.log("angle: ", this.angle);
    ctx.translate(w, h);
    ctx.rotate(this.angle);

    var phase = this.phase;
    var colp = 0;
    var widp = 0;

    for (var x = -r - phase; x <= r;) {
      var ww = this.widths[widp++];
      if (widp == this.widths.length) widp = 0;
      var cc = this.cols[colp++];
      if (colp == this.cols.length) colp = 0;

      ctx.fillStyle = cc;
      ctx.fillRect(x, -r, ww, r * 2);
      x += ww;
    }

  },

  getFrames: function() {
    return this.frames;
  }
});

function makeTestcard(mjpegFile, opts) {

  var movie = new MM.SequenceClip();

  movie.append(new BarsClip(['rgb(0, 0, 0)', 'rgb(255, 255, 255)'], [50, 50], 400, {
    angle: function(ctx) {
      return Math.PI * 2 * this.portion;
    }
  }));

  new MM.MovieMaker(mjpegFile, WIDTH, HEIGHT, movie).render();
}

function usage() {
  var me = process.argv[0] + " " + process.argv[1];
  console.log("Usage: " + me + " <output.mjpeg>");
  process.exit(1);
}

var argv = require('minimist')(process.argv.slice(2), {
  string: ['t']
});
var args = argv._;
if (args.length != 1) usage();

makeTestcard(args[0], argv);

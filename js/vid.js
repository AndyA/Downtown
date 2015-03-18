var Canvas = require('canvas');
var fs = require('fs');

function Clip(render, frames) {
  this.render = render;
  this.frames = frames;
}

Clip.prototype = {
  getFrames: function() {
    return this.frames;
  }
}

function ReverseClip(clip) {
  this.clip = clip;
  this.frames = clip.getFrames();
}

ReverseClip.prototype = {
  render: function(canvas, ctx, framenum) {
    this.clip.render(canvas, ctx, this.frames - 1 - framenum);
  },

  getFrames: function() {
    return this.frames;
  }
}

function SequenceClip() {
  this.clips = [];
  for (var i = 0; i < arguments.length; i++) {
    this.clips.push(arguments[i]);
  }
}

SequenceClip.prototype = {
  append: function() {
    for (var i = 0; i < arguments.length; i++) {
      this.clips.push(arguments[i]);
    }
  },

  render: function(canvas, ctx, framenum) {
    var clips = this.clips;
    for (var i = 0; i < clips.length; i++) {
      var clip_frames = clips[i].getFrames();
      if (framenum < clip_frames) {
        clips[i].render(canvas, ctx, framenum);
        return;
      }
      framenum -= clip_frames;
    }
  },

  getFrames: function() {
    var frames = 0;
    var clips = this.clips;
    for (var i = 0; i < clips.length; i++) {
      frames += clips[i].getFrames();

    }
    return frames;
  }
}

function OverlayClip() {
  this.clips = [];
  for (var i = 0; i < arguments.length; i++) {
    this.clips.push(arguments[i]);
  }
}

OverlayClip.prototype = {

  render: function(canvas, ctx, framenum) {
    var clips = this.clips;
    for (var i = 0; i < clips.length; i++) {
      ctx.save();
      clips[i].render(canvas, ctx, framenum);
      ctx.restore();
    }
  },

  getFrames: function() {
    var frames = null;
    var clips = this.clips;

    for (var i = 0; i < clips.length; i++) {
      var clip_frames = clips[i].getFrames();
      if (clip_frames !== null && (frames === null || clip_frames < frames)) {
        frames = clip_frames;
      }

    }
    return frames;
  }
}

function EditClip(clip, in_frame, frames) {
  var clip_frames = clip.getFrames();
  if (in_frame >= clip_frames) in_frame = clip_frames;
  if (in_frame + frames > clip_frames) frames = clip_frames - in_frame;

  this.clip = clip;
  this.in_frame = in_frame;
  this.frames = frames;
}

EditClip.prototype = {

  render: function(canvas, ctx, framenum) {
    this.clip.render(canvas, ctx, framenum + this.in_frame);
  },

  getFrames: function() {
    return this.frames;
  }
}

function TimecodeClip() {}

TimecodeClip.prototype = {
  render: function(canvas, ctx, framenum) {
    ctx.font = "18px monospace";
    ctx.fillStyle = 'green';
    ctx.fillText(framenum, 100, 100);
    //    ctx.fillRect(framenum, 0, 10, canvas.height);
  },

  getFrames: function() {
    return null; // unlimited
  }
}

function MovieMaker(filename, width, height, clip) {
  this.filename = filename;
  this.width = width;
  this.height = height;
  this.clip = clip;
}

MovieMaker.prototype = {
  render: function() {

    var framenum = 0;
    var frames = this.clip.getFrames();
    var self = this;

    var out = fs.createWriteStream(this.filename);

    function drawFrame() {
      var canvas = new Canvas(self.width, self.height);
      var ctx = canvas.getContext('2d');
      console.log("Rendering frame " + framenum);
      ctx.save();
      self.clip.render(canvas, ctx, framenum);
      ctx.restore();
      var stream = canvas.createJPEGStream();

      stream.on('data', function(chunk) {
        out.write(chunk);
      });

      stream.on('end', function() {
        if (++framenum < frames) drawFrame();
      });
    }

    drawFrame();
  }
}

function drawSpiral(ctx, limit, r, a, r_rate, a_rate) {
  ctx.beginPath();
  var point = 0;

  while (a < limit) {
    var px = Math.sin(a) * r;
    var py = Math.cos(a) * r;

    //    console.log("px=" + px + ", py=" + py);
    if (point) ctx.lineTo(px, py);
    else ctx.moveTo(px, py);

    a += a_rate / r;
    r += r_rate / r;
    point++;
  }
  ctx.stroke();
}

function spiral(canvas, ctx, framenum) {
  ctx.save();
  // ctx.scale() seems to cause drawing to stop
  ctx.translate(canvas.width / 2, canvas.height / 2);
  ctx.lineWidth = 5;
  ctx.lineCap = 'round';
  ctx.lineJoin = 'round';

  var phase = ['rgb(180, 0, 0)', 'rgb(0, 180, 0)', 'rgb(80, 80, 200)'];
  var shift = 0;

  for (var i = 0; i < phase.length; i++) {
    //    console.log("phase " + i + ", shift " + shift);
    ctx.strokeStyle = phase[i];
    drawSpiral(ctx, framenum + shift, 5, shift, 20, 3);
    shift += Math.PI * 2 / phase.length;
  }

  ctx.restore();
}

var clip = new Clip(spiral, 100);
var movie = new SequenceClip(clip, new ReverseClip(clip));

movie = new OverlayClip(movie, new TimecodeClip());

var mm = new MovieMaker(__dirname + '/../state.mjpeg', 1920, 1080, movie);

mm.render();

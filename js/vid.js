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

function TitleClip(text, frames, x, y, setStyle) {
  this.text = text;
  this.frames = frames;
  this.x = x || 0.5;
  this.y = y || 0.5;

  this.setStyle = setStyle;
}

TitleClip.prototype = {
  render: function(canvas, ctx, framenum) {
    ctx.textAlign = 'left';
    ctx.textBaseline = 'alphabetic';
    ctx.font = "80px sans-serif";
    ctx.fillStyle = 'white';
    if (this.setStyle) this.setStyle(ctx);
    var metrics = ctx.measureText(this.text);
    var height = metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent;

    var dw = canvas.width - metrics.width;
    var dh = canvas.height - height;

    ctx.fillText(this.text, dw * this.x - metrics.actualBoundingBoxLeft, dh * this.y - metrics.actualBoundingBoxDescent);
  },

  getFrames: function() {
    return this.frames;
  }
}

function DisolveClip(clip_a, clip_b, frames, easing) {
  this.clip_a = clip_a;
  this.clip_b = clip_b;
  this.frames = frames;
  this.easing = easing ||
  function(x) {
    return x
  }

  this.frames_a = clip_a.getFrames();
  this.frames_b = clip_b.getFrames();
  if (this.frames > this.frames_a) this.frames = this.frames_a;
  if (this.frames > this.frames_b) this.frames = this.frames_b;
}

DisolveClip.prototype = {
  render: function(canvas, ctx, framenum) {
    var start = this.frames_a - this.frames;
    if (framenum < start) {
      this.clip_a.render(canvas, ctx, framenum);
    } else if (framenum < this.frames_a) {
      var mix = this.easing((framenum - start) / this.frames);

      ctx.save();

      // Draw clip_a
      var alpha = ctx.globalAlpha;

      ctx.globalAlpha = alpha * (1 - mix);
      ctx.fillStyle = 'black';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      this.clip_a.render(canvas, ctx, framenum);

      ctx.restore();

      // Overlay clip_b
      ctx.globalAlpha = alpha * mix;
      ctx.fillStyle = 'black';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      this.clip_b.render(canvas, ctx, framenum - start);

    } else {
      this.clip_b.render(canvas, ctx, framenum - start);
    }

  },

  getFrames: function() {
    return this.frames_a - this.frames + this.frames_b;
  }
}

function MovieMaker(filename, width, height, clip) {
  this.filename = filename;
  this.width = width;
  this.height = height;
  this.clip = clip;
}

function BlankClip(frames) {
  this.frames = frames;
}

BlankClip.prototype = {
  render: function(canvas, ctx, framenum) {},

  getFrames: function() {
    return this.frames;
  }
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

function drawSpirals(canvas, ctx, limit, shift) {
  ctx.save();
  // ctx.scale() seems to cause drawing to stop
  ctx.translate(canvas.width / 2, canvas.height / 2);
  ctx.lineWidth = 5;
  ctx.lineCap = 'round';
  ctx.lineJoin = 'round';

  var phase = ['rgb(180, 0, 0)', 'rgb(0, 180, 0)', 'rgb(80, 80, 200)'];

  for (var i = 0; i < phase.length; i++) {
    //    console.log("phase " + i + ", shift " + shift);
    ctx.strokeStyle = phase[i];
    drawSpiral(ctx, limit + shift, 5, shift, 20, 3);
    shift += Math.PI * 2 / phase.length;
  }

  ctx.restore();
}

var spiral_limit = 100;

function spiral(canvas, ctx, framenum) {
  drawSpirals(canvas, ctx, framenum, 0);
}

function spinner(canvas, ctx, framenum) {
  var angle = (framenum * framenum) / 100;
  drawSpirals(canvas, ctx, spiral_limit, angle);
}

function fadeInOut(clip, in_frames, out_frames, in_pad, out_pad) {
  var in_blank = new BlankClip(in_frames + in_pad);
  var out_blank = new BlankClip(out_frames + out_pad);
  //  return new DisolveClip(in_blank, new DisolveClip(clip, out_blank, out_frames), in_frames);
  return new DisolveClip(new DisolveClip(in_blank, clip, in_frames), out_blank, out_frames);
}

function mediumText(ctx) {
  ctx.font = "36px sans-serif";
}

var title = new TitleClip("JavaScript Video Generator", 200, 0.5, 0.5);
var credit = new TitleClip("Andy Armstong, andy@hexten.net", 300, 0.8, 0.9, mediumText);

var spiral_clip = new Clip(spiral, spiral_limit);
var spinner_clip = new Clip(spinner, 200);

var wibble = new SequenceClip(spiral_clip, spinner_clip, new ReverseClip(spiral_clip));

var movie = new SequenceClip(fadeInOut(title, 50, 50, 25, 25), wibble, fadeInOut(credit, 50, 100, 25, 25));

//console.log(JSON.stringify(movie));
//movie = new OverlayClip(movie, new TimecodeClip());
var mm = new MovieMaker(__dirname + '/../state.mjpeg', 1920, 1080, movie);

mm.render();

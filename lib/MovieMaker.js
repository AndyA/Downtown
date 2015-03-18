module.exports = (function() {
  var Canvas = require('canvas');
  var fs = require('fs');

  var exports = {};

  // Exceptions
  exports.CircularReferenceError = function(message) {
    this.name = 'CircularReferenceError';
    this.message = message;
  }
  exports.CircularReferenceError.prototype = new Error();

  exports.DynamicPropertyBase = function() {}

  exports.RampProperty = function(min, max) {
    this.min = min
    this.max = max
  }

  exports.RampProperty.prototype = new exports.DynamicPropertyBase();

  exports.RampProperty.evaluate = function(framenum, portion) {
    return (max - min) * portion + min;
  }

  function ClipBase() {}

  ClipBase.prototype.clearPropertyCache = function() {
    delete this._prop_cache;
  }

  ClipBase.prototype.bindProperty = function(name, prop) {
    if (prop instanceof exports.DynamicPropertyBase) {
      this.bindProperty(name, function(framenum, portion, obj) {
        return prop.evaluate(framenum, portion, obj);
      });
      return;
    }

    if (prop instanceof Function) {
      Object.defineProperty(this, name, {
        get: function() {
          if (!this._prop_cache) this._prop_cache = {};
          if (this._prop_cache.hasOwnProperty(name)) return this._prop_cache[name];

          if (!this._prop_eval) this._prop_eval = {};
          if (this._prop_eval.hasOwnProperty(name)) {
            throw new exports.CircularReferenceError("Property " + name + " contains a circular reference");
          }
          this._prop_eval[name] = true;
          var framenum = this._current_frame;
          var nframes = this.getFrames(); // cache?
          this._prop_cache[name] = prop(framenum, framenum / nframes, this);
          delete this._prop_eval[name];
          return this._prop_cache[name];
        }
      });
    } else {
      Object.defineProperty(this, name, {
        get: function() {
          return prop
        }
      });
    }
  }

  ClipBase.prototype.makeFrame = function(canvas, ctx, framenum) {
    this.clearPropertyCache();
    this._current_frame = framenum;
    this.render(canvas, ctx, framenum);
  }

  // Simple rendered clip
  exports.Clip = function(render, frames) {
    this.render = render;
    this.frames = frames;
  }

  exports.Clip.prototype = new ClipBase();

  exports.Clip.prototype.getFrames = function() {
    return this.frames;
  }

  // Reverse clip
  exports.ReverseClip = function(clip) {
    this.clip = clip;
    this.frames = clip.getFrames();
  }

  exports.ReverseClip.prototype = new ClipBase();

  exports.ReverseClip.prototype.render = function(canvas, ctx, framenum) {
    this.clip.makeFrame(canvas, ctx, this.frames - 1 - framenum);
  }

  exports.ReverseClip.prototype.getFrames = function() {
    return this.frames;
  }

  // A sequence of clips
  exports.SequenceClip = function() {
    this.clips = [];
    for (var i = 0; i < arguments.length; i++) {
      this.clips.push(arguments[i]);
    }

  }

  exports.SequenceClip.prototype = new ClipBase();

  exports.SequenceClip.prototype.render = function(canvas, ctx, framenum) {
    var clips = this.clips;
    for (var i = 0; i < clips.length; i++) {
      var clip_frames = clips[i].getFrames();
      if (framenum < clip_frames) {
        clips[i].makeFrame(canvas, ctx, framenum);
        return;
      }
      framenum -= clip_frames;
    }
  }

  exports.SequenceClip.prototype.getFrames = function() {
    var frames = 0;
    var clips = this.clips;
    for (var i = 0; i < clips.length; i++) {
      frames += clips[i].getFrames();

    }
    return frames;
  }

  exports.SequenceClip.prototype.append = function() {
    for (var i = 0; i < arguments.length; i++) {
      this.clips.push(arguments[i]);
    }
  }

  exports.OverlayClip = function() {
    this.clips = [];
    for (var i = 0; i < arguments.length; i++) {
      this.clips.push(arguments[i]);
    }
  }

  exports.OverlayClip.prototype = new ClipBase();

  exports.OverlayClip.prototype.render = function(canvas, ctx, framenum) {
    var clips = this.clips;
    for (var i = 0; i < clips.length; i++) {
      ctx.save();
      clips[i].makeFrame(canvas, ctx, framenum);
      ctx.restore();
    }
  }

  exports.OverlayClip.prototype.getFrames = function() {
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

  exports.EditClip = function(clip, in_frame, frames) {
    var clip_frames = clip.getFrames();
    if (in_frame >= clip_frames) in_frame = clip_frames;
    if (in_frame + frames > clip_frames) frames = clip_frames - in_frame;

    this.clip = clip;
    this.in_frame = in_frame;
    this.frames = frames;
  }

  exports.EditClip.prototype = new ClipBase();

  exports.EditClip.prototype.render = function(canvas, ctx, framenum) {
    this.clip.makeFrame(canvas, ctx, framenum + this.in_frame);
  }

  exports.EditClip.prototype.getFrames = function() {
    return this.frames;
  }

  exports.TimecodeClip = function() {}
  exports.TimecodeClip.prototype = new ClipBase();

  exports.TimecodeClip.prototype.render = function(canvas, ctx, framenum) {
    ctx.font = "18px monospace";
    ctx.fillStyle = 'green';
    ctx.fillText(framenum, 100, 100);
  }

  exports.TimecodeClip.prototype.getFrames = function() {
    return null; // unlimited
  }

  exports.TitleClip = function(text, frames, x, y, setStyle) {
    this.text = text;
    this.frames = frames;

    this.bindProperty('x', arguments.length > 2 ? x : 0.5);
    this.bindProperty('y', arguments.length > 3 ? y : 0.5);

    this.setStyle = setStyle;
  }

  exports.TitleClip.prototype = new ClipBase();

  exports.TitleClip.prototype.render = function(canvas, ctx, framenum) {
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
  }

  exports.TitleClip.prototype.getFrames = function() {
    return this.frames;
  }

  exports.DisolveClip = function(clip_a, clip_b, frames, easing) {
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

  exports.DisolveClip.prototype = new ClipBase();

  exports.DisolveClip.prototype.render = function(canvas, ctx, framenum) {
    var start = this.frames_a - this.frames;
    if (framenum < start) {
      this.clip_a.makeFrame(canvas, ctx, framenum);
    } else if (framenum < this.frames_a) {
      var mix = this.easing((framenum - start) / this.frames);

      ctx.save();

      // Draw clip_a
      var alpha = ctx.globalAlpha;

      ctx.globalAlpha = alpha * (1 - mix);
      ctx.fillStyle = 'black';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      this.clip_a.makeFrame(canvas, ctx, framenum);

      ctx.restore();

      // Overlay clip_b
      ctx.globalAlpha = alpha * mix;
      ctx.fillStyle = 'black';
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      this.clip_b.makeFrame(canvas, ctx, framenum - start);

    } else {
      this.clip_b.makeFrame(canvas, ctx, framenum - start);
    }
  }

  exports.DisolveClip.prototype.getFrames = function() {
    return this.frames_a - this.frames + this.frames_b;
  }

  exports.BlankClip = function(frames) {
    this.frames = frames
  }

  exports.BlankClip.prototype = new ClipBase();
  exports.BlankClip.prototype.render = function(canvas, ctx, framenum) {}
  exports.BlankClip.prototype.getFrames = function() {
    return this.frames;
  }

  exports.MovieStream = function(out, width, height, clip) {
    this.init(out, width, height, clip);
  }

  exports.MovieStream.prototype = new Object();

  exports.MovieStream.prototype.init = function(out, width, height, clip) {
    this.out = out;
    this.width = width;
    this.height = height;
    this.clip = clip;
  }

  exports.MovieStream.prototype.render = function() {

    var framenum = 0;
    var frames = this.clip.getFrames();
    var self = this;

    function drawFrame() {
      var canvas = new Canvas(self.width, self.height);
      var ctx = canvas.getContext('2d');
      console.log("Rendering frame " + framenum);
      ctx.save();
      self.clip.makeFrame(canvas, ctx, framenum);
      ctx.restore();
      var stream = canvas.createJPEGStream();

      stream.on('data', function(chunk) {
        self.out.write(chunk);
      });

      stream.on('end', function() {
        if (++framenum < frames) drawFrame();
      });
    }

    drawFrame();
  }

  exports.MovieMaker = function(filename, width, height, clip) {
    var out = fs.createWriteStream(filename);
    this.init(out, width, height, clip);
  }

  exports.MovieMaker.prototype = new exports.MovieStream(null, 0, 0, null);

  return exports
})();

module.exports = (function() {
  var Canvas = require('canvas');
  var fs = require('fs');

  var exports = {};

  exports.Clip = function(render, frames) {
    this.render = render;
    this.frames = frames;
  }

  exports.Clip.prototype = {
    getFrames: function() {
      return this.frames;
    }
  }

  exports.ReverseClip = function(clip) {
    this.clip = clip;
    this.frames = clip.getFrames();
  }

  exports.ReverseClip.prototype = {
    render: function(canvas, ctx, framenum) {
      this.clip.render(canvas, ctx, this.frames - 1 - framenum);
    },

    getFrames: function() {
      return this.frames;
    }
  }

  exports.SequenceClip = function() {
    this.clips = [];
    for (var i = 0; i < arguments.length; i++) {
      this.clips.push(arguments[i]);
    }
  }

  exports.SequenceClip.prototype = {
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

  exports.OverlayClip = function() {
    this.clips = [];
    for (var i = 0; i < arguments.length; i++) {
      this.clips.push(arguments[i]);
    }
  }

  exports.OverlayClip.prototype = {

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

  exports.EditClip = function(clip, in_frame, frames) {
    var clip_frames = clip.getFrames();
    if (in_frame >= clip_frames) in_frame = clip_frames;
    if (in_frame + frames > clip_frames) frames = clip_frames - in_frame;

    this.clip = clip;
    this.in_frame = in_frame;
    this.frames = frames;
  }

  exports.EditClip.prototype = {

    render: function(canvas, ctx, framenum) {
      this.clip.render(canvas, ctx, framenum + this.in_frame);
    },

    getFrames: function() {
      return this.frames;
    }
  }

  exports.TimecodeClip = function() {}

  exports.TimecodeClip.prototype = {
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

  exports.TitleClip = function(text, frames, x, y, setStyle) {
    this.text = text;
    this.frames = frames;
    this.x = x || 0.5;
    this.y = y || 0.5;

    this.setStyle = setStyle;
  }

  exports.TitleClip.prototype = {
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

  exports.DisolveClip.prototype = {
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

  exports.BlankClip = function(frames) {
    this.frames = frames;
  }

  exports.BlankClip.prototype = {
    render: function(canvas, ctx, framenum) {},

    getFrames: function() {
      return this.frames;
    }
  }

  exports.MovieStream = function(out, width, height, clip) {
    this.init(out, width, height, clip);
  }

  exports.MovieStream.prototype = {
    init: function(out, width, height, clip) {
      this.out = out;
      this.width = width;
      this.height = height;
      this.clip = clip;
    },
    render: function() {

      var framenum = 0;
      var frames = this.clip.getFrames();
      var self = this;

      function drawFrame() {
        var canvas = new Canvas(self.width, self.height);
        var ctx = canvas.getContext('2d');
        console.log("Rendering frame " + framenum);
        ctx.save();
        self.clip.render(canvas, ctx, framenum);
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
  }

  exports.MovieMaker = function(filename, width, height, clip) {
    var out = fs.createWriteStream(filename);
    this.init(out, width, height, clip);
  }

  exports.MovieMaker.prototype = new exports.MovieStream(null, 0, 0, null);

  return exports
})();

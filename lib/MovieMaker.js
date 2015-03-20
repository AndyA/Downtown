"use strict";

var Canvas = require('canvas');
var fs = require('fs');

module.exports = (function() {

  exports = {};

  exports.util = require("./MovieMaker/Util.js")

  var defineClass = exports.util.defineClass;
  var mixin = exports.util.mixin;;

  var dynamicPropertyMixin = require("./MovieMaker/DynamicProperty.js")

  var exposeContextMixin = {
    exposeContext: function() {
      this.bindProperty('framenum', function(ctx) {
        return ctx.framenum
      });
      this.bindProperty('portion', function(ctx) {
        return ctx.portion
      });
      this.bindProperty('target', function(ctx) {
        return ctx.clip
      });
    }
  }

  //////////////////
  // CircularReferenceError
  //////////////////
  //
  exports.CircularReferenceError = defineClass(
  function(message) {
    this.name = 'CircularReferenceError';
    this.message = message;
  },
  new Error());

  //////////////////
  // StackEmptyError
  //////////////////
  //
  exports.StackEmptyError = defineClass(
  function(message) {
    this.name = 'StackEmptyError';
    this.message = message;
  },
  new Error());

  //////////////////
  // DynamicPropertyBase
  //////////////////
  //
  exports.DynamicPropertyBase = defineClass(function() {
    this.exposeContext()
  },
  {},
  dynamicPropertyMixin, exposeContextMixin);

  //////////////////
  // RampProperty
  //////////////////
  //
  exports.RampProperty = defineClass(
  function(min, max) {
    this.min = min
    this.max = max
  },
  new exports.DynamicPropertyBase(), {
    evaluate: function(ctx) {
      return (this.max - this.min) * ctx.portion + this.min;
    }
  });

  //////////////////
  // CircleProperty
  //////////////////
  //
  exports.CircleProperty = defineClass(function(params) {
    var defaults = {
      frequency: 1,
      amplitute: 1,
      phase: 0,
      scale_x: 1,
      scale_y: 1,
      offset_x: 0,
      offset_y: 0
    }

    this.bindParameters(params, defaults);

    this.bindProperty('x', function(ctx) {
      var angle = (ctx.portion * this.frequency + this.phase) * Math.PI * 2;
      return Math.sin(angle) * this.amplitute * this.scale_x + this.offset_x;
    });

    this.bindProperty('y', function(ctx) {
      var angle = (ctx.portion * this.frequency + this.phase) * Math.PI * 2;
      return Math.cos(angle) * this.amplitute * this.scale_y + this.offset_y;
    });
  },
  new exports.DynamicPropertyBase(), {
    evaluate: function(ctx) {
      throw new Error("I have no value");
    }
  });

  //////////////////
  // ClipBase
  //////////////////
  //
  exports.ClipBase = defineClass(function() {
    this.exposeContext();
  },
  {},
  dynamicPropertyMixin, exposeContextMixin, {
    setup: function() {},
    makeFrame: function(ctx, framenum) {
      if (!this._done_setup++) this.setup();
      this.pushContext({
        framenum: framenum,
        portion: framenum / this.getFrames(),
        clip: this
      });
      try {
        this.render(ctx, framenum);
      } finally {
        this.popContext();
      }
    }
  });

  //////////////////
  // Clip
  //////////////////
  //
  exports.Clip = defineClass(function(render, frames) {
    this.render = render;
    this.frames = frames;
  },
  new exports.ClipBase(), {
    getFrames: function() {
      return this.frames;
    }
  });

  //////////////////
  // ReverseClip
  //////////////////
  //
  exports.ReverseClip = defineClass(function(clip) {
    this.clip = clip;
    this.frames = clip.getFrames();
  },
  new exports.ClipBase(), {
    render: function(ctx, framenum) {
      this.clip.makeFrame(ctx, this.frames - 1 - framenum);
    },
    getFrames: function() {
      return this.frames;
    }
  });

  //////////////////
  // SequenceClip
  //////////////////
  //
  exports.SequenceClip = defineClass(function() {
    var args = Array.prototype.slice.call(arguments);
    this.clips = [];
    this.append(args);
  },

  new exports.ClipBase(), {
    render: function(ctx, framenum) {
      var clips = this.clips;
      for (var i = 0; i < clips.length; i++) {
        var clip_frames = clips[i].getFrames();
        if (framenum < clip_frames) {
          clips[i].makeFrame(ctx, framenum);
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
    },

    getClips: function() {
      return this.clips.slice();
    },

    append: function() {
      var args = Array.prototype.slice.call(arguments);
      while (args.length) {
        var arg = args.shift();
        if (arg instanceof Array) {
          Array.prototype.unshift.apply(args, arg);
          continue;
        }
        this.clips.push(arg);
      }
    },

    map: function(map_func) {
      var nseq = new exports.SequenceClip();
      var clips = this.clips;
      for (var i = 0; i < clips.length; i++) {
        var out_clip = map_func(clips[i]);
        if (!out_clip) continue;
        nseq.append(out_clip);
      }
      return nseq;
    },

    reduce: function(reduce_func) {
      var clips = this.getClips();
      var accum = clips.shift();

      while (clips.length) {
        var out_clip = reduce_func(accum, clips.shift())
        if (out_clip) accum = out_clip;
      }

      return accum;
    }

  }

  );

  //////////////////
  // OverlayClip
  //////////////////
  //
  exports.OverlayClip = defineClass(function() {
    this.clips = [];
    for (var i = 0; i < arguments.length; i++) {
      this.clips.push(arguments[i]);
    }
  },
  new exports.ClipBase(), {
    render: function(ctx, framenum) {
      var clips = this.clips;
      for (var i = 0; i < clips.length; i++) {
        ctx.save();
        clips[i].makeFrame(ctx, framenum);
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
  });

  //////////////////
  // EditClip
  //////////////////
  //
  exports.EditClip = defineClass(function(clip, in_frame, frames) {
    var clip_frames = clip.getFrames();
    if (in_frame >= clip_frames) in_frame = clip_frames;
    if (in_frame + frames > clip_frames) frames = clip_frames - in_frame;
    this.clip = clip;
    this.in_frame = in_frame;
    this.frames = frames;
  },
  new exports.ClipBase(), {
    render: function(ctx, framenum) {
      this.clip.makeFrame(ctx, framenum + this.in_frame);
    },
    getFrames: function() {
      return this.frames;
    }
  });

  //////////////////
  // TimecodeClip
  //////////////////
  //
  exports.TimecodeClip = defineClass(new exports.ClipBase(), {
    render: function(ctx, framenum) {
      ctx.font = "18px monospace";
      ctx.fillStyle = 'green';
      ctx.fillText(framenum, 100, 100);
    },
    getFrames: function() {
      return null;
    }
  });

  //////////////////
  // TextClip
  //////////////////
  //
  exports.TextClip = defineClass(function(frames, params) {
    this.frames = frames;
    var defaults = {
      text: "Hello, World",
      offset_x: 0,
      offset_y: 0,
      x: 0.5,
      y: 0.5,
      textAlign: 'left',
      fillStyle: 'white',
      font: "80px sans-serif"
    }

    this.bindParameters(params, defaults);
  },
  new exports.ClipBase(), {
    render: function(ctx, framenum) {
      ctx.textBaseline = 'alphabetic';
      ctx.textAlign = this.textAlign;
      ctx.font = this.font;
      ctx.fillStyle = this.fillStyle;

      var metrics = ctx.measureText(this.text);
      var height = metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent;

      var dw = ctx.canvas.width - metrics.width;
      var dh = ctx.canvas.height - height;

      var xx = this.offset_x + dw * this.x - metrics.actualBoundingBoxLeft
      var yy = this.offset_y + dh * this.y - metrics.actualBoundingBoxDescent

      ctx.fillText(this.text, xx, yy);
    },
    getFrames: function() {
      return this.frames;
    }
  });

  //////////////////
  // TitleClip
  //////////////////
  //
  exports.TitleClip = defineClass(function(text, frames, x, y, setStyle) {
    this.text = text;
    this.frames = frames;
    this.bindProperty('x', arguments.length > 2 ? x : 0.5);
    this.bindProperty('y', arguments.length > 3 ? y : 0.5);
    this.setStyle = setStyle;
  },
  new exports.ClipBase(), {
    render: function(ctx, framenum) {
      ctx.textAlign = 'left';
      ctx.textBaseline = 'alphabetic';
      ctx.font = "80px sans-serif";
      ctx.fillStyle = 'white';
      if (this.setStyle) this.setStyle(ctx);
      var metrics = ctx.measureText(this.text);
      var height = metrics.actualBoundingBoxAscent + metrics.actualBoundingBoxDescent;
      var dw = ctx.canvas.width - metrics.width;
      var dh = ctx.canvas.height - height;
      ctx.fillText(this.text, dw * this.x - metrics.actualBoundingBoxLeft, dh * this.y - metrics.actualBoundingBoxDescent);
    },
    getFrames: function() {
      return this.frames;
    }
  });

  //////////////////
  // DisolveClip
  //////////////////
  //
  exports.DisolveClip = defineClass(function(clip_a, clip_b, frames, easing) {
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
  },
  new exports.ClipBase(), {
    render: function(ctx, framenum) {
      var start = this.frames_a - this.frames;
      if (framenum < start) {
        this.clip_a.makeFrame(ctx, framenum);
      } else if (framenum < this.frames_a) {
        // TODO: use a dynamic property
        var mix = this.easing((framenum - start) / this.frames);
        ctx.save();

        // Draw clip_a
        var alpha = ctx.globalAlpha;
        ctx.globalAlpha = alpha * (1 - mix);
        ctx.fillStyle = 'black';
        ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
        this.clip_a.makeFrame(ctx, framenum);
        ctx.restore();

        // Overlay clip_b
        ctx.globalAlpha = alpha * mix;
        ctx.fillStyle = 'black';
        ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
        this.clip_b.makeFrame(ctx, framenum - start);
      } else {
        this.clip_b.makeFrame(ctx, framenum - start);
      }
    },
    getFrames: function() {
      return this.frames_a - this.frames + this.frames_b;
    }
  });

  //////////////////
  // BlankClip
  //////////////////
  //
  exports.BlankClip = defineClass(function(frames) {
    this.frames = frames
  },
  new exports.ClipBase(), {
    render: function(ctx, framenum) {},
    getFrames: function() {
      return this.frames;
    }
  }

  );

  //////////////////
  // TransformClip
  //////////////////
  //
  exports.TransformClip = defineClass(function(clip, params) {
    this.clip = clip;
    var defaults = {
      // effect origin
      origin_x: 0.5,
      origin_y: 0.5,
      // convenience
      rotate: 0,
      scale_x: 1,
      scale_y: 1,
      translate_x: 0,
      translate_y: 0,
      // direct matrix access
      transform_a: 1,
      transform_b: 0,
      transform_c: 0,
      transform_d: 1,
      transform_e: 0,
      transform_f: 0
    };
    this.bindParameters(params, defaults);
  },
  new exports.ClipBase(), {
    applyToContext: function(ctx) {
      var shift_x = ctx.canvas.width * this.origin_x;
      var shift_y = ctx.canvas.height * this.origin_y;
      // Convenience 
      ctx.translate(shift_x, shift_y);
      ctx.scale(this.scale_x, this.scale_y);
      ctx.rotate(this.rotate);
      ctx.translate(this.translate_x - shift_x, this.translate_y - shift_y);
      // Raw matrix
      ctx.transform(
      this.transform_a, this.transform_b, this.transform_c, //
      this.transform_d, this.transform_e, this.transform_f);
    },
    render: function(ctx, framenum) {
      ctx.save();
      this.applyToContext(ctx);
      this.clip.makeFrame(ctx, framenum);
      ctx.restore()
    },
    getFrames: function() {
      return this.clip.getFrames();
    }
  });

  //////////////////
  // MovieStream
  //////////////////
  //
  var movieMaker = {
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
        self.clip.makeFrame(ctx, framenum);
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
  };

  exports.MovieStream = defineClass(function(out, width, height, clip) {
    this.init(out, width, height, clip);
  },
  {},
  movieMaker);

  //////////////////
  // MovieMaker
  //////////////////
  //
  exports.MovieMaker = defineClass(function(filename, width, height, clip) {
    var out = fs.createWriteStream(filename);
    this.init(out, width, height, clip);
  },
  {},
  movieMaker);

  return exports
})();

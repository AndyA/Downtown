"use strict";

var Canvas = require('canvas');
var fs = require('fs');

module.exports = (function() {

  var exports = {};

  exports.util = {
    mixin: function() {
      if (arguments.length == 0) throw new Error("Missing argument to mixin");
      var target = arguments[0];
      for (var i = 1; i < arguments.length; i++) {
        var mix = arguments[i];
        var keys = Object.keys(mix);
        for (var j = 0; j < keys.length; j++) {
          target[keys[j]] = mix[keys[j]];
        }
      }
      return target;
    },

    defineClass: function() {
      var args = Array.prototype.slice.call(arguments);

      var klass = (args.length && args[0] instanceof Function) ? args.shift() : function() {};

      if (args.length && args[0] instanceof Array) { // class methods
        var mixin_args = args.shift().slice();
        mixin_args.unshift(klass);
        exports.util.mixin.apply(null, mixin_args);
      }

      if (!args.length) args.push({});
      klass.prototype = exports.util.mixin.apply(null, args);

      return klass;
    }

  }

  var defineClass = exports.util.defineClass;
  var mixin = exports.util.mixin;;

  //////////////////
  // dynamicPropertyMixin
  //////////////////
  //
  var dynamicPropertyMixin = {

    clearPropertyCache: function() {
      delete this._prop_cache;
    },

    bindProperty: function(name, prop) {
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
    },

    bindParameters: function(params, defaults) {
      var keys = Object.keys(defaults);
      for (var i = 0; i < keys.length; i++) {
        var key = keys[i];
        var val = params && params.hasOwnProperty(key) ? params[key] : defaults[key];
        this.bindProperty(key, val);
      }
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
  // DynamicPropertyBase
  //////////////////
  //
  exports.DynamicPropertyBase = defineClass({},
  dynamicPropertyMixin);

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
    evaluate: function(framenum, portion) {
      return (this.max - this.min) * portion + this.min;
    }
  });

  //////////////////
  // ClipBase
  //////////////////
  //
  exports.ClipBase = defineClass({},
  dynamicPropertyMixin, {
    makeFrame: function(canvas, ctx, framenum) {
      this.clearPropertyCache();
      this._current_frame = framenum;
      this.render(canvas, ctx, framenum);
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
    render: function(canvas, ctx, framenum) {
      this.clip.makeFrame(canvas, ctx, this.frames - 1 - framenum);
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
    render: function(canvas, ctx, framenum) {
      var clips = this.clips;
      for (var i = 0; i < clips.length; i++) {
        var clip_frames = clips[i].getFrames();
        if (framenum < clip_frames) {
          clips[i].makeFrame(canvas, ctx, framenum);
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
    render: function(canvas, ctx, framenum) {
      var clips = this.clips;
      for (var i = 0; i < clips.length; i++) {
        ctx.save();
        clips[i].makeFrame(canvas, ctx, framenum);
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
    render: function(canvas, ctx, framenum) {
      this.clip.makeFrame(canvas, ctx, framenum + this.in_frame);
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
    render: function(canvas, ctx, framenum) {
      ctx.font = "18px monospace";
      ctx.fillStyle = 'green';
      ctx.fillText(framenum, 100, 100);
    },
    getFrames: function() {
      return null;
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
    render: function(canvas, ctx, framenum) {
      var start = this.frames_a - this.frames;
      if (framenum < start) {
        this.clip_a.makeFrame(canvas, ctx, framenum);
      } else if (framenum < this.frames_a) {
        // TODO: use a dynamic property
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
    render: function(canvas, ctx, framenum) {},
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
    render: function(canvas, ctx, framenum) {
      ctx.save();
      this.applyToContext(ctx);
      this.clip.makeFrame(canvas, ctx, framenum);
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

"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");
var Canvas = require("canvas");

describe("MovieMaker", function() {

  describe("util", function() {

    describe("mixin", function() {

      it("should merge objects", function() {

        expect(MM.util.mixin({},
        {
          a: 1
        })).to.deep.equal({
          a: 1
        });

        expect(MM.util.mixin({
          a: 1,
          b: 2,
          c: 3,
          d: 4,
          e: 5,
          f: 6
        },
        {
          a: 101,
          c: 103
        },
        {
          a: 201,
          d: 204
        })).to.deep.equal({
          a: 201,
          b: 2,
          c: 103,
          d: 204,
          e: 5,
          f: 6,
        });

      });

      it("should create working classes", function() {

        var MathMixin = {
          getName: function() {
            return "MathMixin"
          },

          fib: function(n) {
            if (n < 2) return n
            return this.fib(n - 1) + this.fib(n - 2)
          }
        }

        var LieMixin = {
          getName: function() {
            return "LieMixin"
          },

          fib: function(n) {
            return n + " == " + (n + 1)
          }
        }

        function ThingA() {}

        ThingA.prototype = {
          getName: function() {
            return "ThingA"
          },

          beThingA: function() {
            throw new Error("Don't call me!");
          }
        }

        function ThingB() {}

        ThingB.prototype = {
          getName: function() {
            return "ThingB"
          },

          beThingB: function() {
            throw new Error("Don't call me!");
          }
        }

        MM.util.mixin(ThingA.prototype, MathMixin);
        MM.util.mixin(ThingB.prototype, LieMixin);

        var thing_a = new ThingA();
        var thing_b = new ThingB();

        expect(thing_a).to.respondTo("getName").and.respondTo("beThingA").and.respondTo("fib");
        expect(thing_b).to.respondTo("getName").and.respondTo("beThingB").and.respondTo("fib");

        expect(thing_a.getName()).to.equal("MathMixin");
        expect(thing_b.getName()).to.equal("LieMixin");

        expect(thing_a.fib(5)).to.equal(5);
        expect(thing_b.fib(5)).to.equal("5 == 6");

      });

    });

    describe("defineClass", function() {

      it("should define simple classes", function() {
        var SuperFoo = MM.util.defineClass({
          greet: function(n) {
            return "Hello, " + n
          }
        })

        var super_foo = new SuperFoo();
        expect(super_foo.greet("you")).to.equal("Hello, you");

      });

      it("should allow a constructor to be provided", function() {
        var constructor_called = 0;
        var ExtraFoo = MM.util.defineClass(function() {
          constructor_called++;
        },
        {
          greet: function(n) {
            return "Goodbye, " + n
          }
        })

        var extra_foo = new ExtraFoo();
        expect(constructor_called).to.equal(1);
        expect(extra_foo.greet("you")).to.equal("Goodbye, you");
      });

      it("should allow class methods to be defined", function() {
        var constructor_called = 0;
        var UltaFoo = MM.util.defineClass(function() {
          constructor_called++;
        },
        [{
          version: function() {
            return 3
          },
          status: function() {
            return "current"
          }
        },
        {
          status: function() {
            return "deprecated"
          }
        }], {
          greet: function(n) {
            return "Goodbye, " + n
          }
        })

        expect(UltaFoo).itself.to.respondTo('version').and.respondTo('status');
        expect(UltaFoo.version()).to.equal(3);
        expect(UltaFoo.status()).to.equal("deprecated");

        var ultra_foo = new UltaFoo();
        expect(constructor_called).to.equal(1);
        expect(ultra_foo.greet("you")).to.equal("Goodbye, you");

      });

      it("should allow class methods with no constructor", function() {
        var MegaFoo = MM.util.defineClass([{
          version: function() {
            return 3
          },
          status: function() {
            return "current"
          }
        },
        {
          status: function() {
            return "deprecated"
          }
        }], {
          greet: function(n) {
            return "Goodbye, " + n
          }
        })

        expect(MegaFoo).itself.to.respondTo('version').and.respondTo('status');
        expect(MegaFoo.version()).to.equal(3);
        expect(MegaFoo.status()).to.equal("deprecated");

        var mega_foo = new MegaFoo();
        expect(mega_foo.greet("you")).to.equal("Goodbye, you");

      });

    });

  });

  describe("DynamicPropertyBase", function() {

    it("should handle evaluate", function() {

      var ramp = new MM.RampProperty(100, 200);

      expect(ramp).to.respondTo('evaluate');

      expect(ramp.evaluate(0, 0)).to.equal(100);
      expect(ramp.evaluate(50, 0.5)).to.equal(150);
      expect(ramp.evaluate(100, 1)).to.equal(200);

    });

  });

  describe("ClipBase", function() {

    it("should handle bound properties", function() {

      var calls_x = 0;
      var calls_y = 0;
      var calls_render = 0;
      var stuff_seen;

      function render(canvas, ctx, framenum) {
        calls_render++;
        stuff_seen = [];

        switch (framenum) {
        case 0:
          /* do nothing */
          break;

        case 1:
        case 2:
        case 3:
          for (var i = 0; i < framenum; i++) stuff_seen.push(this.x);
          break;

        case 4:
        case 5:
        case 6:
          for (var i = 0; i < framenum; i++) stuff_seen.push(this.y);
          break;
        }
      }

      var c = new MM.Clip(render, 100);

      // Abuse Clip: bind our own properties
      c.bindProperty('x', function(framenum, portion, clip) {
        calls_x++;
        return framenum * 2;
      });

      c.bindProperty('y', function(framenum, portion, clip) {
        calls_y++;
        return clip.x + clip.z
      });

      c.bindProperty('z', 100);

      var expect_seen = [
        [],
        [2],
        [4, 4],
        [6, 6, 6],
        [108, 108, 108, 108],
        [110, 110, 110, 110, 110],
        [112, 112, 112, 112, 112, 112]];

      c.makeFrame(null, null, 0);

      expect(calls_render).to.equal(1);
      expect(calls_x).to.equal(0);
      expect(calls_y).to.equal(0);
      expect(stuff_seen).to.deep.equal(expect_seen[0]);

      for (var fn = 1; fn < 4; fn++) {
        c.makeFrame(null, null, fn);
        expect(calls_render).to.equal(fn + 1);
        expect(calls_x).to.equal(fn);
        expect(calls_y).to.equal(0);
        expect(stuff_seen).to.deep.equal(expect_seen[fn]);
      }

      for (fn = 4; fn < 7; fn++) {
        c.makeFrame(null, null, fn);
        expect(calls_render).to.equal(fn + 1);
        expect(calls_x).to.equal(fn);
        expect(calls_y).to.equal(fn - 3);
        expect(stuff_seen).to.deep.equal(expect_seen[fn]);
      }

    });

    it("should throw an exception for circular references", function() {

      var stuff = [];

      function render(canvas, ctx, framenum) {
        stuff.push(this.eric);
      }

      var c = new MM.Clip(render, 100);

      c.bindProperty('eric', function(framenum, portion, clip) {
        return clip.ernie + 1;
      });

      c.bindProperty('ernie', function(framenum, portion, clip) {
        return clip.eric + 1;
      });

      expect(function() {
        c.makeFrame(null, null, 0)
      }).to.Throw(MM.CircularReferenceError);

    });

    it("should handle a long chain of references", function() {

      var stuff = [];

      function render(canvas, ctx, framenum) {
        stuff.push(this.prop9)
      }

      var c = new MM.Clip(render, 100);
      var calls_to = {};

      c.bindProperty('prop0', function(framenum, portion, clip) {
        calls_to['prop0']++;
        return framenum * 2;
      });

      function bindChain(prop) {
        var last_prop = "prop" + (prop - 1);
        var this_prop = "prop" + prop;

        c.bindProperty(this_prop, function(framenum, portion, clip) {
          calls_to[this_prop]++;
          return clip[last_prop] + 5;
        });
      }

      for (var prop = 1; prop < 10; prop++) bindChain(prop);

      c.makeFrame(null, null, 1);

      expect(stuff).to.deep.equal([47]);
    });

    it("should allow multiple parameters to be bound", function() {

      var stuff = [];
      function render(canvas, ctx, framenum) {
        stuff.push({
          x: this.x,
          y: this.y,
          phase: this.phase,
          mix: this.mix,
          angle: this.angle
        });
      }

      var frames = 256;

      var c = new MM.Clip(render, frames);

      var defaults = {
        x: 0.5,
        y: 0.5,
        phase: 1,
        mix: new MM.RampProperty(100, 200),
        angle: function(framenum, portion, clip) {
          return 1 - clip.phase;
        }
      };

      var params = {
        x: 1,
        y: function(framenum, portion, clip) {
          return portion
        },
        mix: 10,
        phase: new MM.RampProperty(1, 0)
      }

      c.bindParameters(params, defaults);

      function predict(framenum) {
        return {
          x: 1,
          y: framenum / frames,
          phase: 1 - framenum / frames,
          mix: 10,
          angle: framenum / frames
        };
      }

      var want = [];

      for (var i = 0; i < 5; i++) {
        c.makeFrame(null, null, i);
        want.push(predict(i));
      }

      expect(stuff).to.deep.equal(want);

    });

    it("shouldn't mind if params is null", function() {
      function render(canvas, ctx, framenum) {}
      var c = new MM.Clip(render, 100);
      c.bindParameters(null, {});
    });

  });

  (function() {
    var clip_types = {
      "BlankClip": MM.BlankClip,
      "DisolveClip": MM.DisolveClip,
      "EditClip": MM.EditClip,
      "OverlayClip": MM.OverlayClip,
      "ReverseClip": MM.ReverseClip,
      "SequenceClip": MM.SequenceClip,
      "TimecodeClip": MM.TimecodeClip,
      "TitleClip": MM.TitleClip,
      "TransformClip": MM.TransformClip,
    };

    var clip_names = Object.keys(clip_types);

    for (var i = 0; i < clip_names.length; i++) {
      var name = clip_names[i];
      var klass = clip_types[name];

      describe(name, function() {

        it("supports standard methods", function() {
          var clip = new klass();
          expect(clip).to.respondTo('render').and.respondTo('getFrames');
        });

      });

    }
  })()

  describe("TransformClip", function() {

    it("should update context", function() {

      var canvas = new Canvas(10, 10);
      var ctx = canvas.getContext('2d');

      var ctx_trans = null;
      function render(canvas, ctx, framenum) {
        ctx_trans = ctx.currentTransform;
      }
      var clip = new MM.Clip(render, 100);

      var transform = new MM.TransformClip(clip);

      transform.makeFrame(canvas, ctx, 0);

      // Would be nice to think of something to assert
    });

  });

});

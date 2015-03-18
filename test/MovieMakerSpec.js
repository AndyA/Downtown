var expect = require("chai").expect;
var MM = require("../lib/MovieMaker.js");

describe("MovieMaker", function() {

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

    it("should be OK to have a long chain of references", function() {

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

  });

});

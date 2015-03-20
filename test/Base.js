"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");

describe("DynamicPropertyBase", function() {

  it("should handle evaluate", function() {

    var ramp = new MM.RampProperty(100, 200);

    expect(ramp).to.respondTo('evaluate');

    expect(ramp.evaluate({
      framenum: 0,
      portion: 0
    })).to.equal(100);
    expect(ramp.evaluate({
      framenum: 50,
      portion: 0.5
    })).to.equal(150);
    expect(ramp.evaluate({
      framenum: 100,
      portion: 1
    })).to.equal(200);

  });

});

describe("ClipBase", function() {

  describe("Bound properties", function() {

    var calls_x = 0;
    var calls_y = 0;
    var calls_render = 0;
    var stuff_seen;

    function render(ctx, framenum) {
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
    c.bindProperty('x', function(ctx) {
      calls_x++;
      return ctx.framenum * 2;
    });

    c.bindProperty('y', function(ctx) {
      calls_y++;
      return ctx.clip.x + ctx.clip.z
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

    c.makeFrame(null, 0);

    it("should not reference unused properties", function() {
      expect(calls_render).to.equal(1);
      expect(calls_x).to.equal(0);
      expect(calls_y).to.equal(0);
      expect(stuff_seen).to.deep.equal(expect_seen[0]);
    })

    it("should evaluate bound properties", function() {
      for (var fn = 1; fn < 4; fn++) {
        c.makeFrame(null, fn);
        expect(calls_render).to.equal(fn + 1);
        expect(calls_x).to.equal(fn);
        expect(calls_y).to.equal(0);
        expect(stuff_seen).to.deep.equal(expect_seen[fn]);
      }
    })

    it("should evaluate properties that depend on others", function() {
      for (var fn = 4; fn < 7; fn++) {
        c.makeFrame(null, fn);
        expect(calls_render).to.equal(fn + 1);
        expect(calls_x).to.equal(fn);
        expect(calls_y).to.equal(fn - 3);
        expect(stuff_seen).to.deep.equal(expect_seen[fn]);
      }
    })

  });

  it("should throw an exception for circular references", function() {

    var stuff = [];

    function render(ctx, framenum) {
      stuff.push(this.eric);
    }

    var c = new MM.Clip(render, 100);

    c.bindProperty('eric', function(ctx) {
      return ctx.clip.ernie + 1;
    });

    c.bindProperty('ernie', function(ctx) {
      return ctx.clip.eric + 1;
    });

    expect(function() {
      c.makeFrame(null, 0)
    }).to.Throw(MM.CircularReferenceError);

  });

  it("should handle a long chain of references", function() {

    var stuff = [];

    function render(ctx, framenum) {
      stuff.push(this.prop9)
    }

    var c = new MM.Clip(render, 100);
    var calls_to = {};

    c.bindProperty('prop0', function(ctx) {
      calls_to['prop0']++;
      return ctx.framenum * 2;
    });

    function bindChain(prop) {
      var last_prop = "prop" + (prop - 1);
      var this_prop = "prop" + prop;

      c.bindProperty(this_prop, function(ctx) {
        calls_to[this_prop]++;
        return ctx.clip[last_prop] + 5;
      });
    }

    for (var prop = 1; prop < 10; prop++) bindChain(prop);

    c.makeFrame(null, 1);

    expect(stuff).to.deep.equal([47]);
  });

  it("should allow multiple parameters to be bound", function() {

    var stuff = [];
    function render(ctx, framenum) {
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
      angle: function(ctx) {
        return 1 - ctx.clip.phase;
      }
    };

    var params = {
      x: 1,
      y: function(ctx) {
        return ctx.portion
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
      c.makeFrame(null, i);
      want.push(predict(i));
    }

    expect(stuff).to.deep.equal(want);

  });

  it("shouldn't mind if params is null", function() {
    function render(ctx, framenum) {}
    var c = new MM.Clip(render, 100);
    c.bindParameters(null, {});
  });

});

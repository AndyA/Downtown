"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");

describe("DynamicProperty", function() {

  var TestClip = MM.util.defineClass(function(name, params) {
    this.name = name

    var defaults = {
      a: 0.1,
      b: 0.1,
      c: 0.2,
      d: 0.3
    }

    this.bindParameters(params, defaults);
  },
  new MM.ClipBase());

  var TestProperty = MM.util.defineClass(function(name, params) {
    this.name = name

    var defaults = {
      p: 0,
      q: 0
    }

    this.bindParameters(params, defaults);

    this.bindProperty('r', function(ctx) {
      return this.p - this.q
    })

    this.bindProperty('s', function(ctx) {
      return this.p + this.q
    })
  },
  new MM.DynamicPropertyBase())

  var tp1 = new TestProperty('tp1', {
    p: function(ctx) {
      return ctx.portion
    },
    q: function(ctx) {
      return 1 - ctx.portion
    },
  })

  var tp2 = new TestProperty('tp2', {
    p: function(ctx) {
      return ctx.portion * 10
    },
    q: function(ctx) {
      return ctx.portion * 20
    },
  })

  var tc1 = new TestClip('tc1', {
    a: tp1.p,
    b: tp1.q,
    c: function() {
      return tp1.r + tp2.r
    },
    d: tp2.s
  })

  var movie = new MM.SequenceClip(tc1)

  it("should update context", function() {
    movie.pushContext({
      portion: 0.25
    })
    try {
      expect(tc1.a).to.equal(0.25)
      expect(tc1.b).to.equal(0.75)
      expect(tc1.c).to.equal(-3)
      expect(tc1.d).to.equal(7.5)
    } finally {
      movie.popContext()
    }
  });

});

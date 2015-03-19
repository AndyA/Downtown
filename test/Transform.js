"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");
var Canvas = require("canvas");

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

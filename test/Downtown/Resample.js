"use strict";

var chai = require("chai");
var expect = chai.expect;

var RS = require("../../lib/Downtown/Resample.js");

function mkArray(len, f) {
  var out = [];
  for (var i = 0; i < len; i++) out.push(f(i));
  return out;
}

function mkRamp(len, m, c) {
  return mkArray(len, function(x) {
    return m * x + c;
  });
}

describe("Resample", function() {

  describe("Basic", function() {
    it("should expose the resample function", function() {

      expect(RS).to.respondTo('resample');

    });

  });

  describe("resample", function() {

    it("should return an array of the correct length", function() {

      var src = mkRamp(100, 0, 10);

      var dst_half = RS.resample(src, 0.5);
      expect(dst_half).to.have.length(src.length / 2);

      var dst_double = RS.resample(src, 2);
      expect(dst_double).to.have.length(src.length * 2);

    });

    it("should always be able to resample to 256", function() {

      for (var len = 1; len < 2000; len++) {

        var src = mkRamp(len, 1, 0);
        var dst = RS.resample(src, 256 / src.length);
        expect(dst).to.have.length(256);
      }

    });

  });

});

"use strict";

var chai = require("chai");
chai.use(require("chai-deep-closeto"));

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

function hasNaN(ar) {
  for (var i = 0; i < ar.length; i++)
  if (isNaN(ar[i])) return true;
  return false;
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

    var cases = [{
      'src': [1.0, 2.0, 3.0, 4.0, 5.0],
      'want': [1.0, 2.0, 3.0, 4.0, 5.0]
    },
    {
      'src': [1.0, 2.0],
      'want': [1.0, 1.5, 2.0]
    },
    {

      'src': [1.0, 2.0, 3.0],
      'want': [1.0, (4.0 / 3.0), 2.0, (8.0 / 3.0), 3.0]
    },
    {

      'src': [1.0, 2.0, 3.0, 4.0],
      'want': [1.5, 3.5]
    },
    {

      'src': [0.0, 0.0, 0.0, 10.0, 0.0, 0.0, 0.0],
      'want': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10.0, 10.0, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, ]
    }];

    for (var i = 0; i < cases.length; i++) {
      (function(tc) {
        it("should correctly resample " + tc.src.join(', '), function() {
          var got = RS.resample(tc.src, tc.want.length / tc.src.length);
          expect(got).to.have.length(tc.want.length);
          expect(got).to.deep.closeTo(tc.want, 0.001);
          expect(hasNaN(got)).to.be.false;
        });
      })(cases[i]);
    }

  });

});

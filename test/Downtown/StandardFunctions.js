"use strict";

var chai = require("chai");
chai.use(require("chai-deep-closeto"));

var expect = chai.expect;

var SF = require("../../lib/Downtown/StandardFunctions.js");

function hasNaN(ar) {
  for (var i = 0; i < ar.length; i++)
  if (isNaN(ar[i])) return true;
  return false;
}

describe("StandardFunctions", function() {

  describe("Basic", function() {
    it("should expose the expected methods", function() {

      expect(SF).to.respondTo('normalise');
      expect(SF).to.respondTo('signature');
      expect(SF).to.respondTo('signatureBits');
      expect(SF).to.respondTo('smooth');
      expect(SF).to.respondTo('smoother');

    });

  });

  describe("normalise", function() {

    it("should handle a scalar", function() {
      var v = SF.normalise(1);
      expect(v).to.be.closeTo(63.434, 0.01);
    });

    it("should handle an array", function() {
      var v = SF.normalise([1, 2, 3]);
      expect(v).to.deep.closeTo([63.434, 47.472, 38.330], 0.01);
    });

  });

  describe("smoother", function() {
    it("should return something that can average", function() {
      var sm = SF.smoother();
      expect(sm).to.respondTo('average');
    });
  });

  describe("smooth", function() {

    it("should return the same number of non-NaNs", function() {
      for (var len = 1; len < 1000; len *= 1.7) {
        var size = Math.floor(len);
        var data_in = [];
        for (var i = 0; i < len; i++) {
          var sample = 15 + Math.sin(i / 2) * 3 + Math.sin(i / 6) * 5.5 + Math.sin(i / 7.1) * 6.3;
          data_in.push(sample);
        }
        var data_out = SF.smooth(data_in);
        expect(data_out).to.have.length(data_in.length);
        expect(hasNaN(data_out)).to.be.false;
      }

    });

  });

  describe("log/exp", function() {

    var lin1 = [];
    for (var i = 0; i < 30; i++) lin1.push(Math.random() + 1);

    var log1 = SF.log(lin1);

    it("should return the same number of data", function() {
      expect(log1).to.have.length(lin1.length);
    });

    var lin2 = SF.exp(log1);

    if ("should roundtrip lin -> log -> lin", function() {
      expect(lin2).to.deep.closeTo(lin1, 0.00001);
    });

  });

  describe("signature", function() {

    var data = [];
    for (var i = 0; i < 700; i++) {
      var sample = 15 + Math.sin(i / 2) * 3 + Math.sin(i / 6) * 3.2 + Math.sin(i / 7.1) * 1.9;
      data.push(sample);
    }

    var sig = SF.signature(data);
    it("should be 256 bits long", function() {
      expect(sig).to.have.length(256);
    });

    it("should look like binary", function() {
      expect(sig).to.match(/^[01]+$/);
    });

  });

});

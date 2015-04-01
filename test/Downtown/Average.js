"use strict";

var chai = require("chai");
var expect = chai.expect;

var Average = require("../../lib/Downtown/Average.js");

function computeAverage(fn, from, to) {
  var total = 0;
  var min = NaN;
  var max = NaN;

  for (var i = from; i <= to; i++) {
    var datum = fn(i);
    if (isNaN(min) || datum < min) min = datum;
    if (isNaN(max) || datum > max) max = datum;
    total += datum;
  }

  return {
    min: min,
    max: max,
    avg: total / (to - from + 1)
  }
}

describe("Average", function() {

  function xn(n) {
    return ((100 - n) * 3.5 * ((n & 1) ? -1.3 : 3.2));
  }

  describe("Basic", function() {

    var a = new Average(5);

    it("should implement add, length & ready", function() {
      expect(Average).to.respondTo('add');
      expect(Average).to.respondTo('length');

      expect(a.length()).to.equal(0);
      expect(a.ready()).to.be.false;

      a.add();
      expect(a.length()).to.equal(0);
      expect(a.ready()).to.be.false;

      a.add(1);
      expect(a.length()).to.equal(1);
      expect(a.ready()).to.be.false;

      a.add(2, 4);
      expect(a.length()).to.equal(3);
      expect(a.ready()).to.be.true;
    });

    it("it should calculate moving average, min & max", function() {

      expect(a.average()).to.equal((1 + 2 + 4) / 3);
      expect(a.min()).to.equal(1);
      expect(a.max()).to.equal(4);

      for (var i = 1; i < 20; i *= 2) {
        for (var j = 0; j < 5; j++) a.add(i);
        expect(a.average()).to.equal(i);
        expect(a.min()).to.equal(i);
        expect(a.max()).to.equal(i);
      }

    });

  });

  describe("Soak test", function() {

    it("should return sensible values", function() {

      for (var len = 1; len < 5; len++) {
        var a = new Average(len);

        for (var x = 0; x < len * 2; x++) {
          var datum = xn(x);
          a.add(datum);
          var got_avg = a.average();
          var got_min = a.min();
          var got_max = a.max();

          var used = Math.min(x + 1, len);
          var want = computeAverage(xn, x - used + 1, x);

          expect(a.length()).to.equal(used);
          expect(got_avg).to.be.closeTo(want.avg, 0.00001);
          expect(got_min).to.be.closeTo(want.min, 0.00001);
          expect(got_max).to.be.closeTo(want.max, 0.00001);
        }
      }
    });
  });

});

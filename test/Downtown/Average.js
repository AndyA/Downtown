"use strict";

var chai = require("chai");
var expect = chai.expect;

var A = require("../../lib/Downtown/Average.js");

describe("Average", function() {

  describe("Basic", function() {

    var a = new A(5);

    it("should implement add, length & ready", function() {
      expect(A).to.respondTo('add');
      expect(A).to.respondTo('length');

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

});

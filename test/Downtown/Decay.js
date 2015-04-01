"use strict";

var chai = require("chai");
var expect = chai.expect;

var Decay = require("../../lib/Downtown/Decay.js");

describe("Decay", function() {
  var decay = new Decay(0.5);

  it("should be zero when empty", function() {
    expect(decay.get()).to.equal(0);
  });

  it("should asymptotically approach a value", function() {
    var prev = decay.get();

    for (var i = 0; i < 10; i++) {
      var next = decay.next(1);
      expect(next).to.be.above(prev);
      expect(next).to.be.below(1);
      prev = next;
    }
  });

});

"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");

describe("CircleProperty", function() {

  it("should have default values", function() {

    var cp = new MM.CircleProperty()
    expect(cp).to.have.property('frequency', 1)
    expect(cp).to.have.property('amplitute', 1)

  });

  it("should have computed values", function() {

    var cp = new MM.CircleProperty()
    cp.pushContext({
      portion: 0
    });
    try {
      expect(cp.evaluateInContext('x', cp.x)).to.be.closeTo(0, 0.000001)
      expect(cp.evaluateInContext('y', cp.y)).to.be.closeTo(1, 0.000001)
    } finally {
      var ctx = cp.popContext();
      console.log(JSON.stringify(ctx, null, 2));
    }

    cp.pushContext({
      portion: 0.25
    });
    try {
      expect(cp.evaluateInContext('x', cp.x)).to.be.closeTo(1, 0.000001)
      expect(cp.evaluateInContext('y', cp.y)).to.be.closeTo(0, 0.000001)
    } finally {
      var ctx = cp.popContext();
      console.log(JSON.stringify(ctx, null, 2));
    }
  });

});

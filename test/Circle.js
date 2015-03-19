"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");

describe("CircleProperty", function() {
  var cp = new MM.CircleProperty()

  it("should be an instance of DynamicPropertyBase", function() {
    expect(cp).to.be.instanceOf(MM.DynamicPropertyBase);
  })

  it("should have default values", function() {

    expect(cp).to.have.property('frequency', 1)
    expect(cp).to.have.property('amplitute', 1)

  });

  it("should have computed values", function() {

    cp.pushContext({
      portion: 0
    });
    try {
      expect(cp.x).to.be.closeTo(0, 0.000001)
      expect(cp.y).to.be.closeTo(1, 0.000001)
    } finally {
      cp.popContext();
    }

    cp.pushContext({
      portion: 0.25
    });
    try {
      expect(cp.x).to.be.closeTo(1, 0.000001)
      expect(cp.y).to.be.closeTo(0, 0.000001)
    } finally {
      cp.popContext();
    }
  });

});

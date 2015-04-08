"use strict";

var chai = require("chai");
chai.use(require("chai-deep-closeto"));

var expect = chai.expect;

var Profile = require("../../lib/Downtown/Profile.js");

describe("Profile", function() {
  var prof = new Profile({
    width: 256,
    height: 256,
    sampler: 'spiral:a_rate=5.062,r_rate=5.062',
    baseline: [20, 10, 5]
  });

  it("should return the image size", function() {
    expect(prof.getSize()).to.deep.equal({
      width: 256,
      height: 256
    });
  });

  it("should resample to the signature length", function() {
    var rs = prof.resample([1, 2, 3, 4]);
    expect(rs).to.have.length(Profile.SIGNATURE_BITS);
  });
});

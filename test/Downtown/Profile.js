"use strict";

var fs = require('fs');
var chai = require("chai");
chai.use(require("chai-deep-closeto"));

var expect = chai.expect;

var Profile = require("../../lib/Downtown/Profile.js");

var MAX_DIFFS = 5;

function ndiff(a, b) {
  var diffs = 0;
  for (var i = 0; i < a.length && i < b.length; i++) if (a[i] !== b[i]) diffs++;
  return diffs + (a.length - i) + (b.length - i);
}

function diffStr(a, b) {
  var out = [];
  for (var i = 0; i < a.length && i < b.length; i++) out.push(a[i] === b[i] ? ' ' : '^');
  for (; i < a.length; i++) out.push('^');
  for (; i < b.length; i++) out.push('^');
  return out.join('');
}

describe("Profile", function() {
  var prof = new Profile({
    width: 256,
    height: 256,
    sampler: 'spiral:a_rate=5.062,r_rate=5.062',
    baseline: [20, 10, 5]
  });

  describe("Basic", function() {

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

  describe("Reference", function() {
    var prof = Profile.load(__dirname + '/data/default.profile');
    var ref = JSON.parse(fs.readFileSync(__dirname + '/data/sig.json', 'utf8'));

    it("should produce the expected sigs from reference data", function() {
      for (var f = 0; f < ref.length; f++) {
        var want = ref[f].signature;
        var got = prof.signature(ref[f].planes[0]);
        var diffs = ndiff(want, got);
        if (diffs >= MAX_DIFFS) {
          console.log("wanted: " + want);
          console.log("   got: " + got);
          console.log("        " + diffStr(want, got));
        }
        expect(diffs).to.be.lessThan(MAX_DIFFS);
      }
    });
  });
});

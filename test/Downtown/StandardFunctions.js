"use strict";

var chai = require("chai");
var expect = chai.expect;

var SF = require("../../lib/Downtown/StandardFunctions.js");

describe("StandardFunctions", function() {

  describe("Basic", function() {
    it("should expose the expected methods", function() {

      expect(SF).to.respondTo('normalise');
      expect(SF).to.respondTo('smoother');
      expect(SF).to.respondTo('smooth');

    });

  });

  describe("smooth", function() {
    it("should return something that can average", function() {
      var sm = SF.smoother();
      expect(sm).to.respondTo('average');
    });
  });

  describe("smoother", function() {

    it("should return the same number of data", function() {
      for (var len = 1; len < 1000; len *= 1.7) {
        var size = Math.floor(len);
        var data_in = [];
        for (var i = 0; i < len; i++) data_in.push(Math.random());
        var data_out = SF.smooth(data_in);
        expect(data_out).to.have.length(data_in.length);
      }

    });

  });

});

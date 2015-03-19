"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");

describe("SequenceClip", function() {

  var doNothingMixin = {
    render: function(canvas, ctx, framenum) {},
    getFrames: function() {
      return null
    }
  }

  var TestClip = MM.util.defineClass(function(tag) {
    this.tag = tag
  },
  new MM.ClipBase(), doNothingMixin, {
    getTag: function() {
      return this.tag
    }
  });

  function sequenceTags(seq) {
    var clips = seq.getClips();
    var tags = [];
    for (var i = 0; i < clips.length; i++) {
      var clip = clips[i];
      tags.push(clip.getTag ? clip.getTag() : "unknown " + i);
    }
    return tags;
  }

  describe("constructor", function() {

    it("should handle simple clips", function() {
      var seq = new MM.SequenceClip(new TestClip("clip 1"));
      expect(seq.getClips()).to.have.property('length', 1);
      var tags = sequenceTags(seq);
      expect(tags).to.deep.equal(["clip 1"]);
    });

    it("should handle arrays", function() {
      var seq = new MM.SequenceClip( //
      [new TestClip("clip 1")], //
      [new TestClip("clip 2"), new TestClip("clip 3")], //
      [new TestClip("clip 4"), new TestClip("clip 5"), new TestClip("clip 6")]);
      expect(seq.getClips()).to.have.property('length', 6);
      var tags = sequenceTags(seq);
      expect(tags).to.deep.equal(["clip 1", "clip 2", "clip 3", "clip 4", "clip 5", "clip 6"]);
    });

  });

  describe("append", function() {

    it("should handle simple clips", function() {

      var seq = new MM.SequenceClip();

      expect(seq.getClips()).to.have.property('length', 0);
      seq.append(new TestClip("clip 1"));
      expect(seq.getClips()).to.have.property('length', 1);
      var tags = sequenceTags(seq);
      expect(tags).to.deep.equal(["clip 1"]);
    });

    it("should handle arrays", function() {

      var seq = new MM.SequenceClip();

      expect(seq.getClips()).to.have.property('length', 0);
      seq.append( //
      [new TestClip("clip 1")], //
      [new TestClip("clip 2"), new TestClip("clip 3")], //
      [new TestClip("clip 4"), new TestClip("clip 5"), new TestClip("clip 6")]);
      expect(seq.getClips()).to.have.property('length', 6);
      var tags = sequenceTags(seq);
      expect(tags).to.deep.equal(["clip 1", "clip 2", "clip 3", "clip 4", "clip 5", "clip 6"]);

    });

  });

  describe("map", function() {
    var seq = new MM.SequenceClip([new TestClip("clip 1")], [new TestClip("clip 2"), new TestClip("clip 3")]);
    expect(sequenceTags(seq)).to.deep.equal(["clip 1", "clip 2", "clip 3"]);

    it("should be a shallow copy with a passthru func", function() {
      var nseq = seq.map(function(clip) {
        return clip
      });
      expect(sequenceTags(nseq)).to.deep.equal(["clip 1", "clip 2", "clip 3"]);
    });

    it("should allow 1:1 transormation", function() {
      var nseq = seq.map(function(clip) {
        return new TestClip("new " + clip.getTag());
      });
      expect(sequenceTags(nseq)).to.deep.equal(["new clip 1", "new clip 2", "new clip 3"]);
    });

    it("should allow deletion", function() {
      var count = 0;
      var nseq = seq.map(function(clip) {
        if (++count & 1) return null;
        return new TestClip("new " + clip.getTag());
      });
      expect(sequenceTags(nseq)).to.deep.equal(["new clip 2"]);
    });

    it("should allow multiple return clips", function() {
      var nseq = seq.map(function(clip) {
        return[new TestClip("new " + clip.getTag()), clip];
      });
      expect(sequenceTags(nseq)).to.deep.equal(["new clip 1", "clip 1", "new clip 2", "clip 2", "new clip 3", "clip 3"]);
    });

  });

  describe("reduce", function() {

  });

});

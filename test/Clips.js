"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");

(function() {
  var clip_types = {
    "BlankClip": MM.BlankClip,
    "DisolveClip": MM.DisolveClip,
    "EditClip": MM.EditClip,
    "OverlayClip": MM.OverlayClip,
    "ReverseClip": MM.ReverseClip,
    "SequenceClip": MM.SequenceClip,
    "TimecodeClip": MM.TimecodeClip,
    "TitleClip": MM.TitleClip,
    "TransformClip": MM.TransformClip,
  };

  var clip_names = Object.keys(clip_types);

  for (var i = 0; i < clip_names.length; i++) {
    var name = clip_names[i];
    var klass = clip_types[name];

    describe(name, function() {

      it("supports standard methods", function() {
        var clip = new klass();
        expect(clip).to.respondTo('render').and.respondTo('getFrames');
      });
    });

  }
})()

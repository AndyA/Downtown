"use strict";

var MM = require('../lib/MovieMaker.js');

function animateClip(name, params) {
  var target = new MM.TitleClip(name, 100, 0.5, 0.5);
  return new MM.TransformClip(target, params);
}

var WIDTH = 1920;
var HEIGHT = 1080;

var movie = new MM.SequenceClip(new MM.TitleClip("Transform Test", 50, 0.5, 0.5));

movie.append(animateClip("translate_x", {
  translate_x: new MM.RampProperty(-WIDTH / 4, WIDTH / 4)
}));

movie.append(animateClip("translate_y", {
  translate_y: new MM.RampProperty(-HEIGHT / 4, HEIGHT / 4)
}));

movie.append(animateClip("scale_x", {
  scale_x: new MM.RampProperty(0.5, 1.5)
}));

movie.append(animateClip("scale_y", {
  scale_y: new MM.RampProperty(0.5, 1.5)
}));

movie.append(animateClip("rotate", {
  rotate: new MM.RampProperty(0, 2 * Math.PI)
}));

new MM.MovieMaker(__dirname + '/../transform.mjpeg', 1920, 1080, movie).render();

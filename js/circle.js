"use strict";

var MM = require('../lib/MovieMaker.js');

var WIDTH = 1920;
var HEIGHT = 1080;

var RADIUS = Math.min(WIDTH, HEIGHT)

var wibbler = new MM.CircleProperty({
  frequency: 7
});

var circle = new MM.CircleProperty({
  amplitute: RADIUS / 4,
  offset_x: 0.5,
  offset_y: 0.5,
  scale_x: wibbler.x
});

var title = new MM.TitleClip("Circle", 500, 0.5, 0.5);

var transform = new MM.TransformClip(title, {
  translate_x: circle.x,
  translate_y: circle.y
});

var movie = new MM.SequenceClip(transform);

new MM.MovieMaker(__dirname + '/../circle.mjpeg', 1920, 1080, movie).render();

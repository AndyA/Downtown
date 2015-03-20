"use strict";

var MM = require('../lib/MovieMaker.js');

var WIDTH = 1920;
var HEIGHT = 1080;

var RADIUS = Math.min(WIDTH, HEIGHT)

var wibbler = new MM.CircleProperty({
  frequency: 7,
  amplitute: RADIUS / 10
});

var circle = new MM.CircleProperty({
  amplitute: function() {
    return RADIUS / 4 + wibbler.x
  },
  offset_x: 0.5,
  offset_y: 0.5
});

var title = new MM.TitleClip("Circle", 500, 0.5, 0.5);

var transform = new MM.TransformClip(title, {
  translate_x: circle.x,
  translate_y: circle.y
});

var movie = new MM.SequenceClip(transform);

new MM.MovieMaker(__dirname + '/../circle.mjpeg', 1920, 1080, movie).render();

"use strict";

var MM = require('../lib/MovieMaker.js');

var WIDTH = 1920;
var HEIGHT = 1080;

var RADIUS = Math.min(WIDTH, HEIGHT)

var wibbler = new MM.CircleProperty({
  frequency: 13,
  amplitute: 0.1
});

var circle = new MM.CircleProperty({
  frequency: 2,
  amplitute: function() {
    return 0.25 + wibbler.x
  },
  offset_x: 0.5,
  offset_y: 0.5
});

var title = new MM.TextClip(500, {
  text: function(ctx) {
    return this.framenum
  },
  x: circle.x,
  y: circle.y
})

var movie = new MM.SequenceClip(title);

new MM.MovieMaker(__dirname + '/../circle.mjpeg', 1920, 1080, movie).render();

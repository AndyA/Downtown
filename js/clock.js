"use strict";

var MM = require('../lib/MovieMaker.js');

function drawClock(ctx, tm) {

  var canvas = ctx.canvas;
  ctx.translate(canvas.width / 2, canvas.height / 2);
  var size = Math.min(canvas.width, canvas.height) * 0.48;

  var baseWidth = size / 50;

  ctx.lineCap = 'square';
  ctx.lineJoin = 'square';
  ctx.strokeStyle = 'rgb(255,255,255)';

  // Face
  for (var tick = 0; tick < 60; tick++) {
    var x = Math.sin(tick * Math.PI * 2 / 60);
    var y = Math.cos(tick * Math.PI * 2 / 60);
    ctx.beginPath();
    ctx.moveTo(x * size, y * size);

    if (tick % 5) {
      ctx.lineWidth = baseWidth;
      ctx.lineTo(x * (size - baseWidth * 3), y * (size - baseWidth * 3));
    } else {
      ctx.lineWidth = baseWidth * 2;
      ctx.lineTo(x * (size - baseWidth * 5), y * (size - baseWidth * 5));
    }
    ctx.stroke();
  }

  // Hands
  var hour = tm / 60 / 60;
  var min = tm / 60 % 60;

  ctx.lineWidth = baseWidth * 5;
  var xh = Math.sin(hour * Math.PI * 2 / 12);
  var yh = Math.cos(hour * Math.PI * 2 / 12);
  ctx.beginPath();
  ctx.moveTo(-(xh * baseWidth * 5), yh * baseWidth * 5);
  ctx.lineTo(xh * baseWidth * 30, -(yh * baseWidth * 30));
  ctx.stroke();

  ctx.lineWidth = baseWidth * 2.5;
  var xm = Math.sin(min * Math.PI * 2 / 60);
  var ym = Math.cos(min * Math.PI * 2 / 60);
  ctx.beginPath();
  ctx.moveTo(-(xm * baseWidth * 7), ym * baseWidth * 7);
  ctx.lineTo(xm * baseWidth * 43, -(ym * baseWidth * 43));
  ctx.stroke();

}

function clock(ctx, framenum) {
  var tm = 12 * 60 * 60 * this.portion;
  drawClock(ctx, tm);
}

var mm = new MM.MovieMaker(__dirname + '/../clock.mjpeg', 400, 400, new MM.Clip(clock, 12 * 60 * 60 * 25));
mm.render();

var MM = require('../lib/MovieMaker.js');

function drawSpiral(ctx, limit, r, a, r_rate, a_rate) {
  ctx.beginPath();
  var point = 0;

  while (a < limit) {
    var px = Math.sin(a) * r;
    var py = Math.cos(a) * r;

    //    console.log("px=" + px + ", py=" + py);
    if (point) ctx.lineTo(px, py);
    else ctx.moveTo(px, py);

    a += a_rate / r;
    r += r_rate / r;
    point++;
  }
  ctx.stroke();
}

function drawSpirals(canvas, ctx, limit, shift) {
  ctx.save();
  // ctx.scale() seems to cause drawing to stop
  ctx.translate(canvas.width / 2, canvas.height / 2);
  ctx.lineWidth = 5;
  ctx.lineCap = 'round';
  ctx.lineJoin = 'round';

  var phase = ['rgb(180, 0, 0)', 'rgb(0, 180, 0)', 'rgb(80, 80, 200)'];

  for (var i = 0; i < phase.length; i++) {
    //    console.log("phase " + i + ", shift " + shift);
    ctx.strokeStyle = phase[i];
    drawSpiral(ctx, limit + shift, 5, shift, 20, 3);
    shift += Math.PI * 2 / phase.length;
  }

  ctx.restore();
}

var spiral_limit = 100;

function spiral(canvas, ctx, framenum) {
  drawSpirals(canvas, ctx, framenum, 0);
}

function spinner(canvas, ctx, framenum) {
  var angle = (framenum * framenum) / 100;
  drawSpirals(canvas, ctx, spiral_limit, angle);
}

function fadeInOut(clip, in_frames, out_frames, in_pad, out_pad) {
  var in_blank = new MM.BlankClip(in_frames + in_pad);
  var out_blank = new MM.BlankClip(out_frames + out_pad);
  return new MM.DisolveClip(new MM.DisolveClip(in_blank, clip, in_frames), out_blank, out_frames);
}

function mediumText(ctx) {
  ctx.font = "36px sans-serif";
}

function makeWibbler(origin, freq, amp) {
  return function(framenum, portion) {
    return origin + Math.sin(framenum * freq) * amp;
  }
}

var title = new MM.TitleClip("Javascript Video Generator", 200, 0.5, 0.5);
var credit = new MM.OverlayClip(
new MM.TitleClip("Code: https://github.com/AndyA/Downtown/tree/master/js", 300, makeWibbler(0.5, 0.05, 0.05), 0.3, mediumText), //
new MM.TitleClip("Andy Armstong, andy@hexten.net", 300, 0.5, 0.8, mediumText) //
);

var spiral_clip = new MM.Clip(spiral, spiral_limit);
var spinner_clip = new MM.Clip(spinner, 200);

var wibble = new MM.SequenceClip(spiral_clip, spinner_clip, new MM.ReverseClip(spiral_clip));

var movie = new MM.SequenceClip(fadeInOut(title, 50, 50, 25, 25), wibble, fadeInOut(credit, 50, 100, 25, 25));

//console.log(JSON.stringify(movie));
//movie = new MM.OverlayClip(movie, new MM.TimecodeClip());
var mm = new MM.MovieMaker(__dirname + '/../state.mjpeg', 1920, 1080, movie);

mm.render();

var Canvas = require('canvas');
var fs = require('fs');

function makeMovie(name, width, height, renderFrame) {

  var canvas = new Canvas(width, height);
  var ctx = canvas.getContext('2d');
  var out = fs.createWriteStream(name);

  function drawFrame(framenum) {
    console.log("Rendering frame " + framenum);
    ctx.clearRect(0, 0, width, height);
    ctx.save();
    var more = renderFrame(canvas, ctx, framenum);
    ctx.restore();

    var stream = canvas.createJPEGStream();

    stream.on('data', function(chunk) {
      out.write(chunk);
    });

    stream.on('end', function() {
      if (more) drawFrame(framenum + 1);
    });
  }

  drawFrame(0);
}

function makeSequencer(scenes) {
  var scene = 0;
  var scene_frame = 0;
  return function(canvas, ctx, framenum) {
    var more = scenes[scene](canvas, ctx, scene_frame++);
    if (!more) scene++;
    return scene < scenes.length;
  }
}

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

function spiral(canvas, ctx, framenum) {
  ctx.save();
  // ctx.scale() seems to cause drawing to stop
  ctx.translate(canvas.width / 2, canvas.height / 2);
  ctx.lineWidth = 5;
  ctx.lineCap = 'round';
  ctx.lineJoin = 'round';

  var phase = ['rgb(180, 0, 0)', 'rgb(0, 180, 0)', 'rgb(80, 80, 200)'];
  var shift = 0;

  for (var i = 0; i < phase.length; i++) {
    console.log("phase " + i + ", shift " + shift);
    ctx.strokeStyle = phase[i];
    drawSpiral(ctx, framenum + shift, 5, shift, 20, 3);
    shift += Math.PI * 2 / phase.length;
  }

  ctx.restore();

  return framenum < 200;
}

makeMovie(__dirname + '/../state.mjpeg', 1920, 1080, spiral);

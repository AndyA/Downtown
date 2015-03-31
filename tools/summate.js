"use strict";

var LineByLineReader = require('line-by-line');

function zipop(a, b, op) {
  var out = [];

  if (b instanceof Array) {
    if (! (a instanceof Array) || a.length != b.length) {
      throw new Error("Shape mismatch (" + JSON.stringify(a) + ' v ' + JSON.stringify(b) + ')');
    }
    for (var i = 0; i < a.length; i++) out.push(zipop(a[i], b[i], op));
  } else {
    if (! (a instanceof Array)) return op(a, b);
    for (var i = 0; i < a.length; i++) out.push(zipop(a[i], b, op));
  }

  return out;
}

function readList(list, done) {
  var total = null;
  var frames = 0;

  function readFile(file, done) {
    console.log('>>> ' + file);
    var lr = new LineByLineReader(file);
    lr.on('error', function(err) {
      console.log(err);
      process.exit(1);
    });

    lr.on('line', function(line) {
      var data = JSON.parse(line);
      var planes = data.planes;
      if (total === null) {
        total = planes;
      } else {
        total = zipop(total, planes, function(a, b) {
          return a + b
        })
      }
      frames++;
    });

    lr.on('end', function() {
      done();
    });
  }

  function next() {
    if (list.length) readFile(list.shift(), next)
    else done(total, frames);
  }

  next();
}

var argv = process.argv.slice(2);
readList(argv, function(total, frames) {
  var average = zipop(total, frames, function(a, b) {
    return a / b
  });
  console.log(JSON.stringify(average));
});

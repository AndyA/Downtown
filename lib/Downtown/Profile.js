"use strict";

function Profile(data) {
  this.data = data;
}

Profile.SIGNATURE_BITS = 256;

var p = Profile.prototype = new Object();

p.getSize = function() {
  return {
    width: this.data.width,
    height: this.data.height
  };
}

p.resample = function(norm) {
  return require('./Resample.js').resample(norm, Profile.SIGNATURE_BITS / norm.length);
}

p.normalise = function(sample) {
  var baseline = this.data.baseline[0];
  if (baseline.length != sample.length) {
    throw new Error("Sample length (" + sample.length + ") doesn't match baseline length (" + baseline.length + ")");
  }

  var norm = [];
  for (var i = 0; i < sample.length; i++) norm.push(sample[i] / baseline[i]);
  return norm;
}

p.smoother = function() {
  var span = this.data.smooth;
  if (!span || span == 1) return null;

  var A = require('./Average.js');
  var a = new A(span);
  a.logarithmic();
  return a;
}

p.smooth = function(data) {
  var sm = this.smoother();
  if (!sm) return null;

  var out = [];

  for (var i = 0; i < data.length; i++) {
    if (sm.ready()) out.push(sm.average());
    sm.add(data[i]);
  }

  while (out.length != data.length) {
    out.push(sm.average());
    sm.remove();
  }

  return out;
}

function zipop(a, b, op) {
  var out = [];

  if (a instanceof Array) {
    if (b instanceof Array) {
      if (a.length != b.length) throw new Error("Array size mismatch");
      for (var i = 0; i < a.length; i++) out.push(zipop(a[i], b[i], op));
      return out;
    }

    for (var i = 0; i < a.length; i++) out.push(zipop(a[i], b, op));
    return out;
  }

  if (b instanceof Array) {
    for (var i = 0; i < b.length; i++) out.push(zipop(a, b[i], op));
    return out;
  }

  return op(a, b);
}

p.signature = function(sample) {
  var norm = this.normalise(sample);

  var smoothed = this.smooth(norm);
  if (!smoothed) smoothed = 1;

  var sig_raw = zipop(norm, smoothed, function(a, b) {
    return a - b
  });

  var sig_data = this.resample(sig_raw);
  var sig = [];
  for (var i = 0; i < sig_data.length; i++) sig.push(sig_data[i] > 0 ? '1' : '0');
  return sig.join('');
}

module.exports = Profile;

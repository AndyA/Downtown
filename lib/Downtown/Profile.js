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

p.resample = function(delta) {
  return require('./Resample.js').resample(delta, Profile.SIGNATURE_BITS / delta.length);
}

p.signature = function(sample) {
  var baseline = this.data.baseline;
  if (baseline.length != sample.length) {
    throw new Error("Sample length (" + sample.length + ") doesn't match baseline length (" + baseline.length + ")");
  }

  var delta = [];

  for (var i = 0; i < sample.length; i++) delta.push(sample[i] / baseline[i]);

  var sig_data = this.resample(delta);
  var sig = [];

  for (var i = 0; i < sig_data.length; i++) sig.push(sig_data[i] > 1 ? '1' : '0');
  return sig.join('');
}

module.exports = Profile;

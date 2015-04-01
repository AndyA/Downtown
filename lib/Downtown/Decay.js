"use strict";

function Decay(rate) {
  this.value = 0;
  this.rate = rate;
  this.scale = 1;
}

var p = Decay.prototype = new Object();

p.set = function(v, s) {
  this.value = v;
  this.scale = arguments.length > 1 ? s : 1;
  return this;
}

p.get = function() {
  return this.value / this.scale;
}

p.next = function(x) {
  this.value = this.value * this.rate + x;
  this.scale = this.scale * this.rate + 1;
  return this.value / this.scale;
}

module.exports = Decay;

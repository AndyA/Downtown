"use strict";

function Decay(rate, value) {
  this.value = value || 0;
  this.rate = rate;
  this.scale = 1;
}

var p = Decay.prototype = new Object();

p.get = function() {
  return this.value / this.scale;
}

p.next = function(x) {
  this.value = this.value * this.rate + x;
  this.scale = this.scale * this.rate + 1;
  return this.value / this.scale;
}

module.exports = Decay;

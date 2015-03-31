"use strict";

// n-point moving average
function Average(npoints) {
  this.size = npoints;
  this.data = [];
  this.total = 0;
  this.f_in = function(x) {
    return x
  }
  this.f_out = function(x) {
    return x
  }
}

var p = Average.prototype = new Object();

p.remove = function() {
  if (this.data.length) this.total -= this.data.shift();
}

p.add = function() {
  for (var i = 0; i < arguments.length; i++) {
    if (this.data.length == this.size) this.remove();
    this.total += arguments[i];
    this.data.push(arguments[i]);
  }
}

p.length = function() {
  return this.data.length;
}

p.ready = function() {
  return this.data.length >= this.size / 2;
}

p.average = function() {
  return this.total / this.data.length;
}

p.min = function() {
  return Math.min.apply(null, this.data);
}

p.max = function() {
  return Math.max.apply(null, this.data);
}

module.exports = Average;

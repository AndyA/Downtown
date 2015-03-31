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

p.logarithmic = function() {
  this.f_in = function(x) {
    return Math.log(x)
  }
  this.f_out = function(x) {
    return Math.exp(x)
  }
}

p.remove = function() {
  if (this.data.length) this.total -= this.data.shift();
}

p.add = function() {
  for (var i = 0; i < arguments.length; i++) {
    if (this.data.length == this.size) this.remove();
    var datum = this.f_in(arguments[i]);
    this.total += datum;
    this.data.push(datum);
  }
}

p.length = function() {
  return this.data.length;
}

p.ready = function() {
  return this.data.length >= this.size / 2;
}

p.average = function() {
  return this.f_out(this.total / this.data.length);
}

p.min = function() {
  return this.f_out(Math.min.apply(null, this.data));
}

p.max = function() {
  return this.f_out(Math.max.apply(null, this.data));
}

module.exports = Average;

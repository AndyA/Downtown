"use strict";

var PLP = {
  c: 4.15,
  m: -0.45,
  p: 1.2
};

var AVERAGE_SPAN = 15;
var SIGNATURE_BITS = 256;

function deepMap(x, f) {
  if (x instanceof Array) {
    var out = [];
    for (var i = 0; i < x.length; i++) out.push(deepMap(x[i], f));
    return out;
  }
  return f(x);
}

function autoMap(f) {
  return function(x) {
    return deepMap(x, f);
  }
}

function num(x) {
  if (x instanceof Array) {
    var out = [];
    for (var i = 0; i < x.length; i++) out.push(num(x[i]));
    return out.join(', ');
  }
  return Math.floor(x * 1000) / 1000;
}

var p = {
  normalise: autoMap(function(x) {
    return Math.exp(PLP.m * Math.pow(Math.log(x), PLP.p) + PLP.c);
  }),

  log: autoMap(Math.log),
  exp: autoMap(Math.exp),

  signatureBits: function() {
    return SIGNATURE_BITS
  },

  scaleLength: function(a) {
    return require('./Resample.js').resample(a, p.signatureBits() / a.length);
  },

  signature: function(data) {
    var scaled = p.scaleLength(data);
    var smoothed = p.smooth(scaled);

    var bits = [];
    for (var i = 0; i < scaled.length; i++) {
      bits.push(scaled[i] > smoothed[i] ? 1 : 0);
    }

    return bits.join('');
  },

  smoother: function() {
    var A = require('./Average.js');
    var a = new A(AVERAGE_SPAN);
    a.logarithmic();
    return a;
  },

  smooth: function(data) {
    var out = [];
    var sm = p.smoother();

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
}

module.exports = p;

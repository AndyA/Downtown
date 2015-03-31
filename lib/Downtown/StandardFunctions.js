"use strict";

var PLP = {
  c: 4.15,
  m: -0.45,
  p: 1.2
};

var AVERAGE_SPAN = 35;

var p = {
  normalise: function(x) {
    return Math.exp(PLP.m * Math.pow(Math.log(x), PLP.p) + PLP.c);
  },

  smoother: function() {
    var A = require('./Average.js');
    return new A(AVERAGE_SPAN);
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

"use strict";

var ColourTools = require('../ColourTools.js');
var Decay = require('./Decay.js');

function heatColour(v, a) {
  var vv = Math.max(0, Math.min(v, 1));
  var rgb = ColourTools.hsvToRgb((1 - vv), 1, vv);
  return 'rgba(' + Math.floor(rgb[0]) + ',' + Math.floor(rgb[1]) + ',' + Math.floor(rgb[2]) + ',' + a + ')';
}

function BitBox(size, decay) {
  this.init(size, decay);
}

var p = BitBox.prototype = new Object();

p.init = function(size, decay) {
  this.ww = Math.floor(Math.sqrt(size));
  this.wh = Math.floor((size + this.ww - 1) / this.ww);

  this.prev = [];
  this.dec = [];

  for (var i = 0; i < size; i++) {
    this.dec.push(new Decay(decay, 1));
    this.prev.push(null);
  }
}

p.renderHeatChip = function(ctx, x, y, w, h) {
  var ww = h / 2;
  for (var xx = 0; xx < w; xx += ww) {
    var v = xx / w;
    ctx.fillStyle = heatColour(v, 0.9);
    ctx.fillRect(x + xx, y, ww - 2, h - 2);
  }
}

p.render = function(ctx, bits, x, y, w, h) {
  var tw = w / this.ww;
  var th = h / this.wh;

  for (var yy = 0; yy < this.ww; yy++) {
    for (var xx = 0; xx < this.wh; xx++) {
      var pos = xx + yy * this.ww;
      if (pos < bits.length) {
        var bit = bits[pos] * 1;
        var dec = this.dec[pos];
        var changed = (this.prev[pos] !== null && this.prev[pos] != bit) ? 1 : 0;
        if (changed) {
          for (var i = 0; i < 10; i++) dec.next(1);
        }
        else {
          dec.next(0);
        }

        ctx.fillStyle = heatColour(dec.get(), 0.9);
        if (bit) ctx.fillRect(x + xx * tw, y + yy * th, tw - 2, th - 2);

        this.prev[pos] = bit;
      }
    }
  }
}

module.exports = BitBox;

"use strict";

var ColourTools = require('../ColourTools.js');
var Decay = require('./Decay.js');

function heatColour(v, a) {
  var vv = Math.max(0, Math.min(v, 1));
  var rgb = ColourTools.hsvToRgb((1 - vv), 1, vv);
  return 'rgba(' + Math.floor(rgb[0]) + ',' + Math.floor(rgb[1]) + ',' + Math.floor(rgb[2]) + ',' + a + ')';
}

function heatGrey(v, a) {
  var vv = Math.max(0, Math.min(v * 0.75 + 0.25, 1)) * 255;
  return 'rgba(' + Math.floor(vv) + ',' + Math.floor(vv) + ',' + Math.floor(vv) + ',' + a + ')';
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
    var d = new Decay(decay);
    d.set(0, 5);
    this.dec.push(d);
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

        if (changed) dec.set(0, 5);
        else dec.next(1);

        var v = dec.get();
        var vv = Math.max(0, Math.min(v * 0.6 + 0.4, 1));
        ctx.fillStyle = 'rgba(' + (bit ? '200, 200, 200' : '230, 20, 40') + ', ' + vv + ')';
        ctx.fillRect(x + xx * tw, y + yy * th, tw - 2, th - 2);

        this.prev[pos] = bit;
      }
    }
  }
}

module.exports = BitBox;

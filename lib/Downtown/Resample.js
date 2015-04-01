"use strict";

var p = {
  resample: function(src, scale) {
    if (scale == 1.0) return src;
    var dst = [];
    var isize = src.length;
    var osize = src.length * scale;

    for (var i = 0; i < osize; i++) {
      var is = i * scale;
      var ie = is + scale;

      var iis = Math.floor(is);
      var iie = Math.floor(ie);

      var sum;
      if (iis == iie) {
        sum = src[iis] * scale;
      }
      else {
        sum = src[iis] * (1 - (is - iis));
        iis++;
        while (iis < iie) sum += src[iis++];
        sum += src[iis] * (ie - iie);
      }
      dst.push(sum / scale);
    }
    return dst;
  }
}

module.exports = p;

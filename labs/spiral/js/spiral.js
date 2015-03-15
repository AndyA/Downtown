function DuploPlotter($elt, scale) {
  this.$elt = $elt;
  this.scale = scale;
  this.path = null;
  this.init();
}

$.extend(DuploPlotter.prototype, (function() {
  return {
    init: function() {
      var elt = this.$elt[0];
      var ctx = elt.getContext('2d');

      ctx.lineWidth = 0.5;
      ctx.lineCap = 'round';
      ctx.lineJoin = 'round';
      ctx.strokeStyle = 'rgb(180, 0, 0)';

      // transform
      ctx.scale(this.scale, this.scale);
      ctx.translate(0.5, 0.5);
      this.ctx = ctx;
    },

    getSize: function() {
      var elt = this.$elt[0];
      return {
        w: Math.floor(elt.width / this.scale),
        h: Math.floor(elt.height / this.scale)
      };
    },

    move: function(x, y) {
      this.flush();
      this.path = new Path2D();
      this.path.moveTo(x, y);
    },

    draw: function(x, y) {
      if (!this.path) {
        this.move(x, y);
        return;
      }
      this.path.lineTo(x, y);
    },

    flush: function() {
      if (this.path) {
        this.ctx.stroke(this.path);
        this.path = null;
      }
    }
  }
})());

$(function() {

  function spiral(dp) {
    var sz = dp.getSize();

    var cx = sz.w / 2;
    var cy = sz.h / 2;
    var r = 1;
    var a = 0;

    for (var i = 0;; i++) {
      var px = cx + Math.sin(a) * r;
      var py = cy + Math.cos(a) * r;
      if (px < 0 || px >= sz.w || py < 0 || py >= sz.h) break;
      dp.draw(px, py);
      a += 2 / r;
      r += 0.2 / r;
    }

    dp.flush();
  }

  var $canvas = $('#spiral');
  var ctx = $canvas[0].getContext('2d');

  var dp = new DuploPlotter($canvas, 10);
  spiral(dp);

});

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
      if (!this.path) return this.move(x, y);
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

  function zigzag(dp) {
    var sz = dp.getSize();

    console.log('ZigZag!');
    dp.move(0, 0);
    for (var x = 1; x < sz.w + sz.h - 1; x++) {
      var x0 = Math.min(sz.w - 1, x);
      var y0 = x - x0;

      var y1 = Math.min(sz.h - 1, x);
      var x1 = x - y1;

      console.log('x0=' + x0 + ', y0=' + y0 + ', x1=' + x1 + ', y1=' + y1);

      if (x & 1) {
        dp.draw(x0, y0);
        dp.draw(x1, y1);
      }
      else {
        dp.draw(x1, y1);
        dp.draw(x0, y0);
      }
    }

    dp.flush();
  }

  var $canvas = $('#zigzag');
  var ctx = $canvas[0].getContext('2d');

  var dp = new DuploPlotter($canvas, 20);
  zigzag(dp);

});

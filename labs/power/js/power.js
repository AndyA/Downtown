$(function() {

  function makeScaler(imin, imax, omin, omax) {
    return function(x) {
      return (x - imin) * (omax - omin) / (imax - imin) + omin;
    }
  }

  function plotFunction(ctx, f, xmin, xmax, xs, ys) {
    ctx.beginPath();
    for (var x = xmin; x <= xmax; x++) {
      var px = xs(x);
      var py = ys(f(x));
      if (x == 0) ctx.moveTo(px, py);
      else ctx.lineTo(px, py);
    }
    ctx.stroke();
  }

  function makeControls(elt, series, xs, ys, parms) {
    var canvas = elt.find('.plot')[0];
    var ctx = canvas.getContext('2d');

    function update() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      ctx.lineCap = 'round';
      ctx.lineJoin = 'round';
      ctx.lineWidth = 2;

      var ys2 = makeScaler(0, MAX, 0, 1);
      var sys = function(y) {
        return ys(ys2(y))
      }

      function get_datum(x) {
        return series[x]
      }

      function get_func(x) {
        return Math.exp(parms.m * Math.pow(Math.log(x), parms.p) + parms.c);
      }

      function get_norm(x) {
        var d = get_datum(x);
        var f = get_func(x);
        return d / f / 2;
      }

      ctx.strokeStyle = 'rgb(255, 0, 0)';
      plotFunction(ctx, get_datum, 0, series.length - 1, xs, sys);

      ctx.strokeStyle = 'rgb(0, 255, 0)';
      plotFunction(ctx, get_func, 0, series.length - 1, xs, sys);

      ctx.strokeStyle = 'rgb(230, 210, 0)';
      plotFunction(ctx, get_norm, 0, series.length - 1, xs, ys);
    }

    function makeForm() {
      var form = $('<form></form>');
      var pn = Object.keys(parms).sort();
      for (var i = 0; i < pn.length; i++) {
        form.append($('<label></label>').text(pn[i]).append($('<input></input>').attr({
          name: pn[i],
          value: parms[pn[i]]
        })));
      }
      form.append($('<input type="submit" value="update"></input>'));
      elt.append(form);
    }

    makeForm();
    update();

    elt.find('form').submit(function(ev) {
      $(this).find(':input:not([type="submit"])').each(function() {
        parms[this.name] = 1 * $(this).val();
      });

      update();
      ev.preventDefault();
    });

  }

  var DF = 'average.json';
  var MAX = 70;

  $.get(DF, {
    dataType: 'json'
  }).done(function(data) {
    if (typeof(data) == 'string') data = JSON.parse(data); // fuck knows...
    var series = data[0];

    var panel = $('#panel');
    var canvas = panel.find('.plot')[0];

    var width = canvas.width;
    var height = canvas.height;
    var margin = 8;

    var xs = makeScaler(0, series.length - 1, margin, width - margin);
    var ys = makeScaler(0, 1, height - margin, margin);

    var parms = {
      c: 4.15,
      m: -0.45,
      p: 1.2
    };

    makeControls(panel, series, xs, ys, parms);

  });

});

var ProxNode = (function() {
  function p(vec) {
    this.vec = vec;
    this.kids = null;
  }

  var pp = p.prototype = new Object();

  p.midway = function(va, vb) {
    var vo = [];
    if (va.length != vb.length) throw new Error("vector size mismatch");
    for (var i = 0; i < va.length; i++) vo.push((va[i] + vb[i]) / 2);
    return vo;
  }

  pp.insert = function(tree) {
    if (!tree) return this;
    if (tree.kids) {}

  }

  return p;
});

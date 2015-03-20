"use strict";

module.exports = (function() {
  var stack = [];
  var next_id = 0;

  var DEBUG = {
    CONTEXT: false,
    PROPERTY_GET: false
  }

  // Not sure why but...
  function MM() {
    return require('../MovieMaker.js')
  }

  function isMagic(x) {
    return (x instanceof Function || x instanceof MM().DynamicPropertyBase)
  }

  function indent(n) {
    var pad = '';
    for (var i = 0; i < n; i++) pad += '  '
    return pad
  }

  function prindent(n, str) {
    var ln = str.split(/\n/)
    var pad = indent(n)
    for (var i = 0; i < ln.length; i++) {
      console.log(pad + ln[i])
    }
  }

  function json(x) {
    return JSON.stringify(x, null, 2)
  }

  function bounder(self, ff) {
    if (ff._bound) return ff
    var ffb = function() {
      return ff.apply(self, arguments)
    }
    ffb._bound = true
    return ffb
  }

  return {

    getUniqueID: function() {
      if (!this.hasOwnProperty('_id')) this._id = ++next_id;
      return this._id;
    },

    pushContext: function(ctx) {
      ctx._prop = {};
      if (DEBUG.CONTEXT) {
        prindent(stack.length, "pushContext:\n" + JSON.stringify(ctx, null, 2))
      }
      stack.push(ctx);
    },

    popContext: function() {
      if (stack.length == 0) {
        throw new exports.StackEmptyError("Evaluation context empty");
      }
      var ctx = stack.pop();
      if (DEBUG.CONTEXT) {
        prindent(stack.length, "popContext:\n" + JSON.stringify(ctx, null, 2))
      }

      return ctx
    },

    getContext: function() {
      return stack.length ? stack[stack.length - 1] : null;
    },

    bindProperty: function(name, prop) {
      if (isMagic(prop)) {
        if (prop instanceof Function) prop = bounder(this, prop);
        Object.defineProperty(this, name, {
          get: function() {
            var ctx = this.getContext();
            if (!ctx) return prop;
            var id = this.getUniqueID();

            if (!ctx._prop[id]) ctx._prop[id] = {};
            if (!ctx._prop[id][name]) ctx._prop[id][name] = {};

            var pctx = ctx._prop[id][name];
            var cache = pctx.cache;

            if (pctx.hasOwnProperty('cache')) return pctx.cache;
            if (pctx.eval) {
              var mm = MM()
              throw new mm.CircularReferenceError("Property " + name + " contains a circular reference");
            }

            pctx.eval = true;
            var val = prop instanceof Function ? prop(ctx) : prop.evaluate(ctx)
            delete pctx.eval;

            if (DEBUG.PROPERTY_GET) {
              console.log("PROPERTY_GET: id = " + id + ", " + name + " = " + val)
            }

            return pctx.cache = val;
          }
        })

      } else {
        Object.defineProperty(this, name, {
          get: function() {
            return prop
          }
        })
      }
    },

    bindParameters: function(params, defaults) {
      var keys = Object.keys(defaults);
      for (var i = 0; i < keys.length; i++) {
        var key = keys[i];
        var val = params && params.hasOwnProperty(key) ? params[key] : defaults[key];
        this.bindProperty(key, val);
      }
    }
  }
})()

"use strict";

module.exports = (function() {
  var exports = {
    mixin: function() {
      if (arguments.length == 0) throw new Error("Missing argument to mixin");
      var target = arguments[0];
      for (var i = 1; i < arguments.length; i++) {
        var mix = arguments[i];
        var keys = Object.keys(mix);
        for (var j = 0; j < keys.length; j++) {
          target[keys[j]] = mix[keys[j]];
        }
      }
      return target;
    },

    defineClass: function() {
      var args = Array.prototype.slice.call(arguments);

      var klass = (args.length && args[0] instanceof Function) ? args.shift() : function() {};

      if (args.length && args[0] instanceof Array) { // class methods
        var mixin_args = args.shift().slice();
        mixin_args.unshift(klass);
        exports.mixin.apply(null, mixin_args);
      }

      if (!args.length) args.push({});
      klass.prototype = exports.mixin.apply(null, args);

      return klass;
    }
  }
  return exports;
})()

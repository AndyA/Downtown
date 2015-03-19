"use strict";

var chai = require("chai");
chai.use(require('chai-subset'));

var expect = chai.expect;

var MM = require("../lib/MovieMaker.js");

describe("util", function() {

  describe("mixin", function() {

    it("should merge objects", function() {

      expect(MM.util.mixin({},
      {
        a: 1
      })).to.deep.equal({
        a: 1
      });

      expect(MM.util.mixin({
        a: 1,
        b: 2,
        c: 3,
        d: 4,
        e: 5,
        f: 6
      },
      {
        a: 101,
        c: 103
      },
      {
        a: 201,
        d: 204
      })).to.deep.equal({
        a: 201,
        b: 2,
        c: 103,
        d: 204,
        e: 5,
        f: 6,
      });

    });

    it("should create working classes", function() {

      var MathMixin = {
        getName: function() {
          return "MathMixin"
        },

        fib: function(n) {
          if (n < 2) return n
          return this.fib(n - 1) + this.fib(n - 2)
        }
      }

      var LieMixin = {
        getName: function() {
          return "LieMixin"
        },

        fib: function(n) {
          return n + " == " + (n + 1)
        }
      }

      function ThingA() {}

      ThingA.prototype = {
        getName: function() {
          return "ThingA"
        },

        beThingA: function() {
          throw new Error("Don't call me!");
        }
      }

      function ThingB() {}

      ThingB.prototype = {
        getName: function() {
          return "ThingB"
        },

        beThingB: function() {
          throw new Error("Don't call me!");
        }
      }

      MM.util.mixin(ThingA.prototype, MathMixin);
      MM.util.mixin(ThingB.prototype, LieMixin);

      var thing_a = new ThingA();
      var thing_b = new ThingB();

      expect(thing_a).to.respondTo("getName").and.respondTo("beThingA").and.respondTo("fib");
      expect(thing_b).to.respondTo("getName").and.respondTo("beThingB").and.respondTo("fib");

      expect(thing_a.getName()).to.equal("MathMixin");
      expect(thing_b.getName()).to.equal("LieMixin");

      expect(thing_a.fib(5)).to.equal(5);
      expect(thing_b.fib(5)).to.equal("5 == 6");

    });

  });

  describe("defineClass", function() {

    it("should define simple classes", function() {
      var SuperFoo = MM.util.defineClass({
        greet: function(n) {
          return "Hello, " + n
        }
      })

      var super_foo = new SuperFoo();
      expect(super_foo.greet("you")).to.equal("Hello, you");

    });

    it("should allow a constructor to be provided", function() {
      var constructor_called = 0;
      var ExtraFoo = MM.util.defineClass(function() {
        constructor_called++;
      },
      {
        greet: function(n) {
          return "Goodbye, " + n
        }
      })

      var extra_foo = new ExtraFoo();
      expect(constructor_called).to.equal(1);
      expect(extra_foo.greet("you")).to.equal("Goodbye, you");
    });

    it("should allow class methods to be defined", function() {
      var constructor_called = 0;
      var UltaFoo = MM.util.defineClass(function() {
        constructor_called++;
      },
      [{
        version: function() {
          return 3
        },
        status: function() {
          return "current"
        }
      },
      {
        status: function() {
          return "deprecated"
        }
      }], {
        greet: function(n) {
          return "Goodbye, " + n
        }
      })

      expect(UltaFoo).itself.to.respondTo('version').and.respondTo('status');
      expect(UltaFoo.version()).to.equal(3);
      expect(UltaFoo.status()).to.equal("deprecated");

      var ultra_foo = new UltaFoo();
      expect(constructor_called).to.equal(1);
      expect(ultra_foo.greet("you")).to.equal("Goodbye, you");

    });

    it("should allow class methods with no constructor", function() {
      var MegaFoo = MM.util.defineClass([{
        version: function() {
          return 3
        },
        status: function() {
          return "current"
        }
      },
      {
        status: function() {
          return "deprecated"
        }
      }], {
        greet: function(n) {
          return "Goodbye, " + n
        }
      })

      expect(MegaFoo).itself.to.respondTo('version').and.respondTo('status');
      expect(MegaFoo.version()).to.equal(3);
      expect(MegaFoo.status()).to.equal("deprecated");

      var mega_foo = new MegaFoo();
      expect(mega_foo.greet("you")).to.equal("Goodbye, you");

    });

  });
});

const assert = require('assert/strict');
const { test, beforeEach } = require('node:test');

let addon;

beforeEach(() => {
  addon = require('../lib');
});

test('Ensure getPrinters function exists', (t) => {
  assert.ok(addon.getPrinters, 'getPrinters function should exist');
  assert.strictEqual(typeof addon.getPrinters, 'function', 'getPrinters should be a function');
});

test('Ensure getPrinters returns an array', (t) => {
  const printers = addon.getPrinters();
  assert.ok(Array.isArray(printers), 'getPrinters should return an array');
});

test('Ensure getPrinters handles no printers scenario', (t) => {
  const printers = addon.getPrinters();
  assert.ok(Array.isArray(printers), 'getPrinters should return an array');
  if (printers.length > 0) {
    assert.strictEqual(typeof printers[0], 'object', 'Each printer should be an object');
  }
});

const assert = require('assert/strict');
const { test, beforeEach } = require('node:test');

let addon;

beforeEach(() => {
  addon = require('../lib');
});
  
test('Ensure getDefaultPrinterName function exists', (t) => {
  assert.ok(addon.getDefaultPrinterName, 'getDefaultPrinterName function should exist');
  assert.strictEqual(typeof addon.getDefaultPrinterName, 'function', 'getDefaultPrinterName should be a function');
});

test('Ensure getDefaultPrinterName returns a string', (t) => {
  const result = addon.getDefaultPrinterName();
  assert.strictEqual(typeof result, "string", 'getDefaultPrinterName should return a string');
});

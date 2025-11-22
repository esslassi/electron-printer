const assert = require('assert/strict');
const { test, beforeEach } = require('node:test');

let addon;

beforeEach(() => {
  addon = require('../lib');
});
  
test('Ensure addon is loaded correctly', (t) => {
    assert.ok(addon, 'Addon should be loaded');
    assert.strictEqual(typeof addon, 'object', 'Addon should be an object');
    // Add checks for specific functions if needed
    assert.strictEqual(typeof addon.sayMyName, 'function', 'Addon should have sayMyName');
  });

// Additional test cases 
test('Test sayMyName behavior', (t) => {
  const result = addon.sayMyName();
  assert.strictEqual(result, 'Hello, From C++ !', 'sayMyName should return "Hello, From C++ !"');
});

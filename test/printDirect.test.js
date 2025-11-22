const assert = require('assert/strict');
const { test, beforeEach } = require('node:test');

let addon;
let template;
beforeEach(() => {
  addon = require('../lib');
  template = "N\nS4\nD15\nq400\nR\nB20,10,0,1,2,30,173,B,\"barcode\"\nP0\n";
});

test('Ensure printDirect function exists', (t) => {
  assert.ok(addon.printDirect, 'printDirect function should exist');
  assert.strictEqual(typeof addon.printDirect, 'function', 'printDirect should be a function');
});

test('Ensure printDirect throws error when no arguments are passed', (t) => {
  assert.throws(() => {
    addon.printDirect();
  }, /Error/, 'printDirect should throw an error when no arguments are passed');
});

test('Ensure printDirect works properly', (t) => {
 let barcode_text = "123";
 let printer_name = addon.getDefaultPrinterName();
  addon.printDirect({
		data:template.replace(/barcode/, barcode_text)
		, printer:printer_name
		, type: "RAW"
		, success:function(){
			console.log("printed: "+barcode_text);
		}
		, error:function(err){console.log(err);}
	});
});
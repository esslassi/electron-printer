const assert = require('assert/strict');
const { test, beforeEach } = require('node:test');

let addon;

beforeEach(() => {
  addon = require('../lib');
});
  
test('Ensure Non-functional apis do not lead any issues', (t) => {
    assert.ok(addon, 'Non-functional apis');
    // Add checks for specific functions if needed
    assert.strictEqual(typeof addon.getPrinter, 'function', 'Addon should have getPrinter');
  });

// Additional test cases 
test('Test getPrinter behavior', (t) => {
  const result = addon.getPrinter();
  assert.strictEqual(result, 'getPrinter is not implemented yet, request for a PR if required', 'getPrinter is not implemented yet, request for a PR if required"');
});

test('Test printFile behavior', (t) => {
  const filename = process.argv[2] || __filename; // use this file as an input
  if( process.platform != 'win32') {
    addon.printFile({filename:filename,
      printer: process.env[3], // printer name, if missing then will print to default printer
      success:function(jobID){
        console.log("addon.printFile: Job ID: "+jobID);
      },
      error:function(err){
        console.log(err);
      }
    });
  } else {
    // not yet implemented, use printDirect and text
    var fs = require('fs');
    addon.printDirect({data:fs.readFileSync(filename),
      printer: addon.getDefaultPrinterName(), // printer name, if missing then will print to default printer
      type: "RAW",
      success:function(jobID){
        assert.equal(jobID>0, true, 'addon.printFile');
      },
      error:function(err){
        console.log(err);
        assert.strictEqual(!err, true, 'addon.printFile'); // failed test case
      }
    });
  }
});

// getSupportedPrintFormats
test('Test getSupportedPrintFormats behavior', (t) => {
  assert.throws(() => {
    addon.getSupportedPrintFormats();
  }, /Error/, 'getSupportedPrintFormats should throw an error when no arguments are passed');
});

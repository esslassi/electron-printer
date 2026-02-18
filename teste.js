const { getPrinters, getDefaultPrinter, printDirect, getStatusPrinter } = require('./lib/');

// Testing printer listing
getPrinters()
  .then((printers) => {
    console.log('Available printers:', printers);
  })
  .catch((error) => {
    console.error('Error listing printers:', error);
  });

// Testing default printer
getDefaultPrinter()
  .then((printer) => {
    console.log('Default printer:', printer);
  })
  .catch((error) => {
    console.error('Error getting default printer:', error);
  });

// Testing printing
const options = {
  printerName: 'HP508140D7C039(HP Laser MFP 131 133 135-138)',
  data: 'Hello, world!',
  dataType: 'RAW'
};

printDirect(options)
  .then((resp) => {
    console.log(resp);
  })
  .catch(console.error);

// Testing printer status
getStatusPrinter({ printerName: 'HP508140D7C039(HP Laser MFP 131 133 135-138)' })
  .then((printerInfo) => {
    console.log('Printer status:', printerInfo);
  })
  .catch((error) => {
    console.error('Error getting printer status:', error);
  });
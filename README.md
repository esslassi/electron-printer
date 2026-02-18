Here is the clean **README.md** content ready to copy and paste:

---

# Electron Printer (@esslassi/electron-printer)

Node.js and Electron bindings for printer management and direct printing. Supports Windows and Linux (CUPS).

## Features

* List all available printers
* Get the system default printer
* Get detailed printer status
* Direct printing (raw printing)
* TypeScript support
* Asynchronous API (Promises)
* Compatible with Node.js and Electron

## Requirements

* Node.js >= 18.20.6
* Electron >= 20.0.0
* Windows or Linux
* For Windows: Visual Studio Build Tools
* For Linux: CUPS development headers

```bash
sudo apt-get install libcups2-dev
```

## Installation

```bash
npm install @esslassi/electron-printer
```

For development:

```bash
git clone https://github.com/esslassi/electron-printer.git
cd electron-printer
npm install
npm run rebuild
```

## Usage

### JavaScript

```javascript
const printer = require('@esslassi/electron-printer');

// List all printers
printer.getPrinters()
  .then(printers => {
    console.log('Available printers:', printers);
  })
  .catch(console.error);

// Get default printer
printer.getDefaultPrinter()
  .then(defaultPrinter => {
    console.log('Default printer:', defaultPrinter);
  })
  .catch(console.error);

// Check printer status
printer.getStatusPrinter({ printerName: 'Printer Name' })
  .then(status => {
    console.log('Printer status:', status);
  })
  .catch(console.error);

// Print directly
const printOptions = {
  printerName: 'Printer Name',
  data: 'Text to print',
  dataType: 'RAW' // optional (default is 'RAW')
};

printer.printDirect(printOptions)
  .then(result => {
    console.log('Result:', result);
  })
  .catch(console.error);
```

### TypeScript

```typescript
import printer, { 
  Printer, 
  PrintDirectOptions, 
  GetStatusPrinterOptions 
} from '@esslassi/electron-printer';

async function example() {
  try {
    const printers: Printer[] = await printer.getPrinters();
    console.log('Printers:', printers);

    const defaultPrinter: Printer = await printer.getDefaultPrinter();
    console.log('Default printer:', defaultPrinter);

    const statusOptions: GetStatusPrinterOptions = {
      printerName: 'Printer Name'
    };
    const status: Printer = await printer.getStatusPrinter(statusOptions);
    console.log('Status:', status);

    const printOptions: PrintDirectOptions = {
      printerName: 'Printer Name',
      data: Buffer.from('Text to print'),
      dataType: 'RAW'
    };

    const result = await printer.printDirect(printOptions);
    console.log('Result:', result);

  } catch (error) {
    console.error('Error:', error);
  }
}
```

## API

### getPrinters(): Promise<Printer[]>

Lists all printers installed on the system.

```typescript
interface Printer {
  name: string;
  isDefault: boolean;
  status: string;
  details: {
    location?: string;
    comment?: string;
    driver?: string;
    port?: string;
    [key: string]: string | undefined;
  };
}
```

---

### getDefaultPrinter(): Promise<Printer>

Returns the system default printer.

---

### getStatusPrinter(options: GetStatusPrinterOptions): Promise<Printer>

```typescript
interface GetStatusPrinterOptions {
  printerName: string;
}
```

---

### printDirect(options: PrintDirectOptions): Promise<string>

```typescript
interface PrintDirectOptions {
  printerName: string;
  data: string | Buffer;
  dataType?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO';
}
```

### Possible Status Values

* ready
* offline
* error
* paper-jam
* paper-out
* manual-feed
* paper-problem
* busy
* printing
* output-bin-full
* not-available
* waiting
* processing
* initializing
* warming-up
* toner-low
* no-toner
* page-punt
* user-intervention
* out-of-memory
* door-open

---

## Supported Platforms

* Windows (32/64-bit)
* Linux (CUPS)

---

## Troubleshooting

### Windows

1. Install Visual Studio Build Tools
2. Run:

```bash
npm run rebuild
```

3. Verify printer access permissions

### Linux

```bash
sudo apt-get install libcups2-dev
sudo service cups status
sudo usermod -a -G lp $USER
```

---

## Development

```bash
git clone https://github.com/esslassi/electron-printer.git
cd electron-printer
npm install
npm run rebuild
node test.js
```

---

## License

MIT

---

## Author

Esslassi
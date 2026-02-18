import bindings from 'bindings';
const electronPrinter = bindings('electron_printer');

export interface PrintOptions {
  printerName: string;
  data: string | Buffer;
  dataType?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO' | undefined;
}

export interface Printer {
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

export interface PrintDirectOptions {
  printerName: string;
  data: string | Buffer;
  dataType?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO' | undefined;
}

export interface GetStatusPrinterOptions {
  printerName: string;
}

export interface PrintDirectOutput {
  name: string;
  status: 'success' | 'failed';
}


export async function printDirect(printOptions: PrintOptions): Promise<PrintDirectOutput> {
  const input = {
    ...printOptions,
    printerName: normalizeString(printOptions.printerName)
  }
  const printer = await electronPrinter.printDirect(input)
  return printer
}

export async function getStatusPrinter(printOptions: GetStatusPrinterOptions): Promise<Printer> {
  const input = {
    ...printOptions,
    printerName: normalizeString(printOptions.printerName)
  }
  const printer = await electronPrinter.getStatusPrinter(input)
  return printer
}


export async function getPrinters(): Promise<Printer[]> {
  const printers = await electronPrinter.getPrinters()
  return printers
}

export async function getDefaultPrinter(): Promise<Printer> {
  const printer = await electronPrinter.getDefaultPrinter()
  return printer
}


function normalizeString(str: string) {
  return String.raw`${str}`
}
import bindings from 'bindings'

const native = bindings('electron_printer')

/* ===========================
   TYPES
=========================== */

export type PrintOnSuccessFunction = (jobId: string) => any
export type PrintOnErrorFunction = (err: Error) => any

export interface PrintDirectOptions {
  data: string | Buffer
  printer?: string
  type?: 'RAW' | 'TEXT' | 'PDF' | 'JPEG' | 'POSTSCRIPT' | 'COMMAND' | 'AUTO'
  options?: { [key: string]: string }
  success?: PrintOnSuccessFunction
  error?: PrintOnErrorFunction
}

export interface PrintFileOptions {
  filename: string
  printer?: string
  success?: PrintOnSuccessFunction
  error?: PrintOnErrorFunction
}

export interface PrinterDetails {
  name: string
  isDefault: boolean
  options: { [key: string]: string }
}

export interface PrinterDriverOptions {
  [key: string]: { [key: string]: boolean }
}

export type JobStatus =
  | 'PAUSED'
  | 'PRINTING'
  | 'PRINTED'
  | 'CANCELLED'
  | 'PENDING'
  | 'ABORTED'

export type JobCommand =
  | "CANCEL"
  | "PAUSE"
  | "RESUME"

export interface JobDetails {
  id: number
  name: string
  printerName: string
  user: string
  format: string
  priority: number
  size: number
  status: JobStatus[]
  completedTime: Date
  creationTime: Date
  processingTime: Date
}

/* ===========================
   DIRECT NATIVE EXPORTS
=========================== */

export function getPrinters(): PrinterDetails[] {
  return native.getPrinters()
}

export function getPrinter(printerName: string): PrinterDetails {
  return native.getPrinter(printerName)
}

export function getPrinterDriverOptions(
  printerName: string
): PrinterDriverOptions {
  return native.getPrinterDriverOptions(printerName)
}

export function getSelectedPaperSize(printerName: string): string {
  return native.getSelectedPaperSize(printerName)
}

export function getDefaultPrinterName(): string | undefined {
  return native.getDefaultPrinterName()
}

export function printDirect(options: PrintDirectOptions): void {
  native.printDirect(options)
}

export function printFile(options: PrintFileOptions): void {
  native.printFile(options)
}

export function getSupportedPrintFormats(): string[] {
  return native.getSupportedPrintFormats()
}

export function getJob(
  printerName: string,
  jobId: number
): JobDetails {
  return native.getJob(printerName, jobId)
}

export function setJob(
  printerName: string,
  jobId: number,
  command: JobCommand
): void {
  native.setJob(printerName, jobId, command)
}

export function getSupportedJobCommands(): string[] {
  return native.getSupportedJobCommands()
}
/* ==================================================
   PROMISE WRAPPERS (Async/Await Friendly)
================================================== */

export function printDirectAsync(
  options: Omit<PrintDirectOptions, 'success' | 'error'>
): Promise<string> {
  return new Promise((resolve, reject) => {
    native.printDirect({
      ...options,
      success: (jobId: string) => resolve(jobId),
      error: (err: Error) => reject(err)
    })
  })
}

export function printFileAsync(
  options: Omit<PrintFileOptions, 'success' | 'error'>
): Promise<string> {
  return new Promise((resolve, reject) => {
    native.printFile({
      ...options,
      success: (jobId: string) => resolve(jobId),
      error: (err: Error) => reject(err)
    })
  })
}

export function getJobAsync(
  printerName: string,
  jobId: number
): Promise<JobDetails> {
  return Promise.resolve(native.getJob(printerName, jobId))
}

export function getPrintersAsync(): Promise<PrinterDetails[]> {
  return Promise.resolve(native.getPrinters())
}

export function getPrinterAsync(
  printerName: string
): Promise<PrinterDetails> {
  return Promise.resolve(native.getPrinter(printerName))
}
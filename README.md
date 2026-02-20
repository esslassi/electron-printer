# 📦 @esslassi/electron-printer

Cross-platform native printer driver for **Windows, macOS, and Linux**, built with Node-API (N-API).
Works in **Node.js** and **Electron**.

Supports:

* 🖨 List printers
* 📄 Print raw/text/command data
* 📁 Print files
* 📋 Get printer driver options
* 📑 Get selected paper size
* 📦 Get job status
* ⛔ Cancel / pause / resume jobs
* 🔄 Promise (async/await) wrappers included

---

## 🚀 Installation

```bash
npm install @esslassi/electron-printer
```

If using Electron:

```bash
npx electron-rebuild
```

---

## 🧩 Supported Platforms

| OS      | Backend      |
| ------- | ------------ |
| Windows | WinSpool API |
| macOS   | CUPS         |
| Linux   | CUPS         |

---

# 📖 Usage

```ts
import * as printer from '@esslassi/electron-printer'
```

---

# 🖨 Printer Management

### Get all printers

```ts
const printers = printer.getPrinters()
```

### Async version

```ts
const printers = await printer.getPrintersAsync()
```

---

### Get single printer

```ts
const details = printer.getPrinter("My Printer")
```

---

### Get default printer

```ts
const defaultPrinter = printer.getDefaultPrinterName()
```

---

### Get printer driver options

```ts
const options = printer.getPrinterDriverOptions("My Printer")
```

---

### Get selected paper size

```ts
const paper = printer.getSelectedPaperSize("My Printer")
```

---

# 🖨 Printing

---

## 🟢 Print Raw/Text Data (Callback)

```ts
printer.printDirect({
  data: "Hello Printer\n\n",
  printer: "My Printer",
  type: "RAW",
  success: (jobId) => {
    console.log("Printed. Job ID:", jobId)
  },
  error: (err) => {
    console.error("Print error:", err)
  }
})
```

---

## 🟢 Print Raw/Text Data (Async/Await)

```ts
const jobId = await printer.printDirectAsync({
  data: Buffer.from("Hello Printer\n\n"),
  printer: "My Printer",
  type: "RAW"
})

console.log("Printed. Job ID:", jobId)
```

---

## 🟢 Print File (Callback)

```ts
printer.printFile({
  filename: "./file.txt",
  printer: "My Printer",
  success: (jobId) => console.log(jobId),
  error: (err) => console.error(err)
})
```

---

## 🟢 Print File (Async)

```ts
const jobId = await printer.printFileAsync({
  filename: "./file.txt",
  printer: "My Printer"
})
```

---

# 📦 Job Management

---

## Get Job Details

```ts
const job = printer.getJob("My Printer", 42)
```

### Async

```ts
const job = await printer.getJobAsync("My Printer", 42)
```

---

## Cancel / Pause / Resume Job

```ts
printer.setJob("My Printer", 42, "CANCEL")
printer.setJob("My Printer", 42, "PAUSE")
printer.setJob("My Printer", 42, "RESUME")
```

---

## Get Supported Job Commands

```ts
const commands = printer.getSupportedJobCommands()
```

---

# 📄 Supported Print Formats

```ts
const formats = printer.getSupportedPrintFormats()
```

Typical values:

* `RAW`
* `TEXT`
* `COMMAND`
* `AUTO`

> Windows supports RAW/TEXT/COMMAND directly.
> macOS/Linux support additional formats via CUPS.

---

# 🧠 TypeScript Types

Fully typed. Example:

```ts
export interface PrintDirectOptions {
  data: string | Buffer
  printer?: string
  type?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO'
  options?: { [key: string]: string }
}
```

---

# ⚡ Promise Wrappers Included

Built-in async wrappers:

* `printDirectAsync`
* `printFileAsync`
* `getPrintersAsync`
* `getPrinterAsync`
* `getJobAsync`

No extra wrapper needed.

---

# 🧪 Example Full Test

```ts
import * as printer from '@esslassi/electron-printer'

async function test() {
  const printers = await printer.getPrintersAsync()
  const defaultPrinter = printer.getDefaultPrinterName()

  const jobId = await printer.printDirectAsync({
    data: "Test print\n\n",
    printer: defaultPrinter,
    type: "RAW"
  })

  const job = await printer.getJobAsync(defaultPrinter!, Number(jobId))

  console.log(job)
}

test()
```

---

# 🏗 Architecture

```
Node.js / Electron
        ↓
N-API
        ↓
PrinterFactory
        ↓
Windows | macOS | Linux drivers
```

Native C++ backend with platform-specific implementations.

---

# 🔧 Development

Build from source:

```bash
npx node-gyp rebuild
```

Electron:

```bash
npx electron-rebuild
```

---

# 📜 License

MIT

---

# 👨‍💻 Author

Esslassi

---

If you want, I can also generate:

* 🔥 NPM package.json template
* 🧱 binding.gyp optimized version
* ⚡ Prebuild support (no node-gyp required for users)
* 🧪 Jest test suite
* 📦 CLI tool version

Tell me the next step 🚀
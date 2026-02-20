const printer = require('./lib'); 

async function run() {
  console.log("===== TEST START =====");

  // -------------------------------------------------
  // 1️⃣ Get printers (sync)
  // -------------------------------------------------
  const printers = printer.getPrinters();
  console.log("Printers:", printers);

  if (!printers.length) {
    console.log("No printers found. Exiting.");
    return;
  }

  const defaultPrinter = printer.getDefaultPrinterName();
  console.log("Default printer:", defaultPrinter);

  const selectedPrinter = defaultPrinter || printers[0].name;
  console.log("Using printer:", selectedPrinter);

  // -------------------------------------------------
  // 2️⃣ Get single printer
  // -------------------------------------------------
  const printerDetails = printer.getPrinter(selectedPrinter);
  console.log("Printer details:", printerDetails);

  // -------------------------------------------------
  // 3️⃣ Driver options
  // -------------------------------------------------
  const driverOptions = printer.getPrinterDriverOptions(selectedPrinter);
  console.log("Driver options:", driverOptions);

  // -------------------------------------------------
  // 4️⃣ Paper size
  // -------------------------------------------------
  const paper = printer.getSelectedPaperSize(selectedPrinter);
  console.log("Selected paper size:", paper);

  // -------------------------------------------------
  // 5️⃣ Supported formats
  // -------------------------------------------------
  console.log("Supported formats:", printer.getSupportedPrintFormats());

  // -------------------------------------------------
  // 6️⃣ Supported job commands
  // -------------------------------------------------
  console.log("Supported job commands:", printer.getSupportedJobCommands());

  // -------------------------------------------------
  // 7️⃣ printDirect (callback version)
  // -------------------------------------------------
  printer.printDirect({
    data: "TEST PRINT FROM CALLBACK\n\n",
    printer: selectedPrinter,
    type: "RAW",
    success: (jobId) => {
      console.log("printDirect success jobId:", jobId);

      const job = printer.getJob(selectedPrinter, parseInt(jobId));
      console.log("Job details (sync):", job);

      // Try cancel test (optional)
      // printer.setJob(selectedPrinter, parseInt(jobId), "CANCEL");
    },
    error: (err) => {
      console.error("printDirect error:", err);
    }
  });

  // -------------------------------------------------
  // 8️⃣ printDirectAsync
  // -------------------------------------------------
  try {
    const jobId = await printer.printDirectAsync({
      data: Buffer.from("TEST PRINT FROM ASYNC\n\n"),
      printer: selectedPrinter,
      type: "RAW"
    });

    console.log("printDirectAsync jobId:", jobId);

    const jobAsync = await printer.getJobAsync(selectedPrinter, parseInt(jobId));
    console.log("Job details (async):", jobAsync);

  } catch (err) {
    console.error("printDirectAsync error:", err);
  }

  // -------------------------------------------------
  // 9️⃣ printFileAsync
  // -------------------------------------------------
  try {
    const jobId = await printer.printFileAsync({
      filename: "./sample.txt", // make sure this exists
      printer: selectedPrinter
    });

    console.log("printFileAsync jobId:", jobId);
  } catch (err) {
    console.error("printFileAsync error:", err);
  }

  // -------------------------------------------------
  // 🔟 getPrintersAsync
  // -------------------------------------------------
  const printersAsync = await printer.getPrintersAsync();
  console.log("Printers (async):", printersAsync);

  // -------------------------------------------------
  // 11️⃣ getPrinterAsync
  // -------------------------------------------------
  const singlePrinterAsync = await printer.getPrinterAsync(selectedPrinter);
  console.log("Single printer (async):", singlePrinterAsync);

  console.log("===== TEST END =====");
}

run().catch(console.error);
#ifndef MAC_PRINTER_H
#define MAC_PRINTER_H

#include "printer_interface.h"

class MacPrinter : public PrinterInterface
{
public:
    std::vector<PrinterDetailsNative> GetPrinters() override;
    PrinterDetailsNative GetPrinter(const std::string &printerName) override;
    std::string GetDefaultPrinterName() override;

    DriverOptions GetPrinterDriverOptions(const std::string &printerName) override;
    std::string GetSelectedPaperSize(const std::string &printerName) override;

    int PrintDirect(const std::string &printerName,
                    const std::vector<uint8_t> &data,
                    const std::string &type,
                    const StringMap &options) override;

    int PrintFile(const std::string &printerName,
                  const std::string &filename) override;

    std::vector<std::string> GetSupportedPrintFormats() override;

    JobDetailsNative GetJob(const std::string &printerName, int jobId) override;
    void SetJob(const std::string &printerName, int jobId, const std::string &command) override;
    std::vector<std::string> GetSupportedJobCommands() override;
};

#endif
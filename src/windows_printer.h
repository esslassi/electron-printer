#ifndef WINDOWS_PRINTER_H
#define WINDOWS_PRINTER_H

#include "printer_interface.h"

#include <windows.h>
#include <winspool.h>

/* Now remove macro pollution */
#ifdef GetDefaultPrinter
#undef GetDefaultPrinter
#endif

#ifdef GetDefaultPrinter
#undef GetDefaultPrinter
#endif

#ifdef GetPrinter
#undef GetPrinter
#endif

#ifdef GetJob
#undef GetJob
#endif

#ifdef SetJob
#undef SetJob
#endif

#include <vector>
#include <string>
#include <cstdint>

class WindowsPrinter : public PrinterInterface
{
private:
    std::wstring Utf8ToWide(const std::string &str);
    std::string WideToUtf8(LPWSTR wstr);
    std::vector<std::string> MapJobStatus(DWORD status);

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
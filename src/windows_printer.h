#ifndef WINDOWS_PRINTER_H
#define WINDOWS_PRINTER_H

#define NOMINMAX
#include <windows.h>
#include <winspool.h>
#include <cstdint>
#include "printer_interface.h"
#include <vector>

// Desabilita todas as macros do Windows que podem interferir
#ifdef GetDefaultPrinter
#undef GetDefaultPrinter
#endif

#ifdef GetPrinter
#undef GetPrinter
#endif

class WindowsPrinter : public PrinterInterface
{
private:
    std::string GetPrinterStatus(DWORD status);
    std::wstring Utf8ToWide(const std::string &str);
    std::string WideToUtf8(LPWSTR wstr);

public:
    virtual PrinterInfo GetPrinterDetails(const std::string &printerName, bool isDefault = false) override;
    virtual std::vector<PrinterInfo> GetPrinters() override;
    virtual PrinterInfo GetSystemDefaultPrinter() override;
    virtual bool PrintDirect(const std::string &printerName, const std::vector<uint8_t> &data, const std::string &dataType) override;
    virtual PrinterInfo GetStatusPrinter(const std::string &printerName) override;
};

#endif
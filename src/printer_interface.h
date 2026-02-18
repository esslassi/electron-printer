#ifndef PRINTER_INTERFACE_H
#define PRINTER_INTERFACE_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

struct PrinterInfo
{
    std::string name;
    bool isDefault;
    std::map<std::string, std::string> details;
    std::string status;
};

class PrinterInterface
{
public:
    virtual ~PrinterInterface() = default;

    virtual PrinterInfo GetPrinterDetails(const std::string &printerName, bool isDefault = false) = 0;
    virtual std::vector<PrinterInfo> GetPrinters() = 0;
    virtual PrinterInfo GetSystemDefaultPrinter() = 0;
    virtual bool PrintDirect(const std::string &printerName, const std::vector<uint8_t> &data, const std::string &dataType) = 0;
    virtual PrinterInfo GetStatusPrinter(const std::string &printerName) = 0;
};

#endif
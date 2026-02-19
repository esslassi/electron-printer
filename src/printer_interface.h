#ifndef PRINTER_INTERFACE_H
#define PRINTER_INTERFACE_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <ctime>

using StringMap = std::map<std::string, std::string>;
using DriverOptions = std::map<std::string, std::map<std::string, bool>>;

struct PrinterDetailsNative {
    std::string name;
    bool isDefault = false;
    StringMap options; // matches TS: options: { [k:string]: string }
};

struct JobDetailsNative {
    int id = 0;
    std::string name;
    std::string printerName;
    std::string user;
    std::string format;
    int priority = 0;
    int size = 0;

    std::vector<std::string> status; // TS: JobStatus[]
    std::time_t completedTime = 0;
    std::time_t creationTime = 0;
    std::time_t processingTime = 0;
};

class PrinterInterface
{
public:
    virtual ~PrinterInterface() = default;

    // Printers
    virtual std::vector<PrinterDetailsNative> GetPrinters() = 0;
    virtual PrinterDetailsNative GetPrinter(const std::string &printerName) = 0;
    virtual std::string GetDefaultPrinterName() = 0;

    // Driver options & paper
    virtual DriverOptions GetPrinterDriverOptions(const std::string &printerName) = 0;
    virtual std::string GetSelectedPaperSize(const std::string &printerName) = 0;

    // Printing
    // return jobId (>0) or 0 on failure
    virtual int PrintDirect(const std::string &printerName,
                            const std::vector<uint8_t> &data,
                            const std::string &type,
                            const StringMap &options) = 0;

    virtual int PrintFile(const std::string &printerName,
                          const std::string &filename) = 0;

    // Capabilities
    virtual std::vector<std::string> GetSupportedPrintFormats() = 0;

    // Jobs
    virtual JobDetailsNative GetJob(const std::string &printerName, int jobId) = 0;
    virtual void SetJob(const std::string &printerName, int jobId, const std::string &command) = 0;
    virtual std::vector<std::string> GetSupportedJobCommands() = 0;
};

#endif
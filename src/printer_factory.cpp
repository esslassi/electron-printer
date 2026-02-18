#include "printer_factory.h"

#ifdef _WIN32
#include "windows_printer.h"
#elif defined(__APPLE__)
#include "mac_printer.h"
#else
#include "linux_printer.h"
#endif

std::unique_ptr<PrinterInterface> PrinterFactory::Create()
{
#ifdef _WIN32
    return std::make_unique<WindowsPrinter>();
#elif defined(__APPLE__)
    return std::make_unique<MacPrinter>();
#else
    return std::make_unique<LinuxPrinter>();
#endif
}
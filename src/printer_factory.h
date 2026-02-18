#ifndef PRINTER_FACTORY_H
#define PRINTER_FACTORY_H

#include "printer_interface.h"
#include <memory>

class PrinterFactory
{
public:
    static std::unique_ptr<PrinterInterface> Create();
};

#endif
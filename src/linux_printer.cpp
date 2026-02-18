#include "linux_printer.h"
#include <cups/cups.h>
#include <cups/ppd.h>

std::string LinuxPrinter::GetPrinterStatus(ipp_pstate_t state)
{
    switch (state)
    {
    case IPP_PRINTER_IDLE:
        return "ready";
    case IPP_PRINTER_PROCESSING:
        return "printing";
    case IPP_PRINTER_STOPPED:
        return "stopped";
    default:
        return "unknown";
    }
}

PrinterInfo LinuxPrinter::GetPrinterDetails(const std::string &printerName, bool isDefault)
{
    PrinterInfo info;
    info.name = printerName;
    info.isDefault = isDefault;

    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(printerName.c_str(), NULL, num_dests, dests);

    if (dest != NULL)
    {
        for (int i = 0; i < dest->num_options; i++)
        {
            info.details[dest->options[i].name] = dest->options[i].value;
        }

        http_t *http = httpConnect2(cupsServer(), ippPort(), NULL, AF_UNSPEC,
                                    HTTP_ENCRYPTION_IF_REQUESTED, 1, 30000, NULL);
        if (http != NULL)
        {
            ipp_t *request = ippNewRequest(IPP_OP_GET_PRINTER_ATTRIBUTES);

            char uri[HTTP_MAX_URI];
            httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL,
                             "localhost", 0, "/printers/%s", printerName.c_str());

            ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI,
                         "printer-uri", NULL, uri);

            ipp_t *response = cupsDoRequest(http, request, "/");
            if (response != NULL)
            {
                ipp_attribute_t *attr = ippFindAttribute(response,
                                                         "printer-state", IPP_TAG_ENUM);
                if (attr != NULL)
                {
                    info.status = GetPrinterStatus((ipp_pstate_t)ippGetInteger(attr, 0));
                }

                attr = ippFindAttribute(response, "printer-location", IPP_TAG_TEXT);
                if (attr != NULL)
                    info.details["location"] = ippGetString(attr, 0, NULL);

                attr = ippFindAttribute(response, "printer-info", IPP_TAG_TEXT);
                if (attr != NULL)
                    info.details["comment"] = ippGetString(attr, 0, NULL);

                attr = ippFindAttribute(response, "printer-make-and-model", IPP_TAG_TEXT);
                if (attr != NULL)
                    info.details["driver"] = ippGetString(attr, 0, NULL);

                attr = ippFindAttribute(response, "port", IPP_TAG_TEXT);
                if (attr != NULL)
                    info.details["port"] = ippGetString(attr, 0, NULL);

                ippDelete(response);
            }
            httpClose(http);
        }
    }
    cupsFreeDests(num_dests, dests);
    return info;
}

std::vector<PrinterInfo> LinuxPrinter::GetPrinters()
{
    std::vector<PrinterInfo> printers;
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);

    for (int i = 0; i < num_dests; i++)
    {
        printers.push_back(GetPrinterDetails(dests[i].name, dests[i].is_default));
    }

    cupsFreeDests(num_dests, dests);
    return printers;
}

PrinterInfo LinuxPrinter::GetSystemDefaultPrinter()
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(NULL, NULL, num_dests, dests);

    PrinterInfo printer;
    if (dest != NULL)
    {
        printer = GetPrinterDetails(dest->name, true);
    }

    cupsFreeDests(num_dests, dests);
    return printer;
}

bool LinuxPrinter::PrintDirect(const std::string &printerName,
                               const std::vector<uint8_t> &data,
                               const std::string &dataType)
{
    int jobId = cupsCreateJob(CUPS_HTTP_DEFAULT, printerName.c_str(),
                              "Node.js Print Job", 0, NULL);

    if (jobId <= 0)
        return false;

    http_status_t status = cupsStartDocument(CUPS_HTTP_DEFAULT, printerName.c_str(),
                                             jobId, "Node.js Print Job",
                                             dataType.c_str(), 1);

    if (status != HTTP_STATUS_CONTINUE)
    {
        cupsCancelJob(printerName.c_str(), jobId);
        return false;
    }

    if (cupsWriteRequestData(CUPS_HTTP_DEFAULT,
                             reinterpret_cast<const char *>(data.data()),
                             data.size()) != HTTP_STATUS_CONTINUE)
    {
        cupsCancelJob(printerName.c_str(), jobId);
        return false;
    }

    status = static_cast<http_status_t>(cupsFinishDocument(CUPS_HTTP_DEFAULT, printerName.c_str()));
    return status == HTTP_STATUS_OK;
}

PrinterInfo LinuxPrinter::GetStatusPrinter(const std::string &printerName)
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);

    cups_dest_t *defaultDest = cupsGetDest(NULL, NULL, num_dests, dests);
    bool isDefault = false;

    if (defaultDest != NULL)
    {
        isDefault = (printerName == defaultDest->name);
    }

    cups_dest_t *dest = cupsGetDest(printerName.c_str(), NULL, num_dests, dests);
    PrinterInfo printer;

    if (dest != NULL)
    {
        printer = GetPrinterDetails(printerName, isDefault);
    }

    cupsFreeDests(num_dests, dests);
    return printer;
}
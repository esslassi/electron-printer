#include "windows_printer.h"
#include <vector>

std::string WindowsPrinter::GetPrinterStatus(DWORD status)
{
    if (status & PRINTER_STATUS_OFFLINE)
        return "offline";
    if (status & PRINTER_STATUS_ERROR)
        return "error";
    if (status & PRINTER_STATUS_PAPER_JAM)
        return "paper-jam";
    if (status & PRINTER_STATUS_PAPER_OUT)
        return "paper-out";
    if (status & PRINTER_STATUS_MANUAL_FEED)
        return "manual-feed";
    if (status & PRINTER_STATUS_PAPER_PROBLEM)
        return "paper-problem";
    if (status & PRINTER_STATUS_BUSY)
        return "busy";
    if (status & PRINTER_STATUS_PRINTING)
        return "printing";
    if (status & PRINTER_STATUS_OUTPUT_BIN_FULL)
        return "output-bin-full";
    if (status & PRINTER_STATUS_NOT_AVAILABLE)
        return "not-available";
    if (status & PRINTER_STATUS_WAITING)
        return "waiting";
    if (status & PRINTER_STATUS_PROCESSING)
        return "processing";
    if (status & PRINTER_STATUS_INITIALIZING)
        return "initializing";
    if (status & PRINTER_STATUS_WARMING_UP)
        return "warming-up";
    if (status & PRINTER_STATUS_TONER_LOW)
        return "toner-low";
    if (status & PRINTER_STATUS_NO_TONER)
        return "no-toner";
    if (status & PRINTER_STATUS_PAGE_PUNT)
        return "page-punt";
    if (status & PRINTER_STATUS_USER_INTERVENTION)
        return "user-intervention";
    if (status & PRINTER_STATUS_OUT_OF_MEMORY)
        return "out-of-memory";
    if (status & PRINTER_STATUS_DOOR_OPEN)
        return "door-open";
    return "ready";
}

std::wstring WindowsPrinter::Utf8ToWide(const std::string &str)
{
    std::wstring wstr;
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (len > 0)
    {
        wstr.resize(len - 1);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);
    }
    return wstr;
}

std::string WindowsPrinter::WideToUtf8(LPWSTR wstr)
{
    if (!wstr)
        return "";

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len <= 0)
        return "";

    std::vector<char> buffer(len);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buffer.data(), len, NULL, NULL);
    return std::string(buffer.data());
}

PrinterInfo WindowsPrinter::GetPrinterDetails(const std::string &printerName, bool isDefault)
{
    PrinterInfo info;
    info.name = printerName;
    info.isDefault = isDefault;

    HANDLE hPrinter;
    std::wstring wPrinterName = Utf8ToWide(printerName);

    if (OpenPrinterW((LPWSTR)wPrinterName.c_str(), &hPrinter, NULL))
    {
        DWORD needed;
        GetPrinterW(hPrinter, 2, NULL, 0, &needed);
        if (needed > 0)
        {
            std::vector<BYTE> buffer(needed);
            if (GetPrinterW(hPrinter, 2, buffer.data(), needed, &needed))
            {
                PRINTER_INFO_2W *pInfo = (PRINTER_INFO_2W *)buffer.data();
                info.status = GetPrinterStatus(pInfo->Status);

                if (pInfo->pLocation)
                    info.details["location"] = WideToUtf8(pInfo->pLocation);
                if (pInfo->pComment)
                    info.details["comment"] = WideToUtf8(pInfo->pComment);
                if (pInfo->pDriverName)
                    info.details["driver"] = WideToUtf8(pInfo->pDriverName);
                if (pInfo->pPortName)
                    info.details["port"] = WideToUtf8(pInfo->pPortName);
            }
        }
        ClosePrinter(hPrinter);
    }

    return info;
}

std::vector<PrinterInfo> WindowsPrinter::GetPrinters()
{
    std::vector<PrinterInfo> printers;
    DWORD needed = 0, returned = 0;

    EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &needed, &returned);
    if (needed > 0)
    {
        std::vector<BYTE> buffer(needed);
        if (EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, buffer.data(), needed, &needed, &returned))
        {
            PRINTER_INFO_2W *pInfo = (PRINTER_INFO_2W *)buffer.data();
            for (DWORD i = 0; i < returned; i++)
            {
                std::string name = WideToUtf8(pInfo[i].pPrinterName);
                bool isDefault = false;

                wchar_t defaultPrinter[256];
                DWORD size = sizeof(defaultPrinter) / sizeof(defaultPrinter[0]);
                if (GetDefaultPrinterW(defaultPrinter, &size))
                {
                    isDefault = (wcscmp(pInfo[i].pPrinterName, defaultPrinter) == 0);
                }

                printers.push_back(GetPrinterDetails(name, isDefault));
            }
        }
    }

    return printers;
}

PrinterInfo WindowsPrinter::GetSystemDefaultPrinter()
{
    wchar_t printerName[256];
    DWORD size = sizeof(printerName) / sizeof(printerName[0]);

    if (GetDefaultPrinterW(printerName, &size))
    {
        std::string name = WideToUtf8(printerName);
        return GetPrinterDetails(name, true);
    }

    return PrinterInfo();
}

bool WindowsPrinter::PrintDirect(const std::string &printerName, const std::vector<uint8_t> &data, const std::string &dataType)
{
    HANDLE hPrinter;
    std::wstring wPrinterName = Utf8ToWide(printerName);
    std::wstring wDataType = Utf8ToWide(dataType);

    if (!OpenPrinterW((LPWSTR)wPrinterName.c_str(), &hPrinter, NULL))
    {
        return false;
    }

    DOC_INFO_1W docInfo;
    wchar_t docName[] = L"Node.js Print Job";
    docInfo.pDocName = docName;
    docInfo.pOutputFile = NULL;
    docInfo.pDatatype = const_cast<LPWSTR>(wDataType.c_str());

    if (StartDocPrinterW(hPrinter, 1, (LPBYTE)&docInfo))
    {
        if (StartPagePrinter(hPrinter))
        {
            DWORD bytesWritten;
            void *buffer = const_cast<void *>(static_cast<const void *>(data.data()));
            if (WritePrinter(hPrinter, buffer, static_cast<DWORD>(data.size()), &bytesWritten))
            {
                EndPagePrinter(hPrinter);
                EndDocPrinter(hPrinter);
                ClosePrinter(hPrinter);
                return true;
            }
        }
        EndDocPrinter(hPrinter);
    }

    ClosePrinter(hPrinter);
    return false;
}

PrinterInfo WindowsPrinter::GetStatusPrinter(const std::string &printerName)
{
    wchar_t defaultPrinter[256];
    DWORD size = sizeof(defaultPrinter) / sizeof(defaultPrinter[0]);
    bool isDefault = false;

    if (GetDefaultPrinterW(defaultPrinter, &size))
    {
        std::string defaultPrinterName = WideToUtf8(defaultPrinter);
        isDefault = (printerName == defaultPrinterName);
    }

    PrinterInfo printer = GetPrinterDetails(printerName, isDefault);
    return printer;
}

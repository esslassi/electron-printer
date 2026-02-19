#include "windows_printer.h"

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>

static std::string ToUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::toupper(c); });
    return s;
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

static std::time_t SystemTimeToTimeT(const SYSTEMTIME &st)
{
    FILETIME ft;
    if (!SystemTimeToFileTime(&st, &ft)) return 0;

    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    // Windows FILETIME is 100-ns since Jan 1, 1601 (UTC)
    // Convert to Unix epoch (seconds since Jan 1, 1970)
    const unsigned long long EPOCH_DIFFERENCE = 116444736000000000ULL; // 100ns
    if (ull.QuadPart < EPOCH_DIFFERENCE) return 0;

    unsigned long long unix100ns = ull.QuadPart - EPOCH_DIFFERENCE;
    return (std::time_t)(unix100ns / 10000000ULL);
}

std::vector<std::string> WindowsPrinter::MapJobStatus(DWORD status)
{
    std::vector<std::string> out;

    if (status & JOB_STATUS_PAUSED) out.push_back("PAUSED");
    if (status & JOB_STATUS_PRINTING) out.push_back("PRINTING");
    if (status & JOB_STATUS_SPOOLING) out.push_back("PENDING");
    if (status & JOB_STATUS_DELETING) out.push_back("CANCELLED");
    if (status & JOB_STATUS_DELETED) out.push_back("CANCELLED");
    if (status & JOB_STATUS_ERROR) out.push_back("ABORTED");
    if (status & JOB_STATUS_OFFLINE) out.push_back("PENDING");
    if (status & JOB_STATUS_PAPEROUT) out.push_back("PENDING");
    if (status & JOB_STATUS_PRINTED) out.push_back("PRINTED");

    if (out.empty()) out.push_back("PENDING");
    return out;
}

static bool ReadAllBytes(const std::string &path, std::vector<uint8_t> &out)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    std::streamsize size = f.tellg();
    if (size < 0) return false;
    f.seekg(0, std::ios::beg);

    out.resize((size_t)size);
    if (size > 0)
        f.read((char*)out.data(), size);

    return true;
}

std::vector<PrinterDetailsNative> WindowsPrinter::GetPrinters()
{
    std::vector<PrinterDetailsNative> printers;

    DWORD needed = 0, returned = 0;
    EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &needed, &returned);
    if (needed == 0) return printers;

    std::vector<BYTE> buffer(needed);
    if (!EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, buffer.data(), needed, &needed, &returned))
        return printers;

    wchar_t defaultPrinter[512];
    DWORD defaultSize = (DWORD)(sizeof(defaultPrinter) / sizeof(defaultPrinter[0]));
    std::string defaultName;
    if (GetDefaultPrinterW(defaultPrinter, &defaultSize))
        defaultName = WideToUtf8(defaultPrinter);

    PRINTER_INFO_2W *pInfo = (PRINTER_INFO_2W *)buffer.data();

    for (DWORD i = 0; i < returned; i++)
    {
        PrinterDetailsNative p;
        p.name = WideToUtf8(pInfo[i].pPrinterName);
        p.isDefault = (!defaultName.empty() && p.name == defaultName);

        // options: keep it clean and stable
        if (pInfo[i].pLocation) p.options["location"] = WideToUtf8(pInfo[i].pLocation);
        if (pInfo[i].pComment) p.options["comment"] = WideToUtf8(pInfo[i].pComment);
        if (pInfo[i].pDriverName) p.options["driver"] = WideToUtf8(pInfo[i].pDriverName);
        if (pInfo[i].pPortName) p.options["port"] = WideToUtf8(pInfo[i].pPortName);

        printers.push_back(std::move(p));
    }

    return printers;
}

PrinterDetailsNative WindowsPrinter::GetPrinter(const std::string &printerName)
{
    // Build from GetPrinters (stable + consistent)
    auto list = GetPrinters();
    for (auto &p : list)
    {
        if (p.name == printerName) return p;
    }

    PrinterDetailsNative p;
    p.name = printerName;
    p.isDefault = false;
    return p;
}

std::string WindowsPrinter::GetDefaultPrinterName()
{
    wchar_t printerName[512];
    DWORD size = (DWORD)(sizeof(printerName) / sizeof(printerName[0]));
    if (GetDefaultPrinterW(printerName, &size))
        return WideToUtf8(printerName);
    return "";
}

DriverOptions WindowsPrinter::GetPrinterDriverOptions(const std::string &printerName)
{
    // Windows driver option enumeration is complex (DEVMODE / DocumentProperties).
    // For now: return empty, still satisfies your TS typing.
    // You can enhance later with DEVMODE-driven options.
    (void)printerName;
    return DriverOptions{};
}

std::string WindowsPrinter::GetSelectedPaperSize(const std::string &printerName)
{
    HANDLE hPrinter = NULL;
    std::wstring wName = Utf8ToWide(printerName);

    if (!OpenPrinterW((LPWSTR)wName.c_str(), &hPrinter, NULL))
        return "";

    // Query DEVMODE size
    LONG dmSize = DocumentPropertiesW(NULL, hPrinter, (LPWSTR)wName.c_str(), NULL, NULL, 0);
    if (dmSize <= 0)
    {
        ClosePrinter(hPrinter);
        return "";
    }

    std::vector<BYTE> dmBuffer((size_t)dmSize);
    DEVMODEW *dm = (DEVMODEW *)dmBuffer.data();

    LONG res = DocumentPropertiesW(NULL, hPrinter, (LPWSTR)wName.c_str(), dm, NULL, DM_OUT_BUFFER);
    if (res != IDOK)
    {
        ClosePrinter(hPrinter);
        return "";
    }

    std::string paper;

    if ((dm->dmFields & DM_FORMNAME) && dm->dmFormName[0] != L'\0')
    {
        paper = WideToUtf8(dm->dmFormName);
    }
    else if (dm->dmFields & DM_PAPERSIZE)
    {
        // Fallback: return numeric code as string (still useful)
        paper = std::to_string((int)dm->dmPaperSize);
    }

    ClosePrinter(hPrinter);
    return paper;
}

std::vector<std::string> WindowsPrinter::GetSupportedPrintFormats()
{
    // On Windows spooler, “RAW” is always safe. PDF/JPEG/PS conversion is not guaranteed without extra pipeline.
    return { "RAW", "TEXT", "COMMAND" };
}

int WindowsPrinter::PrintDirect(const std::string &printerName,
                                const std::vector<uint8_t> &data,
                                const std::string &type,
                                const StringMap &options)
{
    (void)options;

    HANDLE hPrinter = NULL;
    std::wstring wPrinterName = Utf8ToWide(printerName);

    if (!OpenPrinterW((LPWSTR)wPrinterName.c_str(), &hPrinter, NULL))
        return 0;

    std::string t = ToUpper(type);
    // Windows expects a spool datatype string. For our types:
    // RAW/TEXT/COMMAND => use RAW to send bytes directly.
    std::wstring wDataType = Utf8ToWide("RAW");

    DOC_INFO_1W docInfo;
    wchar_t docName[] = L"Node Print Job";
    docInfo.pDocName = docName;
    docInfo.pOutputFile = NULL;
    docInfo.pDatatype = (LPWSTR)wDataType.c_str();

    DWORD jobId = StartDocPrinterW(hPrinter, 1, (LPBYTE)&docInfo);
    if (jobId == 0)
    {
        ClosePrinter(hPrinter);
        return 0;
    }

    if (!StartPagePrinter(hPrinter))
    {
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        return 0;
    }

    DWORD bytesWritten = 0;
    BOOL ok = WritePrinter(hPrinter, (LPVOID)data.data(), (DWORD)data.size(), &bytesWritten);

    EndPagePrinter(hPrinter);
    EndDocPrinter(hPrinter);
    ClosePrinter(hPrinter);

    if (!ok || bytesWritten != (DWORD)data.size())
        return 0;

    return (int)jobId;
}

int WindowsPrinter::PrintFile(const std::string &printerName,
                              const std::string &filename)
{
    std::vector<uint8_t> data;
    if (!ReadAllBytes(filename, data))
        return 0;

    // PrintFile typing does not include type, so we treat it as RAW bytes.
    StringMap emptyOpts;
    return PrintDirect(printerName, data, "RAW", emptyOpts);
}

JobDetailsNative WindowsPrinter::GetJob(const std::string &printerName, int jobId)
{
    JobDetailsNative j;
    j.id = jobId;
    j.printerName = printerName;

    HANDLE hPrinter = NULL;
    std::wstring wPrinterName = Utf8ToWide(printerName);
    if (!OpenPrinterW((LPWSTR)wPrinterName.c_str(), &hPrinter, NULL))
        return j;

    DWORD needed = 0;
    GetJobW(hPrinter, (DWORD)jobId, 2, NULL, 0, &needed);
    if (needed == 0)
    {
        ClosePrinter(hPrinter);
        return j;
    }

    std::vector<BYTE> buffer(needed);
    if (!GetJobW(hPrinter, (DWORD)jobId, 2, buffer.data(), needed, &needed))
    {
        ClosePrinter(hPrinter);
        return j;
    }

    JOB_INFO_2W *ji = (JOB_INFO_2W *)buffer.data();

    j.name = ji->pDocument ? WideToUtf8(ji->pDocument) : "";
    j.user = ji->pUserName ? WideToUtf8(ji->pUserName) : "";
    j.format = "RAW";
    j.priority = (int)ji->Priority;
    j.size = (int)ji->Size;
    j.status = MapJobStatus(ji->Status);

    // Times
    j.creationTime = SystemTimeToTimeT(ji->Submitted);
    j.processingTime = j.creationTime;
    j.completedTime = 0;

    ClosePrinter(hPrinter);
    return j;
}

void WindowsPrinter::SetJob(const std::string &printerName, int jobId, const std::string &command)
{
    HANDLE hPrinter = NULL;
    std::wstring wPrinterName = Utf8ToWide(printerName);
    if (!OpenPrinterW((LPWSTR)wPrinterName.c_str(), &hPrinter, NULL))
        return;

    std::string cmd = ToUpper(command);

    if (cmd == "CANCEL")
    {
        SetJobW(hPrinter, (DWORD)jobId, 0, NULL, JOB_CONTROL_CANCEL);
    }
    else if (cmd == "PAUSE")
    {
        SetJobW(hPrinter, (DWORD)jobId, 0, NULL, JOB_CONTROL_PAUSE);
    }
    else if (cmd == "RESUME")
    {
        SetJobW(hPrinter, (DWORD)jobId, 0, NULL, JOB_CONTROL_RESUME);
    }

    ClosePrinter(hPrinter);
}

std::vector<std::string> WindowsPrinter::GetSupportedJobCommands()
{
    return { "CANCEL", "PAUSE", "RESUME" };
}
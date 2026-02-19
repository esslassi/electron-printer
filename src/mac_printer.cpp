#include "mac_printer.h"

#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/ipp.h>
#include <cups/http.h>

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <cstring>

/* =========================================================
   Helpers
========================================================= */

static std::string ToUpper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return (char)std::toupper(c); });
    return s;
}

static std::vector<uint8_t> ReadAllBytes(const std::string &path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};

    f.seekg(0, std::ios::end);
    std::streamsize size = f.tellg();
    if (size < 0) return {};

    f.seekg(0, std::ios::beg);

    std::vector<uint8_t> buf((size_t)size);
    if (size > 0)
        f.read((char*)buf.data(), size);

    return buf;
}

/* =========================================================
   Printer Listing
========================================================= */

std::vector<PrinterDetailsNative> MacPrinter::GetPrinters()
{
    std::vector<PrinterDetailsNative> out;

    cups_dest_t *dests = nullptr;
    int num = cupsGetDests(&dests);

    for (int i = 0; i < num; i++)
    {
        PrinterDetailsNative p;
        p.name = dests[i].name ? dests[i].name : "";
        p.isDefault = dests[i].is_default != 0;

        for (int k = 0; k < dests[i].num_options; k++)
        {
            if (dests[i].options[k].name && dests[i].options[k].value)
                p.options[dests[i].options[k].name] = dests[i].options[k].value;
        }

        out.push_back(std::move(p));
    }

    cupsFreeDests(num, dests);
    return out;
}

PrinterDetailsNative MacPrinter::GetPrinter(const std::string &printerName)
{
    auto list = GetPrinters();
    for (auto &p : list)
        if (p.name == printerName)
            return p;

    PrinterDetailsNative p;
    p.name = printerName;
    p.isDefault = false;
    return p;
}

std::string MacPrinter::GetDefaultPrinterName()
{
    cups_dest_t *dests = nullptr;
    int num = cupsGetDests(&dests);

    cups_dest_t *def = cupsGetDest(NULL, NULL, num, dests);

    std::string name;
    if (def && def->name)
        name = def->name;

    cupsFreeDests(num, dests);
    return name;
}

/* =========================================================
   Driver Options / Paper
========================================================= */

DriverOptions MacPrinter::GetPrinterDriverOptions(const std::string &printerName)
{
    DriverOptions out;

    const char *ppdPath = cupsGetPPD(printerName.c_str());
    if (!ppdPath) return out;

    ppd_file_t *ppd = ppdOpenFile(ppdPath);
    if (!ppd)
    {
        unlink(ppdPath);
        return out;
    }

    ppdMarkDefaults(ppd);

    for (ppd_option_t *opt = ppd->options; opt; opt = opt->next)
    {
        std::map<std::string, bool> choices;

        for (int i = 0; i < opt->num_choices; i++)
        {
            bool isDefault =
                (opt->defchoice &&
                 strcmp(opt->defchoice, opt->choices[i].choice) == 0);

            choices[opt->choices[i].choice] = isDefault;
        }

        out[opt->keyword ? opt->keyword : ""] = choices;
    }

    ppdClose(ppd);
    unlink(ppdPath);

    return out;
}

std::string MacPrinter::GetSelectedPaperSize(const std::string &printerName)
{
    std::string paper;

    const char *ppdPath = cupsGetPPD(printerName.c_str());
    if (!ppdPath) return paper;

    ppd_file_t *ppd = ppdOpenFile(ppdPath);
    if (!ppd)
    {
        unlink(ppdPath);
        return paper;
    }

    ppdMarkDefaults(ppd);

    ppd_option_t *opt = ppdFindOption(ppd, "PageSize");
    if (!opt)
        opt = ppdFindOption(ppd, "PageRegion");

    if (opt && opt->defchoice)
        paper = opt->defchoice;

    ppdClose(ppd);
    unlink(ppdPath);

    return paper;
}

/* =========================================================
   Capabilities
========================================================= */

std::vector<std::string> MacPrinter::GetSupportedPrintFormats()
{
    return { "RAW", "TEXT", "PDF", "JPEG", "POSTSCRIPT", "COMMAND", "AUTO" };
}

/* =========================================================
   Printing
========================================================= */

int MacPrinter::PrintDirect(const std::string &printerName,
                            const std::vector<uint8_t> &data,
                            const std::string &type,
                            const StringMap &options)
{
    std::string t = ToUpper(type);

    // For PDF/JPEG/POSTSCRIPT -> use temp file + cupsPrintFile
    if (t == "PDF" || t == "JPEG" || t == "POSTSCRIPT")
    {
        char tmpName[] = "/tmp/esslassi_print_XXXXXX";
        int fd = mkstemp(tmpName);
        if (fd < 0)
            return 0;

        FILE *fp = fdopen(fd, "wb");
        if (!fp)
        {
            close(fd);
            return 0;
        }

        fwrite(data.data(), 1, data.size(), fp);
        fclose(fp);

        cups_option_t *cupOpts = nullptr;
        int num = 0;

        for (auto &kv : options)
            num = cupsAddOption(kv.first.c_str(),
                                kv.second.c_str(),
                                num,
                                &cupOpts);

        int jobId = cupsPrintFile(
            printerName.c_str(),
            tmpName,
            "Node Print Job",
            num,
            cupOpts);

        if (cupOpts)
            cupsFreeOptions(num, cupOpts);

        unlink(tmpName);

        return jobId > 0 ? jobId : 0;
    }

    // RAW / TEXT / COMMAND
    int jobId = cupsCreateJob(
        CUPS_HTTP_DEFAULT,
        printerName.c_str(),
        "Node Print Job",
        0,
        NULL);

    if (jobId <= 0)
        return 0;

    http_status_t st = cupsStartDocument(
        CUPS_HTTP_DEFAULT,
        printerName.c_str(),
        jobId,
        "Node Print Job",
        CUPS_FORMAT_RAW,
        1);

    if (st != HTTP_STATUS_CONTINUE)
    {
        cupsCancelJob(printerName.c_str(), jobId);
        return 0;
    }

    if (cupsWriteRequestData(
            CUPS_HTTP_DEFAULT,
            (const char*)data.data(),
            data.size()) != HTTP_STATUS_CONTINUE)
    {
        cupsCancelJob(printerName.c_str(), jobId);
        return 0;
    }

    ipp_status_t fin =
        (ipp_status_t)cupsFinishDocument(
            CUPS_HTTP_DEFAULT,
            printerName.c_str());

    if (fin > IPP_STATUS_OK_CONFLICT)
        return 0;

    return jobId;
}

int MacPrinter::PrintFile(const std::string &printerName,
                          const std::string &filename)
{
    int jobId = cupsPrintFile(
        printerName.c_str(),
        filename.c_str(),
        "Node Print Job",
        0,
        NULL);

    return jobId > 0 ? jobId : 0;
}

/* =========================================================
   Job Management
========================================================= */

JobDetailsNative MacPrinter::GetJob(const std::string &printerName, int jobId)
{
    JobDetailsNative j;
    j.id = jobId;
    j.printerName = printerName;

    cups_job_t *jobs = nullptr;
    int num = cupsGetJobs(
        &jobs,
        printerName.c_str(),
        0,
        CUPS_WHICHJOBS_ALL);

    for (int i = 0; i < num; i++)
    {
        if (jobs[i].id == jobId)
        {
            j.name = jobs[i].title ? jobs[i].title : "";
            j.user = jobs[i].user ? jobs[i].user : "";
            j.format = jobs[i].format ? jobs[i].format : "";
            j.priority = jobs[i].priority;
            j.size = jobs[i].size;

            switch (jobs[i].state)
            {
                case IPP_JSTATE_PENDING:    j.status = { "PENDING" }; break;
                case IPP_JSTATE_HELD:       j.status = { "PAUSED" }; break;
                case IPP_JSTATE_PROCESSING: j.status = { "PRINTING" }; break;
                case IPP_JSTATE_STOPPED:    j.status = { "ABORTED" }; break;
                case IPP_JSTATE_CANCELED:   j.status = { "CANCELLED" }; break;
                case IPP_JSTATE_ABORTED:    j.status = { "ABORTED" }; break;
                case IPP_JSTATE_COMPLETED:  j.status = { "PRINTED" }; break;
                default:                    j.status = { "PENDING" }; break;
            }

            j.creationTime = jobs[i].creation_time;
            j.processingTime = jobs[i].processing_time;
            j.completedTime = jobs[i].completed_time;

            break;
        }
    }

    cupsFreeJobs(num, jobs);
    return j;
}

void MacPrinter::SetJob(const std::string &printerName,
                        int jobId,
                        const std::string &command)
{
    std::string cmd = ToUpper(command);

    if (cmd == "CANCEL")
    {
        cupsCancelJob(printerName.c_str(), jobId);
        return;
    }

    if (cmd == "PAUSE" || cmd == "HOLD")
    {
        cupsHoldJob(printerName.c_str(), jobId);
        return;
    }

    if (cmd == "RESUME" || cmd == "RELEASE")
    {
        cupsReleaseJob(printerName.c_str(), jobId);
        return;
    }
}

std::vector<std::string> MacPrinter::GetSupportedJobCommands()
{
    return { "CANCEL", "PAUSE", "RESUME" };
}
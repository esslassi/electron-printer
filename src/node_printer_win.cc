#include "node_printer.hpp"
#if _MSC_VER
#include <windows.h>
#include <Winspool.h>
#include <Wingdi.h>
#pragma  comment(lib, "Winspool.lib")
#else
#error "Unsupported compiler for windows. Feel free to add it."
#endif

#include <string>
#include <map>
#include <utility>
#include <sstream>
#include <cstdio>

// Utility function to output messages
void ConsolePrint(const std::string& message) {
    printf("%s\n", message.c_str());
    fflush(stdout); // Ensure the output is flushed immediately
}

// Function to convert std::u16string to std::wstring
std::wstring u16stringToWString(const std::u16string& u16str) {
    return std::wstring(u16str.begin(), u16str.end());
}

namespace{
    typedef std::map<std::string, DWORD> StatusMapType;

    /** Memory value class management to avoid memory leak
    */
    template<typename Type>
    class MemValue: public MemValueBase<Type> {
    public:
        /** Constructor of allocating iSizeKbytes bytes memory;
        * @param iSizeKbytes size in bytes of required allocating memory
        */
        MemValue(const DWORD iSizeKbytes) {
            _value = (Type*)malloc(iSizeKbytes);
        }
		
        ~MemValue () {
            free();
        }

        Type* get() {
        return _value;
        }
    protected:
        virtual void free() {
            if(_value != NULL)
            {
                ::free(_value);
                _value = NULL;
            }
        }

    private:
    Type* _value; // Declaration of the member variable
    };

        struct PrinterHandle
    {
        PrinterHandle(LPWSTR iPrinterName)
        {
            _ok = OpenPrinterW(iPrinterName, &_printer, NULL);
        }
        ~PrinterHandle()
        {
            if(_ok)
            {
                ClosePrinter(_printer);
            }
        }
        operator HANDLE() {return _printer;}
        operator bool() { return (!!_ok);}
        HANDLE& operator *() { return _printer;}
        HANDLE* operator ->() { return &_printer;}
        const HANDLE& operator ->() const { return _printer;}
        HANDLE _printer;
        BOOL _ok;
    };

    const StatusMapType& getStatusMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define STATUS_PRINTER_ADD(value, type) result.insert(std::make_pair(value, type))
        STATUS_PRINTER_ADD("BUSY", PRINTER_STATUS_BUSY);
        STATUS_PRINTER_ADD("DOOR-OPEN", PRINTER_STATUS_DOOR_OPEN);
        STATUS_PRINTER_ADD("ERROR", PRINTER_STATUS_ERROR);
        STATUS_PRINTER_ADD("INITIALIZING", PRINTER_STATUS_INITIALIZING);
        STATUS_PRINTER_ADD("IO-ACTIVE", PRINTER_STATUS_IO_ACTIVE);
        STATUS_PRINTER_ADD("MANUAL-FEED", PRINTER_STATUS_MANUAL_FEED);
        STATUS_PRINTER_ADD("NO-TONER", PRINTER_STATUS_NO_TONER);
        STATUS_PRINTER_ADD("NOT-AVAILABLE", PRINTER_STATUS_NOT_AVAILABLE);
        STATUS_PRINTER_ADD("OFFLINE", PRINTER_STATUS_OFFLINE);
        STATUS_PRINTER_ADD("OUT-OF-MEMORY", PRINTER_STATUS_OUT_OF_MEMORY);
        STATUS_PRINTER_ADD("OUTPUT-BIN-FULL", PRINTER_STATUS_OUTPUT_BIN_FULL);
        STATUS_PRINTER_ADD("PAGE-PUNT", PRINTER_STATUS_PAGE_PUNT);
        STATUS_PRINTER_ADD("PAPER-JAM", PRINTER_STATUS_PAPER_JAM);
        STATUS_PRINTER_ADD("PAPER-OUT", PRINTER_STATUS_PAPER_OUT);
        STATUS_PRINTER_ADD("PAPER-PROBLEM", PRINTER_STATUS_PAPER_PROBLEM);
        STATUS_PRINTER_ADD("PAUSED", PRINTER_STATUS_PAUSED);
        STATUS_PRINTER_ADD("PENDING-DELETION", PRINTER_STATUS_PENDING_DELETION);
        STATUS_PRINTER_ADD("POWER-SAVE", PRINTER_STATUS_POWER_SAVE);
        STATUS_PRINTER_ADD("PRINTING", PRINTER_STATUS_PRINTING);
        STATUS_PRINTER_ADD("PROCESSING", PRINTER_STATUS_PROCESSING);
        STATUS_PRINTER_ADD("SERVER-UNKNOWN", PRINTER_STATUS_SERVER_UNKNOWN);
        STATUS_PRINTER_ADD("TONER-LOW", PRINTER_STATUS_TONER_LOW);
        STATUS_PRINTER_ADD("USER-INTERVENTION", PRINTER_STATUS_USER_INTERVENTION);
        STATUS_PRINTER_ADD("WAITING", PRINTER_STATUS_WAITING);
        STATUS_PRINTER_ADD("WARMING-UP", PRINTER_STATUS_WARMING_UP);
#undef STATUS_PRINTER_ADD
        return result;
    }

    const StatusMapType& getJobStatusMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define STATUS_PRINTER_ADD(value, type) result.insert(std::make_pair(value, type))
        // Common statuses
        STATUS_PRINTER_ADD("PRINTING", JOB_STATUS_PRINTING);
        STATUS_PRINTER_ADD("PRINTED", JOB_STATUS_PRINTED);
        STATUS_PRINTER_ADD("PAUSED", JOB_STATUS_PAUSED);

        // Specific statuses
        STATUS_PRINTER_ADD("BLOCKED-DEVQ", JOB_STATUS_BLOCKED_DEVQ);
        STATUS_PRINTER_ADD("DELETED", JOB_STATUS_DELETED);
        STATUS_PRINTER_ADD("DELETING", JOB_STATUS_DELETING);
        STATUS_PRINTER_ADD("ERROR", JOB_STATUS_ERROR);
        STATUS_PRINTER_ADD("OFFLINE", JOB_STATUS_OFFLINE);
        STATUS_PRINTER_ADD("PAPEROUT", JOB_STATUS_PAPEROUT);
        STATUS_PRINTER_ADD("RESTART", JOB_STATUS_RESTART);
        STATUS_PRINTER_ADD("SPOOLING", JOB_STATUS_SPOOLING);
        STATUS_PRINTER_ADD("USER-INTERVENTION", JOB_STATUS_USER_INTERVENTION);
        // XP and later
#ifdef JOB_STATUS_COMPLETE
        STATUS_PRINTER_ADD("COMPLETE", JOB_STATUS_COMPLETE);
#endif
#ifdef JOB_STATUS_RETAINED
        STATUS_PRINTER_ADD("RETAINED", JOB_STATUS_RETAINED);
#endif

#undef STATUS_PRINTER_ADD
        return result;
    }

    const StatusMapType& getAttributeMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define ATTRIBUTE_PRINTER_ADD(value, type) result.insert(std::make_pair(value, type))
        ATTRIBUTE_PRINTER_ADD("DIRECT", PRINTER_ATTRIBUTE_DIRECT);
        ATTRIBUTE_PRINTER_ADD("DO-COMPLETE-FIRST", PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST);
        ATTRIBUTE_PRINTER_ADD("ENABLE-DEVQ", PRINTER_ATTRIBUTE_ENABLE_DEVQ);
        ATTRIBUTE_PRINTER_ADD("HIDDEN", PRINTER_ATTRIBUTE_HIDDEN);
        ATTRIBUTE_PRINTER_ADD("KEEPPRINTEDJOBS", PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS);
        ATTRIBUTE_PRINTER_ADD("LOCAL", PRINTER_ATTRIBUTE_LOCAL);
        ATTRIBUTE_PRINTER_ADD("NETWORK", PRINTER_ATTRIBUTE_NETWORK);
        ATTRIBUTE_PRINTER_ADD("PUBLISHED", PRINTER_ATTRIBUTE_PUBLISHED);
        ATTRIBUTE_PRINTER_ADD("QUEUED", PRINTER_ATTRIBUTE_QUEUED);
        ATTRIBUTE_PRINTER_ADD("RAW-ONLY", PRINTER_ATTRIBUTE_RAW_ONLY);
        ATTRIBUTE_PRINTER_ADD("SHARED", PRINTER_ATTRIBUTE_SHARED);
        ATTRIBUTE_PRINTER_ADD("OFFLINE", PRINTER_ATTRIBUTE_WORK_OFFLINE);
        // XP
#ifdef PRINTER_ATTRIBUTE_FAX
        ATTRIBUTE_PRINTER_ADD("FAX", PRINTER_ATTRIBUTE_FAX);
#endif
        // vista
#ifdef PRINTER_ATTRIBUTE_FRIENDLY_NAME
        ATTRIBUTE_PRINTER_ADD("FRIENDLY-NAME", PRINTER_ATTRIBUTE_FRIENDLY_NAME);
        ATTRIBUTE_PRINTER_ADD("MACHINE", PRINTER_ATTRIBUTE_MACHINE);
        ATTRIBUTE_PRINTER_ADD("PUSHED-USER", PRINTER_ATTRIBUTE_PUSHED_USER);
        ATTRIBUTE_PRINTER_ADD("PUSHED-MACHINE", PRINTER_ATTRIBUTE_PUSHED_MACHINE);
#endif
        // server 2003
#ifdef PRINTER_ATTRIBUTE_TS
        ATTRIBUTE_PRINTER_ADD("TS", PRINTER_ATTRIBUTE_TS);
#endif
#undef ATTRIBUTE_PRINTER_ADD
        return result;
    }

    const StatusMapType& getJobCommandMap()
    {
        static StatusMapType result;
        if(!result.empty())
        {
            return result;
        }
        // add only first time
#define COMMAND_JOB_ADD(value, type) result.insert(std::make_pair(value, type))
        COMMAND_JOB_ADD("CANCEL", JOB_CONTROL_CANCEL);
        COMMAND_JOB_ADD("PAUSE", JOB_CONTROL_PAUSE);
        COMMAND_JOB_ADD("RESTART", JOB_CONTROL_RESTART);
        COMMAND_JOB_ADD("RESUME", JOB_CONTROL_RESUME);
        COMMAND_JOB_ADD("DELETE", JOB_CONTROL_DELETE);
        COMMAND_JOB_ADD("SENT-TO-PRINTER", JOB_CONTROL_SENT_TO_PRINTER);
        COMMAND_JOB_ADD("LAST-PAGE-EJECTED", JOB_CONTROL_LAST_PAGE_EJECTED);
#ifdef JOB_CONTROL_RETAIN
        COMMAND_JOB_ADD("RETAIN", JOB_CONTROL_RETAIN);
#endif
#ifdef JOB_CONTROL_RELEASE
        COMMAND_JOB_ADD("RELEASE", JOB_CONTROL_RELEASE);
#endif
#undef COMMAND_JOB_ADD
        return result;
    }

    void parseJobObject(JOB_INFO_2W *job, Napi::Object result_printer_job)
    {
        Napi::Env env = result_printer_job.Env(); // info.Env();
        // Common fields
        ADD_NAPI_NUMBER_PROPERTY(result_printer_job, "id", job->JobId);
        // Example usage of the string property macro
        ADD_NAPI_STRING_PROPERTY(result_printer_job, "printerName", pPrinterName);
        ADD_NAPI_STRING_PROPERTY(result_printer_job, "user", pUserName);
        ADD_NAPI_STRING_PROPERTY(result_printer_job, "format", pDatatype);
        result_printer_job.Set(Napi::String::New(env, "priority"), Napi::Number::New(env, job->Priority));
        result_printer_job.Set(Napi::String::New(env, "size"), Napi::Number::New(env, job->Size));
        Napi::Array result_printer_job_status = Napi::Array::New(env);
        int i_status = 0;
        for (const auto &itStatus : getJobStatusMap())
        {
            if (job->Status & itStatus.second)
            {
                result_printer_job_status.Set(i_status++, Napi::String::New(env, itStatus.first.c_str()));
            }
        }
        if ((job->pStatus != NULL) && (*job->pStatus != L'\0'))
        {
            result_printer_job_status.Set(i_status++, Napi::String::New(env, (char16_t *)job->pStatus));
        }
        result_printer_job.Set(Napi::String::New(env, "status"), result_printer_job_status);
        // Specific fields
         ADD_NAPI_STRING_PROPERTY(result_printer_job, "machineName", pMachineName);
         ADD_NAPI_STRING_PROPERTY(result_printer_job, "document", pDocument);
         ADD_NAPI_STRING_PROPERTY(result_printer_job, "notifyName", pNotifyName);
         ADD_NAPI_STRING_PROPERTY(result_printer_job, "printProcessor", pPrintProcessor);
         ADD_NAPI_STRING_PROPERTY(result_printer_job, "parameters", pParameters);
         ADD_NAPI_STRING_PROPERTY(result_printer_job, "driverName", pDriverName);
         result_printer_job.Set(Napi::String::New(env, "position"), Napi::Number::New(env, job->Position));
         result_printer_job.Set(Napi::String::New(env, "startTime"), Napi::Number::New(env, job->StartTime));
         result_printer_job.Set(Napi::String::New(env, "untilTime"), Napi::Number::New(env, job->UntilTime));
         result_printer_job.Set(Napi::String::New(env, "totalPages"), Napi::Number::New(env, job->TotalPages));
         result_printer_job.Set(Napi::String::New(env, "time"), Napi::Number::New(env, job->Time));
         result_printer_job.Set(Napi::String::New(env, "pagesPrinted"), Napi::Number::New(env, job->PagesPrinted));

    }
    std::string getLastErrorCodeAndMessage() {
    	std::ostringstream s;
    	DWORD erroCode = GetLastError();
    	s << "code: " << erroCode;
    	DWORD retSize;
    	LPTSTR pTemp = NULL;
    	retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                                FORMAT_MESSAGE_FROM_SYSTEM|
                                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL,
                                erroCode,
                                LANG_NEUTRAL,
                                (LPTSTR)&pTemp,
                                0,
                                NULL );
        if (retSize && pTemp != NULL) {
	    //pTemp[strlen(pTemp)-2]='\0'; //remove cr and newline character
	    //TODO: check if it is needed to convert c string to std::string
	    std::string stringMessage(pTemp);
	    s << ", message: " << stringMessage;
	    LocalFree((HLOCAL)pTemp);
	}

    	return s.str();
    }

    std::string retrieveAndParseJobs(const LPWSTR iPrinterName, const DWORD& iTotalJobs, Napi::Object result_printer_jobs,  PrinterHandle& iPrinterHandle)
    {
        Napi::Env env = result_printer_jobs.Env();
        DWORD bytes_needed = 0, totalJobs = 0;
        BOOL bError = EnumJobsW(*iPrinterHandle, 0, iTotalJobs, 2, NULL, bytes_needed, &bytes_needed, &totalJobs);
        MemValue<JOB_INFO_2W> jobs(bytes_needed);
        if(!jobs)
        {
            std::string error_str("Error on allocating memory for jobs: ");
            error_str += getLastErrorCodeAndMessage();
            Napi::Object result_printer_job = Napi::Object::New(env);
            result_printer_job.Set(NAPI_STRING_NEW_UTF8(env, "error"), NAPI_STRING_NEW_UTF8(env, error_str.c_str()));
            result_printer_jobs.Set(uint32_t(0), result_printer_job);
            return std::string("");
        }
        DWORD dummy_bytes = 0;
        bError = EnumJobsW(*iPrinterHandle, 0, iTotalJobs, 2, (LPBYTE)jobs.get(), bytes_needed, &dummy_bytes, &totalJobs);
        if(!bError)
        {
            std::string error_str("Error on EnumJobsW: ");
            error_str += getLastErrorCodeAndMessage();
            Napi::Object result_printer_job = Napi::Object::New(env);
            result_printer_job.Set(NAPI_STRING_NEW_UTF8(env, "error"), NAPI_STRING_NEW_UTF8(env, error_str.c_str()));
            result_printer_jobs.Set(uint32_t(0), result_printer_job);
            return std::string("");
        }
        JOB_INFO_2W *job = jobs.get();
        for(DWORD i = 0; i < totalJobs; ++i, ++job)
        {
            Napi::Object result_printer_job =  Napi::Object::New(env);
            parseJobObject(job, result_printer_job);
            result_printer_jobs.Set(i, result_printer_job);
        }
        return std::string("");
    }

    std::string parsePrinterInfo(const PRINTER_INFO_2W *printer, Napi::Object result_printer, PrinterHandle& iPrinterHandle) {
        Napi::Env env = result_printer.Env();
        #define ADD_V8_STRING_PROPERTY(name, key) if((printer->##key != NULL) && (*printer->##key != L'\0'))    \
        {                                   \
            result_printer.Set(Napi::String::New(result_printer.Env(), #name), Napi::String::New(result_printer.Env(), (char16_t*)printer->##key)); \
        }
        //LPTSTR               pPrinterName;
        ADD_V8_STRING_PROPERTY(name, pPrinterName)
        //LPTSTR               pServerName;
        ADD_V8_STRING_PROPERTY(serverName, pServerName)
        //LPTSTR               pShareName;
        ADD_V8_STRING_PROPERTY(shareName, pShareName)
        //LPTSTR               pPortName;
        ADD_V8_STRING_PROPERTY(portName, pPortName)
        //LPTSTR               pDriverName;
        ADD_V8_STRING_PROPERTY(driverName, pDriverName)
        //LPTSTR               pComment;
        ADD_V8_STRING_PROPERTY(comment, pComment)
        //LPTSTR               pLocation;
        ADD_V8_STRING_PROPERTY(location, pLocation)
        //LPTSTR               pSepFile;
        ADD_V8_STRING_PROPERTY(sepFile, pSepFile)
        //LPTSTR               pPrintProcessor;
        ADD_V8_STRING_PROPERTY(printProcessor, pPrintProcessor)
        //LPTSTR               pDatatype;
        ADD_V8_STRING_PROPERTY(datatype, pDatatype)
        //LPTSTR               pParameters;
        ADD_V8_STRING_PROPERTY(parameters, pParameters)
    #undef ADD_V8_STRING_PROPERTY
        //DWORD                Status;
        // statuses from:
        // http://msdn.microsoft.com/en-gb/library/windows/desktop/dd162845(v=vs.85).aspx
       Napi::Array result_printer_status = Napi::Array::New(env);
       int i_status = 0;
       for (const auto& itStatus : getStatusMap()) {
        if (printer->Status & itStatus.second) {
            result_printer_status.Set(i_status++, Napi::String::New(env, itStatus.first.c_str()));
         }
       }
        result_printer.Set(Napi::String::New(env, "status"), result_printer_status);
        result_printer.Set(Napi::String::New(env, "statusNumber"), Napi::Number::New(env, printer->Status));
        // Attributes
        Napi::Array result_printer_attributes = Napi::Array::New(env);
        int i_attribute = 0;
        for (const auto& itAttribute : getAttributeMap()) {
        if (printer->Attributes & itAttribute.second) {
            result_printer_attributes.Set(i_attribute++, Napi::String::New(env, itAttribute.first.c_str()));
        }
         result_printer.Set(Napi::String::New(env, "attributes"), result_printer_attributes);
        // Other properties
        result_printer.Set(Napi::String::New(env, "priority"), Napi::Number::New(env, printer->Priority));
        result_printer.Set(Napi::String::New(env, "defaultPriority"), Napi::Number::New(env, printer->DefaultPriority));
        result_printer.Set(Napi::String::New(env, "averagePPM"), Napi::Number::New(env, printer->AveragePPM));

        if (printer->StartTime > 0) {
            result_printer.Set(Napi::String::New(env, "startTime"), Napi::Number::New(env, printer->StartTime));
        }
        if (printer->UntilTime > 0) {
            result_printer.Set(Napi::String::New(env, "untilTime"), Napi::Number::New(env, printer->UntilTime));
        }
        if (printer->cJobs > 0) {
            Napi::Array result_printer_jobs = Napi::Array::New(env, printer->cJobs);
            std::string error_str = retrieveAndParseJobs(printer->pPrinterName, printer->cJobs, result_printer_jobs, iPrinterHandle);
            if (!error_str.empty()) {
                return error_str;
            }
            result_printer.Set(Napi::String::New(env, "jobs"), result_printer_jobs);
       }


    }

    return "";
    }


}

MY_NODE_MODULE_CALLBACK(getPrinters) 
{
    MY_NODE_MODULE_HANDLESCOPE;
    DWORD printers_size = 0;
    DWORD printers_size_bytes = 0, dummyBytes = 0;
    DWORD Level = 2;
    DWORD flags = PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS;
    // First try to retrieve the number of printers
    BOOL bError = EnumPrintersW(flags, NULL, Level, NULL, 0, &printers_size_bytes, &printers_size);
    // Check for errors in the first call
    if (!bError && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        RETURN_EXCEPTION_STR(MY_NODE_MODULE_ENV, "Error retrieving printer size.");
    }
    
    // Check if we got a valid size
    if (printers_size_bytes == 0) {
        RETURN_EXCEPTION_STR(info.Env(), "No printers found or invalid size returned.");
    }

    // Allocate the required memory
    MemValue<PRINTER_INFO_2W> printers(printers_size_bytes);
    if (!printers.get()) // Make sure to check the pointer
    {
        RETURN_EXCEPTION_STR(MY_NODE_MODULE_ENV, "Error allocating memory for printers");
    }
    
    // Second call to retrieve printer information
    bError = EnumPrintersW(flags, NULL, Level, (LPBYTE)(printers.get()), printers_size_bytes, &dummyBytes, &printers_size);
    if (!bError)
    {
        std::string error_str("Error on EnumPrinters");
        error_str += getLastErrorCodeAndMessage();
        RETURN_EXCEPTION_STR(MY_NODE_MODULE_ENV, error_str.c_str());
    }

    // Prepare result array
    Napi::Array result = Napi::Array::New(MY_NODE_MODULE_ENV, printers_size);
    PRINTER_INFO_2W *printer = printers.get();
    DWORD i = 0;
    for (; i < printers_size; ++i, ++printer)
    {
        Napi::Object a_printer = Napi::Object::New(MY_NODE_MODULE_ENV);
        PrinterHandle printerHandle((LPWSTR)(printer->pPrinterName));
        std::string error_str = parsePrinterInfo(printer, a_printer, printerHandle);
        if (!error_str.empty())
        {
            RETURN_EXCEPTION_STR(MY_NODE_MODULE_ENV, error_str.c_str());
        }
        result.Set(i, a_printer);
    }
    MY_NODE_MODULE_RETURN_VALUE(result);
}

MY_NODE_MODULE_CALLBACK(getDefaultPrinterName) 
{
    MY_NODE_MODULE_HANDLESCOPE;
    DWORD cSize = 0;
    GetDefaultPrinterW(NULL, &cSize);

    if(cSize == 0) {
        MY_NODE_MODULE_RETURN_VALUE(NAPI_STRING_NEW_UTF8(MY_NODE_MODULE_ENV, ""));
    }

    MemValue<uint16_t> bPrinterName(cSize*sizeof(uint16_t));
    BOOL res = GetDefaultPrinterW((LPWSTR)(bPrinterName.get()), &cSize);

    if(!res) {
        MY_NODE_MODULE_RETURN_VALUE(NAPI_STRING_NEW_UTF8(MY_NODE_MODULE_ENV, ""));
    }
    MY_NODE_MODULE_RETURN_VALUE(NAPI_STRING_NEW_2BYTES(MY_NODE_MODULE_ENV, bPrinterName.get()));
}

MY_NODE_MODULE_CALLBACK(printDirect)
{
    MY_NODE_MODULE_HANDLESCOPE;
       // Ensure the correct number of arguments
    if (info.Length() < 5) {
        Napi::TypeError::New(env, "Expected 5 arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Check and extract the first argument (string or buffer)
    std::string data;
    if (!getStringOrBufferFromNapiValue(info[0], data)) {
        Napi::TypeError::New(env, "Argument 0 must be a string or Buffer").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Check and extract the second argument (string)
    if (!info[1].IsString()) {
        Napi::TypeError::New(env, "Argument 1 must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }
    std::u16string u16PrinterName = info[1].As<Napi::String>().Utf16Value();
    std::wstring printername = u16stringToWString(u16PrinterName);

    // Check and extract the third argument (string)
    if (!info[2].IsString()) {
        Napi::TypeError::New(env, "Argument 2 must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }
    std::u16string u16DocName = info[2].As<Napi::String>().Utf16Value();
    std::wstring docname = u16stringToWString(u16DocName);

    // Check and extract the fourth argument (string)
    if (!info[3].IsString()) {
        Napi::TypeError::New(env, "Argument 3 must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }
    std::u16string u16Type = info[3].As<Napi::String>().Utf16Value();
    std::wstring type = u16stringToWString(u16Type);

    // Convert std::wstring to LPWSTR (mutable wide string pointer)
    std::vector<wchar_t> printernameMutable(printername.begin(), printername.end());
    printernameMutable.push_back(L'\0'); // Ensure null-termination

    PrinterHandle printerHandle(printernameMutable.data());
    if (!printerHandle) {
        std::string error_str("Error on PrinterHandle: ");
        error_str += getLastErrorCodeAndMessage();
        Napi::TypeError::New(env, error_str).ThrowAsJavaScriptException();
        return env.Null();
    }

    // Fill in the structure with info about this "document."
    DOC_INFO_1W DocInfo;
    DocInfo.pDocName = const_cast<LPWSTR>(docname.c_str());
    DocInfo.pOutputFile = NULL;
    DocInfo.pDatatype = const_cast<LPWSTR>(type.c_str());

    // Inform the spooler the document is beginning.
    DWORD dwJob = StartDocPrinterW(*printerHandle, 1, (LPBYTE)&DocInfo);
    if (dwJob > 0) {
        // Start a page.
        BOOL bStatus = StartPagePrinter(*printerHandle);
        if (bStatus) {
            // Send the data to the printer.
            DWORD dwBytesWritten = 0L;
            bStatus = WritePrinter(*printerHandle, (LPVOID)(data.c_str()), (DWORD)data.size(), &dwBytesWritten);
            EndPagePrinter(*printerHandle);

            // Check to see if correct number of bytes were written.
            if (dwBytesWritten != data.size()) {
                Napi::TypeError::New(env, "Not all bytes were sent").ThrowAsJavaScriptException();
                return env.Null();
            }
        } else {
            std::string error_str("StartPagePrinter error: ");
            error_str += getLastErrorCodeAndMessage();
            Napi::TypeError::New(env, error_str).ThrowAsJavaScriptException();
            return env.Null();
        }
        // Inform the spooler that the document is ending.
        EndDocPrinter(*printerHandle);
    } else {
        std::string error_str("StartDocPrinterW error: ");
        error_str += getLastErrorCodeAndMessage();
        Napi::TypeError::New(env, error_str).ThrowAsJavaScriptException();
        return env.Null();
    }

    MY_NODE_MODULE_RETURN_VALUE(Napi::Number::New(env, dwJob));
}


MY_NODE_MODULE_CALLBACK(getPrinter) 
{
    MY_NODE_MODULE_HANDLESCOPE;
    MY_NODE_MODULE_RETURN_VALUE(NAPI_STRING_NEW_UTF8(MY_NODE_MODULE_ENV, "getPrinter is not implemented yet, request for a PR if required"));
}

MY_NODE_MODULE_CALLBACK(printFile)
{
    MY_NODE_MODULE_HANDLESCOPE;
    RETURN_EXCEPTION(MY_NODE_MODULE_ENV, "Not yet implemented on Windows");
}

MY_NODE_MODULE_CALLBACK(getSupportedPrintFormats)
{
    MY_NODE_MODULE_HANDLESCOPE;
    RETURN_EXCEPTION(MY_NODE_MODULE_ENV, "Not yet implemented, Request for a PR");
}
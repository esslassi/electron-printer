#include <napi.h>

/* Forward declarations (implemented in print.cpp) */

Napi::Value getPrinters(const Napi::CallbackInfo &info);
Napi::Value getPrinter(const Napi::CallbackInfo &info);
Napi::Value getPrinterDriverOptions(const Napi::CallbackInfo &info);
Napi::Value getSelectedPaperSize(const Napi::CallbackInfo &info);
Napi::Value getDefaultPrinterName(const Napi::CallbackInfo &info);

Napi::Value printDirect(const Napi::CallbackInfo &info);
Napi::Value printFile(const Napi::CallbackInfo &info);

Napi::Value getSupportedPrintFormats(const Napi::CallbackInfo &info);

Napi::Value getJob(const Napi::CallbackInfo &info);
Napi::Value setJob(const Napi::CallbackInfo &info);
Napi::Value getSupportedJobCommands(const Napi::CallbackInfo &info);

/* Module initialization */

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    // Printer listing
    exports.Set("getPrinters", Napi::Function::New(env, getPrinters));
    exports.Set("getPrinter", Napi::Function::New(env, getPrinter));
    exports.Set("getPrinterDriverOptions", Napi::Function::New(env, getPrinterDriverOptions));
    exports.Set("getSelectedPaperSize", Napi::Function::New(env, getSelectedPaperSize));
    exports.Set("getDefaultPrinterName", Napi::Function::New(env, getDefaultPrinterName));

    // Printing
    exports.Set("printDirect", Napi::Function::New(env, printDirect));
    exports.Set("printFile", Napi::Function::New(env, printFile));

    // Capabilities
    exports.Set("getSupportedPrintFormats", Napi::Function::New(env, getSupportedPrintFormats));
    exports.Set("getSupportedJobCommands", Napi::Function::New(env, getSupportedJobCommands));

    // Job management
    exports.Set("getJob", Napi::Function::New(env, getJob));
    exports.Set("setJob", Napi::Function::New(env, setJob));

    return exports;
}

/*
  IMPORTANT:
  Module name MUST match bindings('electron_printer')
  and binding.gyp target_name
*/
NODE_API_MODULE(electron_printer, Init)
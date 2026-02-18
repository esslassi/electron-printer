#include <napi.h>
#include "printer_factory.h"

Napi::Value PrintDirect(const Napi::CallbackInfo &info);
Napi::Value GetPrinters(const Napi::CallbackInfo &info);
Napi::Value GetSystemDefaultPrinter(const Napi::CallbackInfo &info);
Napi::Value GetStatusPrinter(const Napi::CallbackInfo &info);

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "printDirect"),
                Napi::Function::New(env, PrintDirect));
    exports.Set(Napi::String::New(env, "getPrinters"),
                Napi::Function::New(env, GetPrinters));
    exports.Set(Napi::String::New(env, "getDefaultPrinter"),
                Napi::Function::New(env, GetSystemDefaultPrinter));
    exports.Set(Napi::String::New(env, "getStatusPrinter"),
                Napi::Function::New(env, GetStatusPrinter));
    return exports;
}

NODE_API_MODULE(electron_printer, Init)
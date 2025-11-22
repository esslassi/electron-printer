#include "node_printer.hpp"



Napi::Object Init(Napi::Env env, Napi::Object exports) {
  MY_MODULE_SET_METHOD(exports, "SayMyName",  SayMyName);
  MY_MODULE_SET_METHOD(exports, "getPrinters", getPrinters);
  MY_MODULE_SET_METHOD(exports, "getDefaultPrinterName", getDefaultPrinterName);
  MY_MODULE_SET_METHOD(exports, "printDirect", printDirect); 
  MY_MODULE_SET_METHOD(exports, "getPrinter", getPrinter);
  MY_MODULE_SET_METHOD(exports, "printFile", printFile);
  MY_MODULE_SET_METHOD(exports, "getSupportedPrintFormats", getSupportedPrintFormats);
  return exports;
}

NODE_API_MODULE(addon, Init)

// Helpers

bool getStringOrBufferFromNapiValue(const Napi::Value& value, std::string& oData) {
    if (value.IsString()) {
        oData = value.As<Napi::String>().Utf8Value();
        return true;
    }
    if (value.IsBuffer()) {
        Napi::Buffer<char> buffer = value.As<Napi::Buffer<char>>();
        oData.assign(buffer.Data(), buffer.Length());
        return true;
    }
    return false;
}
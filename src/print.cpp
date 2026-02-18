#include <napi.h>
#include "printer_factory.h"

class PrinterWorker : public Napi::AsyncWorker
{
private:
    std::unique_ptr<PrinterInterface> printer;
    std::function<void(PrinterWorker *)> work;
    PrinterInfo printerResult;
    std::vector<PrinterInfo> printersResult;
    bool isMultiplePrinters;
    bool success;

public:
    PrinterWorker(Napi::Function &callback, std::function<void(PrinterWorker *)> executeWork)
        : Napi::AsyncWorker(callback),
          work(executeWork),
          isMultiplePrinters(false),
          success(false)
    {
        printer = PrinterFactory::Create();
    }

    void Execute() override
    {
        if (printer)
        {
            work(this);
        }
        else
        {
            SetError("Failed to create printer");
        }
    }

    void OnOK() override
    {
        Napi::Env env = Env();
        Napi::HandleScope scope(env);

        if (isMultiplePrinters)
        {
            Napi::Array result = Napi::Array::New(env, printersResult.size());
            for (size_t i = 0; i < printersResult.size(); i++)
            {
                result.Set(i, CreatePrinterObject(env, printersResult[i]));
            }
            Callback().Call({env.Null(), result});
        }
        else
        {
            Callback().Call({env.Null(), CreatePrinterObject(env, printerResult)});
        }
    }

    PrinterInterface *GetPrinter() { return printer.get(); }
    void SetPrinterResult(const PrinterInfo &result) { printerResult = result; }
    void SetPrintersResult(const std::vector<PrinterInfo> &result)
    {
        printersResult = result;
        isMultiplePrinters = true;
    }
    void SetSuccess(bool value) { success = value; }
    bool GetSuccess() const { return success; }

private:
    Napi::Object CreatePrinterObject(Napi::Env env, const PrinterInfo &printer)
    {
        Napi::Object result = Napi::Object::New(env);
        result.Set("name", printer.name);
        result.Set("status", printer.status);

        // Só incluir estes campos se NÃO for um resultado do PrintDirect
        if (!success)
        { // Se success for true, é um PrintDirect
            result.Set("isDefault", printer.isDefault);

            Napi::Object details = Napi::Object::New(env);
            for (const auto &detail : printer.details)
            {
                details.Set(detail.first, detail.second);
            }
            result.Set("details", details);
        }

        return result;
    }
};

Napi::Value PrintDirect(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "Expected an object as argument").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object options = info[0].As<Napi::Object>();

    if (!options.Has("printerName") || !options.Has("data"))
    {
        Napi::TypeError::New(env, "Object must have 'printerName' and 'data' properties").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!options.Get("printerName").IsString())
    {
        Napi::TypeError::New(env, "printerName must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Value data = options.Get("data");
    if (!data.IsString() && !data.IsBuffer())
    {
        Napi::TypeError::New(env, "data must be a string or Buffer").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string printerName = options.Get("printerName").As<Napi::String>().Utf8Value();
    std::vector<uint8_t> printData;

    if (data.IsString())
    {
        std::string strData = data.As<Napi::String>().Utf8Value();
        printData.assign(strData.begin(), strData.end());
    }
    else
    {
        Napi::Buffer<uint8_t> buffer = data.As<Napi::Buffer<uint8_t>>();
        printData.assign(buffer.Data(), buffer.Data() + buffer.Length());
    }

    std::string dataType = "RAW";
    if (options.Has("dataType") && options.Get("dataType").IsString())
    {
        dataType = options.Get("dataType").As<Napi::String>().Utf8Value();
    }

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    auto callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo &info)
                                        {
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return info.Env().Undefined(); });

    auto worker = new PrinterWorker(
        callback,
        [printerName, printData, dataType](PrinterWorker *worker)
        {
            bool success = worker->GetPrinter()->PrintDirect(printerName, printData, dataType);
            worker->SetSuccess(true); // Indica que é um resultado do PrintDirect
            PrinterInfo result;
            result.name = printerName;
            result.status = success ? "success" : "failed";
            worker->SetPrinterResult(result);
        });

    worker->Queue();
    return deferred.Promise();
}

Napi::Value GetPrinters(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    auto callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo &info)
                                        {
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return info.Env().Undefined(); });

    auto worker = new PrinterWorker(
        callback,
        [](PrinterWorker *worker)
        {
            auto printers = worker->GetPrinter()->GetPrinters();
            worker->SetPrintersResult(printers);
        });

    worker->Queue();
    return deferred.Promise();
}

Napi::Value GetSystemDefaultPrinter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    auto callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo &info)
                                        {
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return info.Env().Undefined(); });

    auto worker = new PrinterWorker(
        callback,
        [](PrinterWorker *worker)
        {
            auto printer = worker->GetPrinter()->GetSystemDefaultPrinter();
            worker->SetPrinterResult(printer);
        });

    worker->Queue();
    return deferred.Promise();
}

Napi::Value GetStatusPrinter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "Expected an object as argument").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object options = info[0].As<Napi::Object>();

    Napi::Array propertyNames = options.GetPropertyNames();
    for (uint32_t i = 0; i < propertyNames.Length(); i++)
    {
        propertyNames.Get(i).As<Napi::String>();
    }

    if (!options.Has("printerName"))
    {
        Napi::TypeError::New(env, "Object must have 'printerName' property").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!options.Get("printerName").IsString())
    {
        Napi::TypeError::New(env, "printerName must be a string").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string printerName = options.Get("printerName").As<Napi::String>().Utf8Value();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    auto callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo &info)
                                        {
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return info.Env().Undefined(); });

    auto worker = new PrinterWorker(
        callback,
        [printerName](PrinterWorker *worker)
        {
            auto printer = worker->GetPrinter()->GetStatusPrinter(printerName);
            worker->SetPrinterResult(printer);
        });

    worker->Queue();
    return deferred.Promise();
}

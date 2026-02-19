#include <napi.h>
#include <fstream>
#include <vector>
#include <memory>
#include <functional>

#include "printer_factory.h"
#include "printer_interface.h"

static std::unique_ptr<PrinterInterface> P()
{
    return PrinterFactory::Create();
}

/* =========================================================
   JS Converters
========================================================= */

static Napi::Object JsPrinterDetails(Napi::Env env, const PrinterDetailsNative &p)
{
    Napi::Object o = Napi::Object::New(env);
    o.Set("name", p.name);
    o.Set("isDefault", p.isDefault);

    Napi::Object opts = Napi::Object::New(env);
    for (auto &kv : p.options)
        opts.Set(kv.first, kv.second);

    o.Set("options", opts);
    return o;
}

static Napi::Object JsDriverOptions(Napi::Env env, const DriverOptions &opts)
{
    Napi::Object out = Napi::Object::New(env);
    for (auto &group : opts)
    {
        Napi::Object choices = Napi::Object::New(env);
        for (auto &choice : group.second)
            choices.Set(choice.first, Napi::Boolean::New(env, choice.second));
        out.Set(group.first, choices);
    }
    return out;
}

static Napi::Object JsJobDetails(Napi::Env env, const JobDetailsNative &j)
{
    Napi::Object o = Napi::Object::New(env);
    o.Set("id", j.id);
    o.Set("name", j.name);
    o.Set("printerName", j.printerName);
    o.Set("user", j.user);
    o.Set("format", j.format);
    o.Set("priority", j.priority);
    o.Set("size", j.size);

    Napi::Array st = Napi::Array::New(env, j.status.size());
    for (size_t i = 0; i < j.status.size(); i++)
        st.Set((uint32_t)i, j.status[i]);
    o.Set("status", st);

    o.Set("completedTime", Napi::Date::New(env, (double)j.completedTime * 1000.0));
    o.Set("creationTime", Napi::Date::New(env, (double)j.creationTime * 1000.0));
    o.Set("processingTime", Napi::Date::New(env, (double)j.processingTime * 1000.0));

    return o;
}

/* =========================================================
   Sync Methods
========================================================= */

Napi::Value getPrinters(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    auto printer = P();
    auto list = printer->GetPrinters();

    Napi::Array arr = Napi::Array::New(env, list.size());
    for (size_t i = 0; i < list.size(); i++)
        arr.Set((uint32_t)i, JsPrinterDetails(env, list[i]));

    return arr;
}

Napi::Value getPrinter(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    if (info.Length() < 1 || !info[0].IsString())
        Napi::TypeError::New(env, "printerName required").ThrowAsJavaScriptException();

    auto printer = P();
    auto p = printer->GetPrinter(info[0].As<Napi::String>().Utf8Value());
    return JsPrinterDetails(env, p);
}

Napi::Value getPrinterDriverOptions(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    if (info.Length() < 1 || !info[0].IsString())
        Napi::TypeError::New(env, "printerName required").ThrowAsJavaScriptException();

    auto printer = P();
    auto opts = printer->GetPrinterDriverOptions(info[0].As<Napi::String>().Utf8Value());
    return JsDriverOptions(env, opts);
}

Napi::Value getSelectedPaperSize(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    if (info.Length() < 1 || !info[0].IsString())
        Napi::TypeError::New(env, "printerName required").ThrowAsJavaScriptException();

    auto printer = P();
    auto ps = printer->GetSelectedPaperSize(info[0].As<Napi::String>().Utf8Value());
    return Napi::String::New(env, ps);
}

Napi::Value getDefaultPrinterName(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    auto printer = P();
    auto name = printer->GetDefaultPrinterName();
    if (name.empty())
        return env.Undefined();
    return Napi::String::New(env, name);
}

Napi::Value getSupportedPrintFormats(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    auto printer = P();
    auto formats = printer->GetSupportedPrintFormats();

    Napi::Array arr = Napi::Array::New(env, formats.size());
    for (size_t i = 0; i < formats.size(); i++)
        arr.Set((uint32_t)i, formats[i]);

    return arr;
}

Napi::Value getSupportedJobCommands(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    auto printer = P();
    auto cmds = printer->GetSupportedJobCommands();

    Napi::Array arr = Napi::Array::New(env, cmds.size());
    for (size_t i = 0; i < cmds.size(); i++)
        arr.Set((uint32_t)i, cmds[i]);

    return arr;
}

Napi::Value getJob(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsNumber())
        Napi::TypeError::New(env, "getJob(printerName, jobId)").ThrowAsJavaScriptException();

    auto printer = P();
    auto job = printer->GetJob(
        info[0].As<Napi::String>().Utf8Value(),
        info[1].As<Napi::Number>().Int32Value());

    return JsJobDetails(env, job);
}

Napi::Value setJob(const Napi::CallbackInfo &info)
{
    auto env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString())
        Napi::TypeError::New(env, "setJob(printerName, jobId, command)").ThrowAsJavaScriptException();

    auto printer = P();
    printer->SetJob(
        info[0].As<Napi::String>().Utf8Value(),
        info[1].As<Napi::Number>().Int32Value(),
        info[2].As<Napi::String>().Utf8Value());

    return env.Undefined();
}

/* =========================================================
   Async Print Worker
========================================================= */

class PrintWorker : public Napi::AsyncWorker
{
public:
    PrintWorker(
        Napi::Function successCb,
        Napi::Function errorCb,
        std::function<int()> workFn)
        : Napi::AsyncWorker(successCb),
          successRef(Napi::Persistent(successCb)),
          errorRef(Napi::Persistent(errorCb)),
          work(workFn)
    {}

    void Execute() override
    {
        try
        {
            jobId = work();
            if (jobId <= 0)
                SetError("Print failed");
        }
        catch (...)
        {
            SetError("Print failed (exception)");
        }
    }

    void OnOK() override
    {
        Napi::HandleScope scope(Env());
        successRef.Call({ Napi::String::New(Env(), std::to_string(jobId)) });
    }

    void OnError(const Napi::Error &e) override
    {
        Napi::HandleScope scope(Env());
        errorRef.Call({ e.Value() });
    }

private:
    Napi::FunctionReference successRef;
    Napi::FunctionReference errorRef;
    std::function<int()> work;
    int jobId = 0;
};

static Napi::Function SafeCb(Napi::Env env, Napi::Object opt, const char *key)
{
    if (opt.Has(key) && opt.Get(key).IsFunction())
        return opt.Get(key).As<Napi::Function>();

    return Napi::Function::New(env, [](const Napi::CallbackInfo &) {});
}

/* =========================================================
   printDirect
========================================================= */

Napi::Value printDirect(const Napi::CallbackInfo &info)
{
    auto env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
        Napi::TypeError::New(env, "options object required").ThrowAsJavaScriptException();

    Napi::Object opt = info[0].As<Napi::Object>();

    if (!opt.Has("data"))
        Napi::TypeError::New(env, "options.data required").ThrowAsJavaScriptException();

    std::string printerName;
    if (opt.Has("printer") && opt.Get("printer").IsString())
        printerName = opt.Get("printer").As<Napi::String>().Utf8Value();

    std::string type = "RAW";
    if (opt.Has("type") && opt.Get("type").IsString())
        type = opt.Get("type").As<Napi::String>().Utf8Value();

    StringMap driverOpts;
    if (opt.Has("options") && opt.Get("options").IsObject())
    {
        Napi::Object o = opt.Get("options").As<Napi::Object>();
        auto props = o.GetPropertyNames();
        for (uint32_t i = 0; i < props.Length(); i++)
        {
            auto k = props.Get(i).As<Napi::String>().Utf8Value();
            auto v = o.Get(k).ToString().Utf8Value();
            driverOpts[k] = v;
        }
    }

    std::vector<uint8_t> data;
    auto d = opt.Get("data");

    if (d.IsBuffer())
    {
        auto b = d.As<Napi::Buffer<uint8_t>>();
        data.assign(b.Data(), b.Data() + b.Length());
    }
    else
    {
        auto s = d.ToString().Utf8Value();
        data.assign(s.begin(), s.end());
    }

    auto successCb = SafeCb(env, opt, "success");
    auto errorCb = SafeCb(env, opt, "error");

    auto worker = new PrintWorker(
        successCb,
        errorCb,
        [printerName, data, type, driverOpts]() -> int
        {
            auto printer = P();
            std::string usePrinter = printerName.empty()
                ? printer->GetDefaultPrinterName()
                : printerName;

            return printer->PrintDirect(usePrinter, data, type, driverOpts);
        });

    worker->Queue();
    return env.Undefined();
}

/* =========================================================
   printFile
========================================================= */

Napi::Value printFile(const Napi::CallbackInfo &info)
{
    auto env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
        Napi::TypeError::New(env, "options object required").ThrowAsJavaScriptException();

    Napi::Object opt = info[0].As<Napi::Object>();

    if (!opt.Has("filename") || !opt.Get("filename").IsString())
        Napi::TypeError::New(env, "options.filename required").ThrowAsJavaScriptException();

    std::string filename = opt.Get("filename").As<Napi::String>().Utf8Value();

    std::string printerName;
    if (opt.Has("printer") && opt.Get("printer").IsString())
        printerName = opt.Get("printer").As<Napi::String>().Utf8Value();

    auto successCb = SafeCb(env, opt, "success");
    auto errorCb = SafeCb(env, opt, "error");

    auto worker = new PrintWorker(
        successCb,
        errorCb,
        [printerName, filename]() -> int
        {
            auto printer = P();
            std::string usePrinter = printerName.empty()
                ? printer->GetDefaultPrinterName()
                : printerName;

            return printer->PrintFile(usePrinter, filename);
        });

    worker->Queue();
    return env.Undefined();
}
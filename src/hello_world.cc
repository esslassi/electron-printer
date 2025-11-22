#include "node_printer.hpp"

MY_NODE_MODULE_CALLBACK(SayMyName) 
{
  MY_NODE_MODULE_HANDLESCOPE
   Napi::String result = Napi::String::New(info.Env(), "Hello, From C++ !");
  MY_NODE_MODULE_RETURN_VALUE(result);
}
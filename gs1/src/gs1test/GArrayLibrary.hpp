#include <gs1/vm/Context.hpp>

namespace gs1
{
class GArrayLibrary : public GLibrary
{
public:
  GArrayLibrary()
  {
    RegisterFunction("arraylen", [&](Context *context) {
      GValue value = context->stack.Pop();
      GVariable* array = value.GetVariable();
      if (array && array->GetVarType() == GVARTYPE_ARRAY) {
        int len = ((GArrayVariable*)array)->values.size();

        context->stack.Push((float)len);
        Log::Get().Print(LOGLEVEL_VERBOSE, "array length = %d\n", len);
      }
    });
  }

  ~GArrayLibrary(){};

  static std::string GetName() { return "GArrayLibrary"; }
};
};

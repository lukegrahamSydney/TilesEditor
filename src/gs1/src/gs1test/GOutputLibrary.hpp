#include <gs1/vm/Context.hpp>

namespace gs1
{
class GOutputLibrary : public GLibrary
{
public:
  GOutputLibrary()
  {

    RegisterCommand("message", [&](Context *context) {
      std::string strValue =
          context->InterpolateString(context->stack.Pop().GetString());

      Log::Get().Print(LOGLEVEL_INFO, "%s\n", strValue.c_str());
    });

    RegisterCommand("print", [&](Context *context) {
      std::string strValue =
          context->InterpolateString(context->stack.Pop().GetString());

      Log::Get().Print(LOGLEVEL_INFO, "%s\n", strValue.c_str());
    });
  }

  ~GOutputLibrary(){};

  static std::string GetName() { return "GOutputLibrary"; }
};
};
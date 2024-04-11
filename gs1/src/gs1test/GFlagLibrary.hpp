#include <gs1/vm/Context.hpp>

namespace gs1
{
class GFlagLibrary : public GLibrary
{
public:
  GFlagLibrary()
  {

    RegisterCommand("set", [&](Context *context) {
      std::string flagName = context->stack.Pop().GetString();

      context->SetVariable(flagName, GVARTYPE_FLAG, true);
    });

    RegisterCommand("unset", [&](Context *context) {
      std::string flagName = context->stack.Pop().GetString();

      context->SetVariable(flagName, GVARTYPE_FLAG, false);
    });

    RegisterCommand("setplayerprop", [&](Context *context) {
      std::string propValue = context->stack.Pop().GetString();
      std::string propName = context->stack.Pop().GetString();

      Log::Get().Print(LOGLEVEL_VERBOSE, "Setting player prop %s = %s\n",
                       propName.c_str(), propValue.c_str());
    });
  };

  ~GFlagLibrary(){};

  static std::string GetName() { return "GFlagLibrary"; }
};
};
#include <gs1/vm/Context.hpp>
#include <gs1/vm/GStringFormatter.hpp>

#include <gs1/common/Log.hpp>
#include <regex>
#include <cstdio>

using namespace gs1;

int GStringFormatter::NeedsFormatting(const char *str)
{
  // Number variable
  if (str[0] == 'v')
    return 1;

  // String variable
  if (str[0] == 's')
    return 1;

  // Substring
  if (str[0] == 'e')
    return 1;

  return 0;
};

std::string GStringFormatter::Format(Context *context, const std::string &type,
                                     const std::string &param)
{
  // Variable values
  if (type == "v") {
    // TODO:
    // These need to be properly eval'd
    // Right now it just fetches a single number
    GVariable *variable = context->GetVariable(param, GVARTYPE_NUMBER);

    if (variable && variable->GetVarType() == GVARTYPE_NUMBER) {
      Log::Get().Print(LOGLEVEL_VERBOSE, "#v %s=%f\n", param.c_str(),
                       ((GNumberVariable *)variable)->number);

      // TODO: Fix this
      std::string output;
      output.resize(16);
      snprintf(&output[0], 16, "%.6g", ((GNumberVariable *)variable)->number);

      return output;
    } else
      Log::Get().Print(LOGLEVEL_VERBOSE, "#v %s not found!\n", param.c_str());
  }
  // String values
  else if (type == "s") {
    GVariable *variable = context->GetVariable(param, GVARTYPE_STRING);

    if (variable && variable->GetVarType() == GVARTYPE_STRING) {
      Log::Get().Print(LOGLEVEL_VERBOSE, "#s %s=%s\n", param.c_str(),
                       ((GStringVariable *)variable)->string.c_str());

      return ((GStringVariable *)variable)->string;
    } else
      Log::Get().Print(LOGLEVEL_VERBOSE, "#s %s not found!\n", param.c_str());
  }
  // Substring values
  else if (type == "e") {
    // Janky regex
    // TODO:
    // At some point this should maybe become part of the lexer/parser
    std::regex regex("\\s*([0-9]+)\\s*,\\s*([0-9]+)\\s*,\\s*(.*)\\s*");
    std::smatch results;
    std::regex_search(param, results, regex);

    if (results.size() != 4)
      return "";

    int startIndex = std::stoi(results[1].str());
    int length = std::stoi(results[2].str());
    std::string str = context->InterpolateString(results[3].str());

    Log::Get().Print(LOGLEVEL_VERBOSE, "#e %s[%d:%d] = %s\n", param.c_str(),
                     startIndex, length, str.c_str());

    return str.substr(startIndex, length);
  }

  return "";
};

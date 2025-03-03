#ifndef GS1VM_GLIBRARY_HPP
#define GS1VM_GLIBRARY_HPP

#include <gs1/vm/GVarStore.hpp>

#include <functional>
#include <string>

namespace gs1
{
class Context;

class GLibrary
{
public:
  GLibrary();
  virtual ~GLibrary();

  std::function<void(Context *context)> *GetCommand(const std::string &name);
  std::function<void(Context *context)> *GetFunction(const std::string &name);

protected:
  void RegisterCommand(std::string name,
                       std::function<void(Context *context)> commandHandler);
  void RegisterFunction(std::string name,
                        std::function<void(Context *context)> functionHandler);

private:
  std::unordered_map<std::string, std::function<void(Context *context)>>
      commandHandlers;
  std::unordered_map<std::string, std::function<void(Context *context)>>
      funcHandlers;
};
}

#endif
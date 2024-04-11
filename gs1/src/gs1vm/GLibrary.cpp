#include <gs1/vm/GLibrary.hpp>

using namespace gs1;

GLibrary::GLibrary() {}

GLibrary::~GLibrary() {}

std::function<void(Context *context)> *
GLibrary::GetCommand(const std::string &name)
{
  auto itr = commandHandlers.find(name);

  if (itr != commandHandlers.end())
    return &itr->second;

  return nullptr;
}

std::function<void(Context *context)> *
GLibrary::GetFunction(const std::string &name)
{
  auto itr = funcHandlers.find(name);

  if (itr != funcHandlers.end())
    return &itr->second;

  return nullptr;
}

void GLibrary::RegisterCommand(
    std::string name, std::function<void(Context *context)> commandHandler)
{
  commandHandlers[name] = commandHandler;
}

void GLibrary::RegisterFunction(
    std::string name, std::function<void(Context *context)> functionHandler)
{
  funcHandlers[name] = functionHandler;
}
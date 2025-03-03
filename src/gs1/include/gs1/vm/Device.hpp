#ifndef GS1VM_DEVICE_HPP
#define GS1VM_DEVICE_HPP

#include <gs1/common/ByteBuffer.hpp>
#include <gs1/common/Log.hpp>
#include <gs1/parse/Parser.hpp>
#include <gs1/vm/Context.hpp>

namespace gs1
{
class Device
{
public:
  Device();
  virtual ~Device();

  std::shared_ptr<Context>
  CreateContext(std::shared_ptr<GVarStore> primaryVarStore);

  template <typename T> std::shared_ptr<GLibrary> LoadLibrary()
  {
    std::shared_ptr<GLibrary> lib = std::make_shared<T>();

    std::string name = T::GetName();

    // Library is already loaded
    if (libraries.find(name) == libraries.end())
      libraries[name] = lib;

    return libraries[name];
  };

  std::shared_ptr<Bytecode> LoadBytecode(const char *data, unsigned int len);

  std::shared_ptr<GVarStore> CreateVarStore();

  ByteBuffer CompileSourceFromString(std::string str, PrototypeMap cmds,
                                     PrototypeMap funcs);
  ByteBuffer CompileSourceFromFile(std::string path, PrototypeMap cmds,
                                   PrototypeMap funcs);

private:
  std::unordered_map<std::string, std::shared_ptr<GLibrary>> libraries;
};
};

#endif
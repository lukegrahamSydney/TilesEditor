#include <gs1/compiler/CompileVisitor.hpp>
#include <gs1/vm/Device.hpp>

using namespace gs1;

Device::Device() {}

Device::~Device() {}

void observer(const Diag &d)
{
  switch (d.severity) {
  case Diag::Info:
    Log::Get().Print(LOGLEVEL_INFO, "info: %d@%d: %s\n", d.pos.line + 1,
                     d.pos.offset, d.message.c_str());
    break;
  case Diag::Warning:
    Log::Get().Print(LOGLEVEL_WARNING, "warning: %d@%d: %s\n", d.pos.line + 1,
                     d.pos.offset, d.message.c_str());
    break;
  case Diag::Error:
    Log::Get().Print(LOGLEVEL_ERROR, "error: %d@%d: %s\n", d.pos.line + 1,
                     d.pos.offset, d.message.c_str());
    break;
  }
}

ByteBuffer Device::CompileSourceFromString(std::string str, PrototypeMap cmds,
                                           PrototypeMap funcs)
{
  MemorySource source(str.c_str());
  DiagBuilder diag(observer);
  CompileVisitor visitor(source);
  Lexer lexer(diag, source);
  Parser parser(diag, lexer, cmds, funcs);

  auto tree = parser.Parse();
  tree->Accept(&visitor);

  return visitor.GetBytecode();
}

ByteBuffer Device::CompileSourceFromFile(std::string path, PrototypeMap cmds,
                                         PrototypeMap funcs)
{
  FileSource source(path);
  DiagBuilder diag(observer);
  CompileVisitor visitor(source);
  Lexer lexer(diag, source);
  Parser parser(diag, lexer, cmds, funcs);

  auto tree = parser.Parse();
  tree->Accept(&visitor);

  return visitor.GetBytecode();
}

std::shared_ptr<Context>
Device::CreateContext(std::shared_ptr<GVarStore> primaryVarStore)
{
  return std::make_shared<Context>(this, primaryVarStore);
}

std::shared_ptr<GVarStore> Device::CreateVarStore()
{
  return std::make_shared<GVarStore>();
}

std::shared_ptr<Bytecode> Device::LoadBytecode(const char *data,
                                               unsigned int length)
{
  return std::make_shared<Bytecode>(data, length);
}
#ifndef GS1COMMON_CONSTANTTABLE_HPP
#define GS1COMMON_CONSTANTTABLE_HPP

#include <gs1/common/Util.hpp>

#include <algorithm>
#include <string>

namespace gs1
{
struct ConstantKey {
  ConstantKey(int index) : index(index){};

  int index;
};

template <typename T> class ConstantTable
{
public:
  struct Constant {
    Constant(T val) : val(val) {}

    T val;
  };

  ConstantTable(){};

  virtual ~ConstantTable(){};

  ConstantKey GetKey(const T &value)
  {
    auto itr =
        std::find_if(constants.begin(), constants.end(),
                     [&](const typename std::vector<Constant>::value_type &vt) {
                       return vt.val == value;
                     });

    if (itr == constants.end()) {
      // No name was found, add our own
      constants.push_back(Constant(value));

      return ConstantKey(constants.size() - 1);
    } else {
      // Get key for name
      return ConstantKey(itr - constants.begin());
    }
  };

  const Constant &GetConstant(const ConstantKey &key)
  {
    if (key.index < 0 || key.index >= constants.size())
      throw Exception("Name not found");
    else
      return constants[key.index];
  };

  std::vector<Constant> constants;
};
}

#endif

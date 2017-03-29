#include "ignition/common/plugin/RegisterMacros.hh"
#include "util/DummyPlugins.hh"


namespace test
{
namespace util
{

std::string DummyPlugin::MyNameIs()
{
  return std::string("DummyPlugin");
}

}
}

IGN_COMMON_REGISTER_SINGLE_PLUGIN(test::util::DummyPlugin,
    test::util::DummyPluginBase);

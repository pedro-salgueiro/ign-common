/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

// Defining this macro before including ignition/common/SpecializedPluginPtr.hh
// allows us to test that the low-cost routines are being used to access the
// specialized plugin interfaces.
#define IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS

#include <gtest/gtest.h>
#include <dlfcn.h>
#include <iostream>
#include "ignition/common/PluginLoader.hh"
#include "ignition/common/SystemPaths.hh"
#include "ignition/common/PluginPtr.hh"
#include "ignition/common/SpecializedPluginPtr.hh"
#include "ignition/common/Console.hh"

#include "test_config.h"
#include "util/DummyPlugins.hh"


/////////////////////////////////////////////////
std::string GetPluginLibraryPath()
{
  std::string projectPath(PROJECT_BINARY_PATH);

  ignition::common::SystemPaths sp;
  sp.AddPluginPaths(projectPath + "/test/util");
  std::string path = sp.FindSharedLibrary("IGNDummyPlugins");

  return path;
}

/////////////////////////////////////////////////
TEST(PluginLoader, LoadExistingLibrary)
{
  const std::string path = GetPluginLibraryPath();
  ASSERT_FALSE(path.empty());

  ignition::common::PluginLoader pl;

  // Make sure the expected plugins were loaded.
  std::unordered_set<std::string> pluginNames = pl.LoadLibrary(path);
  EXPECT_EQ(1u, pluginNames.count("::test::util::DummySinglePlugin"));
  EXPECT_EQ(1u, pluginNames.count("::test::util::DummyMultiPlugin"));

  std::cout << pl.PrettyStr();

  // Make sure the expected interfaces were loaded.
  EXPECT_EQ(4u, pl.InterfacesImplemented().size());
  EXPECT_EQ(1u, pl.InterfacesImplemented()
            .count("::test::util::DummyNameBase"));
  EXPECT_EQ(2u, pl.PluginsImplementing("::test::util::DummyNameBase").size());
  EXPECT_EQ(1u, pl.PluginsImplementing("::test::util::DummyDoubleBase").size());


  ignition::common::PluginPtr firstPlugin =
      pl.Instantiate("test::util::DummySinglePlugin");
  EXPECT_FALSE(firstPlugin.IsEmpty());

  ignition::common::PluginPtr secondPlugin =
      pl.Instantiate("test::util::DummyMultiPlugin");
  EXPECT_FALSE(secondPlugin.IsEmpty());

  // Check that the DummyNameBase interface exists and that it returns the
  // correct value.
  test::util::DummyNameBase* nameBase =
      firstPlugin->GetInterface<test::util::DummyNameBase>(
        "test::util::DummyNameBase");
  ASSERT_NE(nullptr, nameBase);
  EXPECT_EQ(std::string("DummySinglePlugin"), nameBase->MyNameIs());

  // Check that DummyDoubleBase does not exist for this plugin
  test::util::DummyDoubleBase* doubleBase =
      firstPlugin->GetInterface<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  EXPECT_EQ(nullptr, doubleBase);

  // Check that DummyDoubleBase does exist for this function and that it returns
  // the correct value.
  doubleBase = secondPlugin->GetInterface<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  ASSERT_NE(nullptr, doubleBase);
  EXPECT_NEAR(3.14159, doubleBase->MyDoubleValueIs(), 1e-8);

  // Check that the DummyNameBase interface exists for this plugin and that it
  // returns the correct value.
  nameBase = secondPlugin->GetInterface<test::util::DummyNameBase>(
        "test::util::DummyNameBase");
  ASSERT_NE(nullptr, nameBase);
  EXPECT_EQ(std::string("DummyMultiPlugin"), nameBase->MyNameIs());
}


/////////////////////////////////////////////////
class SomeInterface
{
  public: IGN_COMMON_SPECIALIZE_INTERFACE(SomeInterface)
};

using SomeSpecializedPluginPtr =
    ignition::common::SpecializedPluginPtr<
        SomeInterface,
        test::util::DummyIntBase,
        test::util::DummySetterBase>;

/////////////////////////////////////////////////
TEST(SpecializedPluginPtr, Construction)
{
  const std::string path = GetPluginLibraryPath();
  ASSERT_FALSE(path.empty());

  ignition::common::PluginLoader pl;
  pl.LoadLibrary(path);

  SomeSpecializedPluginPtr plugin(
        pl.Instantiate("::test::util::DummyMultiPlugin"));
  EXPECT_FALSE(plugin.IsEmpty());

  // Make sure the specialized interface is available, that it is accessed using
  // the specialized access, and that it returns the expected value.
  usedSpecializedInterfaceAccess = false;
  test::util::DummyIntBase *fooBase =
      plugin->GetInterface<test::util::DummyIntBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_NE(nullptr, fooBase);
  EXPECT_EQ(5, fooBase->MyIntegerValueIs());

  // Make sure the specialized interface is available and that it is accessed
  // using the specialized access.
  usedSpecializedInterfaceAccess = false;
  test::util::DummySetterBase *setterBase =
      plugin->GetInterface<test::util::DummySetterBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_NE(nullptr, setterBase);

  // Make sure that the interface makes the expected changes to the plugin.
  const int newIntValue = 54321;
  setterBase->SetIntegerValue(newIntValue);
  EXPECT_EQ(newIntValue, fooBase->MyIntegerValueIs());

  // Make sure the specialized interface is available, that it is accessed using
  // the specialized access, and that it returns the expected value.
  usedSpecializedInterfaceAccess = false;
  test::util::DummyDoubleBase *doubleBase =
      plugin->GetInterface<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  EXPECT_FALSE(usedSpecializedInterfaceAccess);
  EXPECT_NE(nullptr, doubleBase);
  EXPECT_NEAR(3.14159, doubleBase->MyDoubleValueIs(), 1e-8);

  // Make sure that the interfaces work together as expected.
  const double newDubValue = 2.718281828459045;
  setterBase->SetDoubleValue(newDubValue);
  EXPECT_NEAR(newDubValue, doubleBase->MyDoubleValueIs(), 1e-8);

  // Make sure that the unexpected interface is not available, even though the
  // plugin was specialized for it. Also make sure that the specialized access
  // is being used.
  usedSpecializedInterfaceAccess = false;
  SomeInterface *someInterface = plugin->GetInterface<SomeInterface>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_EQ(nullptr, someInterface);
}

/////////////////////////////////////////////////
template <typename PluginPtrType1, typename PluginPtrType2>
void TestSetAndMapUsage(
    const ignition::common::PluginLoader &loader,
    const ignition::common::PluginPtr &plugin)
{
  PluginPtrType1 plugin1 = plugin;
  PluginPtrType2 plugin2 = plugin1;

  EXPECT_TRUE(plugin1 == plugin);
  EXPECT_TRUE(plugin1 == plugin2);
  EXPECT_FALSE(plugin1 != plugin2);

  EXPECT_TRUE(plugin2 == plugin);
  EXPECT_TRUE(plugin2 == plugin1);
  EXPECT_FALSE(plugin2 != plugin1);

  std::set<ignition::common::PluginPtr> orderedSet;
  EXPECT_TRUE(orderedSet.insert(plugin1).second);
  EXPECT_FALSE(orderedSet.insert(plugin1).second);
  EXPECT_FALSE(orderedSet.insert(plugin2).second);

  std::unordered_set<ignition::common::PluginPtr> unorderedSet;
  EXPECT_TRUE(unorderedSet.insert(plugin1).second);
  EXPECT_FALSE(unorderedSet.insert(plugin1).second);
  EXPECT_FALSE(unorderedSet.insert(plugin2).second);

  std::map<ignition::common::PluginPtr, std::string> orderedMap;
  EXPECT_TRUE(orderedMap.insert(std::make_pair(plugin1, "some string")).second);
  EXPECT_FALSE(orderedMap.insert(std::make_pair(plugin1, "a string")).second);
  EXPECT_FALSE(orderedMap.insert(std::make_pair(plugin2, "chars")).second);

  std::unordered_map<ignition::common::PluginPtr, std::string> unorderedMap;
  EXPECT_TRUE(unorderedMap.insert(std::make_pair(plugin1, "strings")).second);
  EXPECT_FALSE(unorderedMap.insert(std::make_pair(plugin1, "letters")).second);
  EXPECT_FALSE(unorderedMap.insert(std::make_pair(plugin2, "")).second);


  plugin2 = loader.Instantiate("test::util::DummyMultiPlugin");
  EXPECT_TRUE(plugin1 != plugin2);
  EXPECT_FALSE(plugin1 == plugin2);
  EXPECT_TRUE(plugin2 != plugin1);
  EXPECT_FALSE(plugin2 == plugin1);

  EXPECT_TRUE(orderedSet.insert(plugin2).second);
  EXPECT_FALSE(orderedSet.insert(plugin2).second);

  EXPECT_TRUE(unorderedSet.insert(plugin2).second);
  EXPECT_FALSE(unorderedSet.insert(plugin2).second);

  EXPECT_TRUE(orderedMap.insert(std::make_pair(plugin2, "letters")).second);
  EXPECT_FALSE(orderedMap.insert(std::make_pair(plugin2, "chars")).second);

  EXPECT_TRUE(unorderedMap.insert(std::make_pair(plugin2, "abc")).second);
  EXPECT_FALSE(unorderedMap.insert(std::make_pair(plugin2, "def")).second);
}

/////////////////////////////////////////////////
using SingleSpecializedPluginPtr =
    ignition::common::SpecializedPluginPtr<SomeInterface>;

using AnotherSpecializedPluginPtr =
    ignition::common::SpecializedPluginPtr<
        SomeInterface,
        test::util::DummyIntBase>;

/////////////////////////////////////////////////
TEST(PluginPtr, CopyMoveSemantics)
{
  ignition::common::PluginPtr plugin;
  EXPECT_TRUE(plugin.IsEmpty());

  const std::string path = GetPluginLibraryPath();
  ASSERT_FALSE(path.empty());

  ignition::common::PluginLoader pl;
  pl.LoadLibrary(path);

  plugin = pl.Instantiate("test::util::DummySinglePlugin");
  EXPECT_FALSE(plugin.IsEmpty());

  ignition::common::PluginPtr otherPlugin =
      pl.Instantiate("test::util::DummySinglePlugin");
  EXPECT_FALSE(otherPlugin.IsEmpty());

  EXPECT_TRUE(plugin != otherPlugin);
  EXPECT_FALSE(plugin == otherPlugin);

  otherPlugin = plugin;
  EXPECT_TRUE(plugin == otherPlugin);
  EXPECT_FALSE(plugin != otherPlugin);

  igndbg << "Testing sets and maps with PluginPtr and PluginPtr\n";
  TestSetAndMapUsage<
      ignition::common::PluginPtr,
      ignition::common::PluginPtr>(
        pl, plugin);

  igndbg << "Testing sets and maps with PluginPtr and "
         << "SomeSpecializedPluginPtr\n";
  TestSetAndMapUsage<
      ignition::common::PluginPtr,
      SomeSpecializedPluginPtr>(
        pl, plugin);

  igndbg << "Testing sets and maps with SomeSpecializedPluginPtr and "
         << "AnotherSpecializedPluginPtr\n";
  TestSetAndMapUsage<
      SomeSpecializedPluginPtr,
      AnotherSpecializedPluginPtr>(
        pl, plugin);

  igndbg << "Testing sets and maps with AnotherSpecializedPluginPtr and "
         << "SingleSpecializedPluginPtr\n";
  TestSetAndMapUsage<
      AnotherSpecializedPluginPtr,
      SingleSpecializedPluginPtr>(
        pl, plugin);

  ignition::common::ConstPluginPtr c_plugin(plugin);
  EXPECT_FALSE(c_plugin.IsEmpty());
  EXPECT_TRUE(c_plugin == plugin);

  c_plugin = nullptr;
  EXPECT_TRUE(c_plugin.IsEmpty());

  c_plugin = otherPlugin;
  EXPECT_FALSE(c_plugin.IsEmpty());
  EXPECT_TRUE(c_plugin == otherPlugin);
}

/////////////////////////////////////////////////
void SetSomeValues(std::shared_ptr<test::util::DummySetterBase> setter)
{
  setter->SetIntegerValue(2468);
  setter->SetDoubleValue(6.28);
  setter->SetName("Changed using shared_ptr");
}

/////////////////////////////////////////////////
void CheckSomeValues(
    std::shared_ptr<test::util::DummyIntBase> getInt,
    std::shared_ptr<test::util::DummyDoubleBase> getDouble,
    std::shared_ptr<test::util::DummyNameBase> getName)
{
  EXPECT_EQ(2468, getInt->MyIntegerValueIs());
  EXPECT_NEAR(6.28, getDouble->MyDoubleValueIs(), 1e-8);
  EXPECT_EQ(std::string("Changed using shared_ptr"), getName->MyNameIs());
}

/////////////////////////////////////////////////
TEST(PluginPtr, as_shared_ptr)
{
  const std::string path = GetPluginLibraryPath();
  ASSERT_FALSE(path.empty());

  ignition::common::PluginLoader pl;
  pl.LoadLibrary(path);

  std::shared_ptr<test::util::DummyIntBase> int_ptr =
      pl.Instantiate("test::util::DummyMultiPlugin")->
        as_shared_ptr<test::util::DummyIntBase>(
          "test::util::DummyIntBase");
  EXPECT_TRUE(int_ptr.get());
  EXPECT_EQ(5, int_ptr->MyIntegerValueIs());

  SomeSpecializedPluginPtr plugin =
      pl.Instantiate("test::util::DummyMultiPlugin");


  usedSpecializedInterfaceAccess = false;
  std::shared_ptr<test::util::DummySetterBase> setter =
      plugin->as_shared_ptr<test::util::DummySetterBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  ASSERT_TRUE(setter.get());

  usedSpecializedInterfaceAccess = false;
  std::shared_ptr<SomeInterface> someInterface =
      plugin->as_shared_ptr<SomeInterface>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  EXPECT_FALSE(someInterface.get());

  usedSpecializedInterfaceAccess = false;
  std::shared_ptr<test::util::DummyIntBase> getInt =
      plugin->as_shared_ptr<test::util::DummyIntBase>();
  EXPECT_TRUE(usedSpecializedInterfaceAccess);
  ASSERT_TRUE(getInt.get());

  std::shared_ptr<test::util::DummyDoubleBase> getDouble =
      plugin->as_shared_ptr<test::util::DummyDoubleBase>(
        "test::util::DummyDoubleBase");
  ASSERT_TRUE(getDouble.get());

  std::shared_ptr<test::util::DummyNameBase> getName =
      plugin->as_shared_ptr<test::util::DummyNameBase>(
        "test::util::DummyNameBase");
  ASSERT_TRUE(getName.get());

  SetSomeValues(setter);
  CheckSomeValues(getInt, getDouble, getName);
}

/////////////////////////////////////////////////
ignition::common::PluginPtr GetSomePlugin(const std::string &_path)
{
  ignition::common::PluginLoader pl;
  pl.LoadLibrary(_path);

  return pl.Instantiate("test::util::DummyMultiPlugin");
}

/////////////////////////////////////////////////
// Note (MXG): According to some online discussions, there is no guarantee
// that a correct number of calls to dlclose(void*) will actually unload the
// shared library. In fact, there is no guarantee that a dynamically loaded
// library from dlopen will ever be unloaded until the program is terminated.
// This may cause dlopen(~, RTLD_NOLOAD) to return a non-null handle even if
// we are managing the handles correctly. If the test for
// EXPECT_EQ(nullptr, dlHandle) is found to fail occasionally, we should
// consider removing it because it may be unreliable. At the very least, if
// it fails very infrequently, then we can safely consider the failures to be
// false negatives.
#define CHECK_FOR_LIBRARY(_path, _isLoaded)\
{\
  void *dlHandle = dlopen(_path.c_str(), \
                          RTLD_NOLOAD | RTLD_LAZY | RTLD_GLOBAL);\
\
  if(_isLoaded)\
    EXPECT_NE(nullptr, dlHandle);\
  else\
    EXPECT_EQ(nullptr, dlHandle);\
\
  if(dlHandle)\
    dlclose(dlHandle);\
}

/////////////////////////////////////////////////
TEST(PluginPtr, LibraryManagement)
{
  const std::string path = GetPluginLibraryPath();

  // Use scoping to destroy somePlugin
  {
    ignition::common::PluginPtr somePlugin = GetSomePlugin(path);
    EXPECT_TRUE(somePlugin);

    CHECK_FOR_LIBRARY(path, true);
  }

  CHECK_FOR_LIBRARY(path, false);

  // Test that we can forget libraries
  {
    ignition::common::PluginLoader pl;
    pl.LoadLibrary(path);

    CHECK_FOR_LIBRARY(path, true);

    EXPECT_TRUE(pl.ForgetLibrary(path));

    CHECK_FOR_LIBRARY(path, false);
  }

  // Test that we can forget libraries, but the library will remain loaded if
  // a plugin instance is still using it.
  {
    ignition::common::PluginPtr plugin;

    ignition::common::PluginLoader pl;
    pl.LoadLibrary(path);

    CHECK_FOR_LIBRARY(path, true);

    plugin = pl.Instantiate("test::util::DummyMultiPlugin");
    EXPECT_TRUE(plugin);

    EXPECT_TRUE(pl.ForgetLibrary(path));

    CHECK_FOR_LIBRARY(path, true);
  }

  // Check that the library will be unloaded once the plugin instance is deleted
  CHECK_FOR_LIBRARY(path, false);

  // Check that we can unload libraries based on plugin name
  {
    ignition::common::PluginLoader pl;
    pl.LoadLibrary(path);

    CHECK_FOR_LIBRARY(path, true);

    pl.ForgetLibraryOfPlugin("test::util::DummyMultiPlugin");

    CHECK_FOR_LIBRARY(path, false);
  }

  // Check that the std::shared_ptrs that we provide for interfaces will
  // successfully keep the library loaded.
  {
    std::shared_ptr<test::util::DummyNameBase> interface;

    CHECK_FOR_LIBRARY(path, false);
    {
      interface = GetSomePlugin(path)->as_shared_ptr<test::util::DummyNameBase>(
            "test::util::DummyNameBase");
      EXPECT_EQ("DummyMultiPlugin", interface->MyNameIs());

      CHECK_FOR_LIBRARY(path, true);
    }

    EXPECT_EQ("DummyMultiPlugin", interface->MyNameIs());

    CHECK_FOR_LIBRARY(path, true);
  }

  CHECK_FOR_LIBRARY(path, false);

  // Check that mulitple PluginLoaders can work side-by-side
  {
    ignition::common::PluginLoader pl1;
    pl1.LoadLibrary(path);

    CHECK_FOR_LIBRARY(path, true);

    {
      ignition::common::PluginLoader pl2;
      pl2.LoadLibrary(path);

      CHECK_FOR_LIBRARY(path, true);
    }

    CHECK_FOR_LIBRARY(path, true);
  }

  CHECK_FOR_LIBRARY(path, false);
}


/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

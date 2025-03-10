/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <gtest/gtest.h>

#include "ignition/msgs.hh"
#include "ignition/transport.hh"
#include "ignition/gazebo/components/Model.hh"
#include "ignition/gazebo/components/Name.hh"

#include "ignition/gazebo/Server.hh"
#include "ignition/gazebo/test_config.hh"  // NOLINT(build/include)

#include "../helpers/EnvTestFixture.hh"
#include "../helpers/Relay.hh"

using namespace ignition;
using namespace gazebo;
using namespace std::chrono_literals;

class WorldControlState : public InternalFixture<::testing::Test>
{
};

/////////////////////////////////////////////////
TEST_F(WorldControlState, SetState)
{
  common::Console::SetVerbosity(4);

  ServerConfig serverConfig;
  Server server(serverConfig);
  server.SetUpdatePeriod(1us);
  transport::Node node;

  bool received = false;
  std::function<void(const msgs::SerializedStepMap &)> cb =
      [&](const msgs::SerializedStepMap &_res)
  {
    if (_res.stats().iterations() == 1)
    {
      EntityComponentManager localEcm;
      localEcm.SetState(_res.state());
      Entity entity = localEcm.CreateEntity();
      localEcm.CreateComponent(entity, components::Name("box"));

      msgs::WorldControlState req;
      req.mutable_state()->CopyFrom(localEcm.State());

      std::function<void(const ignition::msgs::Boolean &, const bool)> cb2 =
          [](const ignition::msgs::Boolean &/*_rep*/, const bool _result)
          {
            if (!_result)
              ignerr << "Error sharing WorldControl info with the server.\n";
          };
      node.Request("/world/default/control/state", req, cb2);
    }
    received = true;
  };

  // Create a system that checks for state changes in the ECM
  ignition::gazebo::test::Relay testSystem;

  testSystem.OnUpdate([](const gazebo::UpdateInfo &_info,
    gazebo::EntityComponentManager &_ecm)
    {
      // After the first iteration, there should be an entity with the name
      // "box"
      bool hasBox = false;
      _ecm.Each<ignition::gazebo::components::Name>(
        [&](const ignition::gazebo::Entity &,
            const ignition::gazebo::components::Name *_name)->bool
        {
          if (_name->Data() == "box")
          {
            hasBox = true;
          }
          return true;
        });
      if (_info.iterations > 1)
        EXPECT_TRUE(hasBox);
      else
        EXPECT_FALSE(hasBox);
    });
  server.AddSystem(testSystem.systemPtr);

  EXPECT_TRUE(node.Subscribe("/world/default/state", cb));

  // Run the server once
  server.RunOnce(false);

  // Wait for the state cb to take place
  unsigned int sleep = 0u;
  unsigned int maxSleep = 30u;
  while (!received && sleep++ < maxSleep)
    IGN_SLEEP_MS(100);

  // Run again, and the test system should now find an entity with the name
  // "box"
  server.RunOnce(false);
}

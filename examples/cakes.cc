/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <thread>

#include <ignition/common/SpecializedPluginPtr.hh>

#include "plugins/cakes.hh"

using namespace ignition::common::examples;

/////////////////////////////////////////////////
/// \brief This typedef creates a WorkerPtr type that provides very high-speed
/// access to the specified list of interfaces.
using WorkerPtr = ignition::common::SpecializedPluginPtr<
    IngredientFetcher,
    BatterMixer,
    OvenHandler,
    Froster,
    Credentials>;

/////////////////////////////////////////////////
class Kitchen
{
  public: CounterTop ingredientCounterTop;
  public: CounterTop batterCounterTop;
  public: CounterTop bakedCounterTop;
  public: CounterTop finishedCounterTop;
  public: std::vector<Oven> ovens;

  public: unsigned int numOrders;
};

/////////////////////////////////////////////////
std::chrono::milliseconds convertToSimTime(const std::chrono::seconds t)
{
  std::chrono::milliseconds simTime = t;
  simTime /= SimulationFactor;
  return simTime;
}

/////////////////////////////////////////////////
Cake::Status Cake::GetStatus() const
{
  return this->status;
}

/////////////////////////////////////////////////
Cake::Cake(const std::chrono::seconds _duration)
  : status(UNMIXED_INGREDIENTS)
{
  std::this_thread::sleep_for(convertToSimTime(_duration));
}

/////////////////////////////////////////////////
bool Cake::MixIngredients(const std::chrono::seconds _duration)
{
  std::this_thread::sleep_for(convertToSimTime(_duration));

  if (UNMIXED_INGREDIENTS == status)
  {
    status = UNBAKED_BATTER;
    return true;
  }

  status = RUINED;
  return false;
}

/////////////////////////////////////////////////
bool Cake::AddFrosting(const std::chrono::seconds _duration)
{
  std::this_thread::sleep_for(convertToSimTime(_duration));

  if (BAKED == status)
  {
    status = FROSTED;
    return true;
  }

  status = RUINED;
  return false;
}

/////////////////////////////////////////////////
void Cake::Ruin()
{
  status = RUINED;
}

/////////////////////////////////////////////////
void Cake::FinishHeating()
{
  if (UNBAKED_BATTER == status)
  {
    status = BAKED;
    return;
  }

  status = RUINED;
}

/////////////////////////////////////////////////
void BakeCakes(const WorkerPtr &_worker, Kitchen &_kitchen)
{
  bool needMoreCakes = true;
  while (needMoreCakes)
  {


    // If this worker is an OvenHandler, have them check the oven alarms. If a
    // cake is close to being ready, the OvenHandler should wait for it and then
    // pull the cake out.
    if (_worker->QueryInterface<OvenHandler>() &&
        _worker->QueryInterface<OvenHandler>()->CheckAlarms(
          _kitchen.ovens, _kitchen.bakedCounterTop))
    {
      // We pulled a cake out of the oven, so let's start the work cycle over
      continue;
    }


  }
}

int main()
{

}

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

std::atomic_ulong Cake::ruinCount(0u);

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
std::chrono::milliseconds convertToActualTime(const std::chrono::seconds t)
{
  std::chrono::milliseconds actualTime = t;
  actualTime /= SimulationFactor;
  return actualTime;
}

/////////////////////////////////////////////////
std::chrono::seconds convertToSimTime(const std::chrono::nanoseconds t)
{
  return std::chrono::duration_cast<std::chrono::seconds>(
        SimulationFactor*t);
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
  std::this_thread::sleep_for(convertToActualTime(_duration));
}

/////////////////////////////////////////////////
bool Cake::MixIngredients(const std::chrono::seconds _duration)
{
  std::this_thread::sleep_for(convertToActualTime(_duration));

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
  std::this_thread::sleep_for(convertToActualTime(_duration));

  if (BAKED == status)
  {
    status = FINISHED;
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
std::size_t Cake::GetRuinCount() const
{
  return ruinCount;
}

/////////////////////////////////////////////////
Cake::~Cake()
{
  if (FINISHED != this->status)
    ++ruinCount;
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
void SetAlarm(OvenAlarm &_alarm)
{
  std::unique_lock<std::mutex> lock(_alarm.set);
  std::this_thread::sleep_for(convertToActualTime(RequiredBakeTime));
  lock.unlock();
  _alarm.ringing.notify_all();
}

/////////////////////////////////////////////////
bool Oven::InsertCake(Cake &&_cake)
{
  if (this->bakingCake.size() > 0)
    return false;

  bakingCake.push_back(std::move(_cake));
  this->bakeStartTime = std::chrono::steady_clock::now();

  std::thread alarm(&SetAlarm);
  alarm.detach();

  return true;
}

/////////////////////////////////////////////////
Cake Oven::RemoveCake()
{
  if (bakingCake.empty())
  {
    throw std::runtime_error("Someone is trying to burn down the kitchen!");
  }

  const std::chrono::time_point<std::chrono::steady_clock> currentTime =
      std::chrono::steady_clock::now();

  const std::chrono::seconds bakeDuration =
      convertToSimTime(currentTime - bakeStartTime);

  Cake removedCake = std::move(bakingCake.front());
  bakingCake.pop_front();

  if (bakeDuration < RequiredBakeTime)
    removedCake.Ruin();
  else if (bakeDuration > MaximumBakeTime)
    removedCake.Ruin();
  else
    removedCake.FinishHeating();

  return removedCake;
}

/////////////////////////////////////////////////
bool Oven::Occupied() const
{
  return !bakingCake.empty();
}

/////////////////////////////////////////////////
std::chrono::seconds Oven::TimeRemaining() const
{
  if (bakingCake.empty())
    return 0s;

  const auto currentTime = std::chrono::steady_clock::now();

  const std::chrono::seconds bakeDuration =
      convertToSimTime(currentTime - bakeStartTime);

  if (bakeDuration > RequiredBakeTime)
    return 0s;

  return RequiredBakeTime - bakeDuration;
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
    //
    // We trigger this job before any others each loop because we don't want the
    // OvenHandler to get tied up with another job when a cake will be ready to
    // pull out soon. Otherwise, the cake could get ruined if it sits in the
    // oven for too long.
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

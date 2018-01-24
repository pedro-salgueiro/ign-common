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

#ifndef IGNITION_COMMON_EXAMPLES_PLUGINS_CAKES_HH_
#define IGNITION_COMMON_EXAMPLES_PLUGINS_CAKES_HH_

#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <deque>
#include <vector>

#include <ignition/common/PluginMacros.hh>

namespace ignition
{
  namespace common
  {
    namespace examples
    {
      // Forward declaration
      class Oven;

      /// \brief This is the multiplier for how much faster the simulation runs
      /// than the real-time baking would take.
      const int SimulationFactor = 18000;

      /// \brief A cake. It might need to be baked before it is safe to consume.
      class Cake
      {
        public: enum Status
        {
          UNMIXED_INGREDIENTS = 0,
          UNBAKED_BATTER,
          BAKED,
          RUINED,
          FROSTED
        };

        /// \brief Get the status of this cake
        public: Status GetStatus() const;

        /// \brief Construct a cake, starting out as a bunch of unmixed
        /// ingredients.
        /// \brief _duration The amount of time it took the worker to fetch the
        /// ingredients.
        public: Cake(std::chrono::seconds _duration);

        /// \brief Mix the ingredients. This must be called on a cake with the
        /// status of UNMIXED_INGREDIENTS, otherwise this will result in a
        /// RUINED status.
        /// \param[in] _duration The amount of time it took the worker to mix
        /// the cake. The simulation will run at 18000x this speed.
        public: bool MixIngredients(std::chrono::seconds _duration);

        /// \brief Add frosting to the cake. This must be called on a cake with
        /// the status of BAKED, otherwise this will result in a RUINED status.
        /// \param[in] _duration The amount of time it takes you to add the
        /// frosting. The simulation will run at 18000x this speed.
        public: bool AddFrosting(std::chrono::seconds _duration);

        /// \brief Ruin this cake. Not recommended.
        public: void Ruin();

        // The oven class is a friend, because only the oven can heat the cake.
        friend class Oven;

        /// \brief Finish heating the cake. This gets called by the Oven class,
        /// so pass the cake into an oven in order to heat it.
        private: void FinishHeating();

        /// \brief The current status of this cake.
        /// \sa GetStatus()
        private: Status status;
      };

      /// \brief The alarm on a simulated oven.
      class OvenAlarm
      {
        /// \brief This mutex indicates that the oven is currently being handled
        /// by an OvenHandler.
        public: std::mutex beingHandled;

        /// \brief This condition variable will notify the OvenHandlers whenever
        /// a cake is ready to be removed.
        public: std::condition_variable ringing;
      };

      /// \brief A simulated oven.
      class Oven
      {
        /// \brief Put a cake into the oven so it can bake. This will return
        /// false if another cake is already in the oven. Our kitchen has very
        /// small ovens, so each oven can only fit one cake at a time.
        public: bool InsertCake(Cake &&_cake);

        /// \brief Pull the cake out of the oven. If called too early or too
        /// late, this will return a RUINED cake. When called within the proper
        /// time range, it will successfully return a BAKED cake.
        public: Cake RemoveCake();

        /// \brief Returns true if this simulated oven is already occupied by a
        /// cake.
        public: bool Occupied() const;

        /// \brief If the a cake is in this oven, get the amount of time that is
        /// remaining for it. If this returns zero and the oven is occupied, a
        /// cake is ready.
        public: std::chrono::seconds TimeRemaining() const;

        /// \brief The alarm tells the OvenHandlers when a new cake is ready.
        public: OvenAlarm alarm;
      };

      using namespace std::chrono_literals;
      /// \brief Our baking simulation will run at 18000x real speed, so a cake
      /// that takes 30min to bake in real life will take 100ms to bake in the
      /// simulation.
      const std::chrono::minutes RequiredBakeTime = 30min;

      /// \brief A simulated countertop where ingredients or cakes may be placed
      /// until they can be used. Each countertop is designated for one type of
      /// cake.
      class CounterTop
      {
        public: bool PlaceCake(Cake &&_cake);

        public: Cake RemoveCake();

        public: std::size_t Limit() const;

        public: std::size_t RemainingSpace() const;

        public: void SetLimit(std::size_t _limit);

        public: std::condition_variable spaceAvailable;

        private: std::deque<Cake> cakes;

        private: std::mutex beingHandled;

        private: std::mutex beingViewed;
      };

      /// \brief A simulated worker who knows how to fetch ingredients.
      class IngredientFetcher
      {
        /// \brief Returns true if the correct ingredients were fetched,
        /// otherwise returns false.
        public: virtual Cake FetchIngredients() = 0;

        /// \brief Virtual destructor
        public: virtual ~IngredientFetcher() = default;

        IGN_COMMON_SPECIALIZE_INTERFACE(
            ignition::common::examples::IngredientFetcher)
      };

      /// \brief A simulated worker who knows how to mix batter. Some
      /// BatterMixers might be faster than other BatterMixers.
      class BatterMixer
      {
        /// \brief Have this worker mix some batter. The incoming cake should
        /// have a Cake::UNMIXED_INGREDIENTS status, and the BatterMixer should
        /// return it with UNBAKED_BATTER status.
        public: virtual void MixBatter(Cake &_cake) = 0;

        /// \brief Virtual destructor
        public: virtual ~BatterMixer() = default;

        IGN_COMMON_SPECIALIZE_INTERFACE(
            ignition::common::examples::BatterMixer)
      };

      /// \brief A simulated worker who knows how to put cakes into the oven and
      /// take them out.
      ///
      /// If the OvenHandler is clumsy, the cake might fall on the floor instead
      /// of making it into the oven.
      ///
      /// If an OvenHandler does not respond to an alarm in time, the cake will
      /// be ruined.
      class OvenHandler
      {
        public: virtual bool InsertCake(
          Oven &_oven, Cake &&_cake) = 0;

        public: virtual void ListenForAlarms(
          const std::vector<OvenAlarm*> &_alarms,
          const CounterTop *_counterTopForCooling) = 0;

        /// \brief Have this worker check the alarms
        public: virtual bool CheckAlarms(
          const std::vector<Oven> &_ovens,
          CounterTop &_counterTopForCooling) const = 0;

        public: virtual ~OvenHandler() = default;

        IGN_COMMON_SPECIALIZE_INTERFACE(
            ignition::common::examples::OvenHandler)
      };

      /// \brief A simulated worker who can apply frosting to a cake.
      class Froster
      {
        /// \brief Have this worker apply frosting to a baked cake. The incoming
        /// cake should have a Cake::BAKED status, and the FrostingApplicator
        /// should return it with FROSTED status.
        public: virtual void ApplyFrosting(Cake &_cake) = 0;

        /// \brief Virtual destructor
        public: virtual ~Froster() = default;

        IGN_COMMON_SPECIALIZE_INTERFACE(
            ignition::common::examples::Froster)
      };

      /// \brief The simulated worker's credentials. Every worker must have
      /// credentials, or else they won't be allowed into the kitchen.
      class Credentials
      {
        /// \brief The title that the worker has
        public: virtual std::string Title() const = 0;

        /// \brief A description of how well they do their job(s)
        public: virtual std::string Description() const = 0;

        /// \brief Virtual destructor
        public: virtual ~Credentials() = default;

        IGN_COMMON_SPECIALIZE_INTERFACE(
            ignition::common::examples::Credentials)
      };
    }
  }
}

#endif

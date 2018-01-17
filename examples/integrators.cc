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

#include <iostream>
#include <chrono>
#include <cassert>
#include <cmath>

#include <ignition/common/PluginLoader.hh>

#include "plugins/Interfaces.hh"

using NumericalIntegrator = ignition::common::examples::NumericalIntegrator;

// The macro that this uses is provided as a compile definition in the
// examples/CMakeLists.txt file.
const std::string PluginLibDir = IGN_COMMON_EXAMPLE_PLUGIN_LIBDIR;

/// \brief Return structure for numerical integration test results. If the name
/// is blank, that means the test was not run.
struct TestResult
{
  public: std::string name;
  public: long timeSpent_us;
  public: std::vector<double> percentError;
};

/// \brief A system that each NumericalIntegrator can be tested against.
struct System
{
  /// \brief A system of Ordinary Differential Equations to test the numerical
  /// integrators.
  public: NumericalIntegrator::SystemODE ode;

  /// \brief The initial state of the system.
  public: NumericalIntegrator::State initialState;

  /// \brief The initial time of the system.
  public: NumericalIntegrator::Time initialTime;

  /// \brief A function that describes the exact solution of the ODE system that
  /// is being tested.
  public: NumericalIntegrator::ExactSolution exact;
};

/// \brief Compute the component-wise percent error of the estimate produced by
/// a numerical integrator, compared to the theoretical exact solution of the
/// system.
std::vector<double> ComputeError(
    const NumericalIntegrator::State &_estimate,
    const NumericalIntegrator::State &_exact)
{
  assert(_estimate.size() == _exact.size());
  std::vector<double> result(_estimate.size());

  for(std::size_t i = 0 ; i < _estimate.size(); ++i)
    result[i] = (_estimate[i] - _exact[i])/_exact[i] * 100.0;

  return result;
}

/// \brief Pass in a plugin that provides the numerical integration interface.
/// The numerical integration results of the plugin will be tested against the
/// results of the _exactFunction.
TestResult TestIntegrator(
    const std::string &_name,
    const ignition::common::PluginPtr &_plugin,
    const System &_system,
    const double _timeStep,
    const unsigned int _numSteps)
{
  NumericalIntegrator* integrator =
      _plugin->QueryInterface<NumericalIntegrator>(
        "ignition::common::examples::NumericalIntegrator");

  if(!integrator)
  {
    std::cout << "The plugin named [" << _name << "] does not provide a "
              << "NumericalIntegrator interface. It will not be tested."
              << std::endl;
    return TestResult();
  }

  TestResult result;
  result.name = _name;

  integrator->SetFunction(_system.ode);
  integrator->SetTimeStep(_timeStep);

  double time = _system.initialTime;
  NumericalIntegrator::State state = _system.initialState;

  auto performanceStart = std::chrono::high_resolution_clock::now();
  for(std::size_t i=0; i < _numSteps; ++i)
  {
    state = integrator->Integrate(time, state);
    time += integrator->GetTimeStep();
  }
  auto performanceStop = std::chrono::high_resolution_clock::now();

  result.timeSpent_us = std::chrono::duration_cast<std::chrono::microseconds>(
        performanceStop - performanceStart).count();

  result.percentError = ComputeError(state, _system.exact(time));

  return result;
}

/// \brief Create a parabolic system, like an object falling under the influence
/// of gravity.
System CreateParabolicSystem(
    const double p0 = 0.0,
    const double t0 = 0.0,
    const double v0=1.0,
    const double a=2.0)
{
  System parabola;

  parabola.initialState = {p0};
  parabola.initialTime = t0;

  parabola.exact = [=](const NumericalIntegrator::Time _t)
      -> NumericalIntegrator::State
  {
    return {0.5*a*std::pow(_t+t0, 2) + v0*(_t+t0) + p0};
  };

  parabola.ode = [=](const NumericalIntegrator::Time _t,
                     const NumericalIntegrator::State & /*_state*/)
      -> NumericalIntegrator::State
  {
    return {a*(_t+t0) + v0};
  };

  return parabola;
}

int main()
{

}

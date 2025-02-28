// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_PARAM_PARAM_H_
#define CORE_PARAM_PARAM_H_

#include <cinttypes>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "core/analysis/style.h"
#include "core/param/param_group.h"
#include "core/real_t.h"
#include "core/util/root.h"
#include "core/util/type.h"

namespace bdm {

class Simulation;

struct Param {
  static void RegisterParamGroup(ParamGroup* param);

  Param();

  ~Param();

  Param(const Param& other);

  void Restore(Param&& other);

  /// Returns a Json representation of this parameter and all
  /// ParamGroupeter.
  /// The groups_ data member has been flattened to simplify
  /// JSON merge patches (https://tools.ietf.org/html/rfc7386).
  std::string ToJsonString() const;

  /// Applies a JSON merge patch (https://tools.ietf.org/html/rfc7386)
  /// to this parameter and ParamGroupeter.
  /// The groups_ data member must be flattened. See output of
  /// `ToJsonString()`.
  void MergeJsonPatch(const std::string& patch);

  template <typename TParamGroup>
  const TParamGroup* Get() const {
    if (groups_.find(TParamGroup::kUid) != groups_.end()) {
      return bdm_static_cast<const TParamGroup*>(groups_.at(TParamGroup::kUid));
    } else {
      Log::Error("TParamGroup::Get",
                 "Couldn't find the requested group parameter.");
      return nullptr;
    }
  }

  template <typename TParamGroup>
  TParamGroup* Get() {
    if (groups_.find(TParamGroup::kUid) != groups_.end()) {
      return bdm_static_cast<TParamGroup*>(groups_.at(TParamGroup::kUid));
    } else {
      Log::Error("TParamGroup::Get",
                 "Couldn't find the requested group parameter.");
      return nullptr;
    }
  }

  // simulation values ---------------------------------------------------------
  /// Set random number seed.\n
  /// The pseudo random number generator (prng) of each thread will be
  /// initialized as follows:
  /// `prng[tid].SetSeed(random_seed * (tid + 1));`\n
  /// Default value: `4357`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     random_seed = 4357
  uint64_t random_seed = 4357;

  /// List of default operation names that should not be scheduled by default
  /// Default value: `{}`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     unschedule_default_operations = ["mechanical forces", "load
  ///     balancing"]
  std::vector<std::string> unschedule_default_operations;

  /// Variable which specifies method using for solving differential equation
  /// {"Euler", "RK4"}.
  enum NumericalODESolver { kEuler = 1, kRK4 = 2 };
  NumericalODESolver numerical_ode_solver = NumericalODESolver::kEuler;

  /// Output Directory name used to store visualization and other files.\n
  /// Path is relative to working directory.\n
  /// Default value: `"output"`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     output_dir = "output"
  std::string output_dir = "output";

  /// The method used to query the environment of a simulation object.
  /// Default value: `"uniform_grid"`\n
  /// Other allowed values: `"kd_tree", "octree"`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     environment = "uniform_grid"
  std::string environment = "uniform_grid";

  /// The depth of the kd tree if it's set as the environment (see
  /// Param::environment). For more information see:
  /// https://github.com/jlblancoc/nanoflann\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     nanoflann_depth = 10
  uint32_t nanoflann_depth = 10;

  /// The bucket size of the octree if it's set as the environment (see
  /// Param::environment). For more information see:
  /// https://github.com/jbehley/octree
  /// TOML config file:
  ///
  ///     [simulation]
  ///     unibn_bucketsize = 16
  uint32_t unibn_bucketsize = 16;

  /// If set to true (default), BioDynaMo will automatically delete all contents
  /// inside `Param::output_dir` at the beginning of the simulation.
  /// Use with caution in combination with `Param::output_dir`. If you do not
  /// want to delete the content, set this parameter to false. BioDynaMo then
  /// organizes your simulation outputs in additional subfolders labelled with
  /// the date-time of your simulation `YYYY-MM-DD-HH:MM:SS`. Note that you will
  /// inevitably use more disk space with this option.
  bool remove_output_dir_contents = true;

  /// Backup file name for full simulation backups\n
  /// Path is relative to working directory.\n
  /// Default value: `""` (no backups will be made)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     backup_file = <path>/<filename>.root
  /// Command line argument: `-b, --backup`
  std::string backup_file = "";

  /// File name to restore simulation from\n
  /// Path is relative to working directory.\n
  /// Default value: `""` (no restore will be made)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     restore_file = <path>/<filename>.root
  /// Command line argument: `-r, --restore`
  std::string restore_file = "";

  /// Specifies the interval (in seconds) in which backups will be performed.\n
  /// Default Value: `1800` (every half an hour)\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     backup_interval = 1800  # backup every half an hour
  uint32_t backup_interval = 1800;

  /// Time between two simulation steps, in hours.
  /// Default value: `0.01`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     time_step = 0.0125
  real_t simulation_time_step = 0.01;

  /// Maximum jump that a point mass can do in one time step. Useful to
  /// stabilize the simulation\n
  /// Default value: `3.0`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     max_displacement = 3.0
  real_t simulation_max_displacement = 3.0;

  enum BoundSpaceMode {
    /// The simulation space grows to encapsulate all agents.
    kOpen = 0,
    /// Enforce an artificial cubic bound around the simulation space.
    /// The dimensions of this cube are determined by parameter
    /// `min_bound` and `max_bound`.\n
    /// If agents move outside the cube they are moved back inside.
    kClosed,
    /// Enforce an artificial cubic bound around the simulation space.
    /// The dimensions of this cube are determined by parameter
    /// `min_bound` and `max_bound`.\n
    /// Agents that move outside the cube are moved back in on the opposite
    /// side.
    kTorus
  };

  /// Default value: `open` (simulation space is "infinite")\n
  /// \see BoundSpaceMode
  /// TOML config file:
  ///
  ///     [simulation]
  ///     bound_space = "open"
  BoundSpaceMode bound_space = kOpen;

  /// Minimum allowed value for x-, y- and z-position if simulation space is
  /// bound (@see `bound_space`).\n
  /// Default value: `0`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     min_bound = 0
  real_t min_bound = 0;

  /// Maximum allowed value for x-, y- and z-position if simulation space is
  /// bound (@see `bound_space`).\n
  /// Default value: `100`\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     max_bound = 100
  real_t max_bound = 100;

  /// Define the boundary condition of the diffusion grid [open, closed,
  /// Neumann, Dirichlet]\n
  /// Default value: `"Neumann"`\n TOML config file:
  ///
  ///     [simulation]
  ///     diffusion_boundary_condition = "Neumann"
  std::string diffusion_boundary_condition = "Neumann";

  /// A string for determining diffusion type within the simulation space.
  /// current inputs include "euler" and "runge-kutta".
  /// Default value: `"euler"`\n
  /// TOML config file:
  ///
  ///        [simulation]
  ///        diffusion_method = <diffusion method>
  ///

  std::string diffusion_method = "euler";

  /// Calculate the diffusion gradient for each substance.\n
  /// TOML config file:
  /// Default value: `true`\n
  ///
  ///     [simulation]
  ///     calculate_gradients = true
  bool calculate_gradients = true;

  /// List of thread-safety mechanisms \n
  /// `kNone`: \n
  /// `kUserSpecified`: The user has to define all agent that must
  /// not be processed in parallel. \see `Agent::CriticalRegion`.\n
  /// `kAutomatic`: The simulation automatically locks all agents
  /// of the microenvironment.
  enum ThreadSafetyMechanism { kNone = 0, kUserSpecified, kAutomatic };

  /// Select the thread-safety mechanism.\n
  /// Possible values are: none, user-specified, automatic.\n
  /// TOML config file:
  ///
  ///     [simulation]
  ///     thread_safety_mechanism = "none"
  ThreadSafetyMechanism thread_safety_mechanism =
      ThreadSafetyMechanism::kUserSpecified;

  // visualization values ------------------------------------------------------

  /// Name of the visualization engine to use for visualizaing BioDynaMo
  /// simulations\n
  /// Default value: `"paraview"`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     adaptor = <name_of_adaptor>
  std::string visualization_engine = "paraview";

  /// Use ParaView Catalyst for insitu visualization.\n
  /// Insitu visualization supports live visualization
  /// and rendering without writing files to the harddisk.\n
  ///
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     insitu = false
  bool insitu_visualization = false;

  /// Write data to file for post-simulation visualization
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export = false
  bool export_visualization = false;

  /// Use ROOT for enable visualization.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     root = false
  bool root_visualization = false;

  /// Enable insitu visualization with a custom python pipeline
  /// Default value:
  /// `"<path-to-bdm>/include/core/visualization/paraview/default_insitu_pipeline.py"`\n
  /// TOML config file:
  ///     [visualization]
  ///     pv_insitu_pipeline = ""
  std::string pv_insitu_pipeline =
      Concat(std::getenv("BDMSYS"),
             "/include/core/visualization/paraview/default_insitu_pipeline.py");

  /// Arguments that will be passed to the python ParaView insitu pipeline
  /// specified in `Param::pv_insitu_pipeline`.\n
  /// The arguments will be passed to the ExtendDefaultPipeline function
  /// `def ExtendDefaultPipeline(renderview, coprocessor, datadescription,
  /// script_args):`
  /// as fourth argument.\n
  /// Default value: ""\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     pv_insitu_pipelinearguments = ""
  std::string pv_insitu_pipelinearguments = "";

  /// If `export_visualization` is set to true, this parameter specifies
  /// how often it should be exported. 1 = every timestep, 10: every 10
  /// time steps.\n
  /// Default value: `1`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     interval = 1
  uint32_t visualization_interval = 1;

  /// If `export_visualization` is set to true, this parameter specifies
  /// if the ParaView pvsm file will be generated!\n
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export_generate_pvsm = true
  bool visualization_export_generate_pvsm = true;

  /// Specifies which agents should be visualized. \n
  /// Every agent defines the minimum set of data members which
  /// are required to visualize it. (e.g. Cell: `position_` and `diameter_`).\n
  /// With this parameter it is also possible to extend the number of data
  /// members that are sent to the visualization engine.
  /// Default value: empty (no agent will be visualized)\n
  /// NB: This data member is not backed up, due to a ROOT error.
  /// TOML config file:
  ///
  ///     [visualization]
  ///     # turn on insitu or export
  ///     export = true
  ///
  ///       [[visualize_agent]]
  ///       name = "Cell"
  ///       # the following entry is optional
  ///       additional_data_members = [ "density_" ]
  ///
  ///       # The former block can be repeated for further agents
  ///       [[visualize_agent]]
  ///       name = "Neurite"
  std::map<std::string, std::set<std::string>>
      visualize_agents;  ///<  JSON_object

  struct VisualizeDiffusion {
    std::string name;
    bool concentration = true;
    bool gradient = false;
  };

  /// Specifies for which substances extracellular diffusion should be
  /// visualized.\n
  /// Default value: empty (no diffusion will be visualized)\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     # turn on insitu or export
  ///     export = true
  ///
  ///       [[visualize_diffusion]]
  ///       # Name of the substance
  ///       name = "Na"
  ///       # the following two entries are optional
  ///       #   default value for concentration is true
  ///       concentration = true
  ///       #   default value for gradient is false
  ///       gradient = false
  ///
  ///       # The former block can be repeated for further substances
  ///       [[visualize_diffusion]]
  ///       name = "K"
  ///       # default values: concentration = true and gradient = false
  std::vector<VisualizeDiffusion> visualize_diffusion;

  /// Specifies if the ParView files that are generated in export mode
  /// should be compressed.\n
  /// Default value: true\n
  /// TOML config file:
  ///
  ///     [visualization]
  ///     export = true
  ///     compress_pv_files = true
  ///
  bool visualization_compress_pv_files = true;

  // performance values --------------------------------------------------------

  /// Batch size used by the `Scheduler` to iterate over agents\n
  /// Default value: `1000`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     scheduling_batch_size = 1000
  uint64_t scheduling_batch_size = 1000;

  enum ExecutionOrder { kForEachAgentForEachOp = 0, kForEachOpForEachAgent };

  /// This parameter determines whether to execute  `kForEachAgentForEachOp`
  /// \code
  /// for (auto* agent : agents) {
  ///   for (auto* op : agent_ops) {
  ///     (*op)(agent);
  ///   }
  /// }
  /// \endcode
  /// or `kForEachOpForEachAgent`
  /// \code
  /// for (auto* op : agent_ops) {
  ///   for (auto* agent : agents) {
  ///     (*op)(agent);
  ///   }
  /// }
  /// \endcode
  ExecutionOrder execution_order = ExecutionOrder::kForEachAgentForEachOp;

  /// Calculation of the displacement (mechanical interaction) is an
  /// expensive operation. If agents do not move or grow,
  /// displacement calculation is omitted if detect_static_agents is turned
  /// on. However, the detection mechanism introduces an overhead. For dynamic
  /// simulations where agents move and grow, the overhead outweighs the
  /// benefits.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     detect_static_agents = false
  bool detect_static_agents = false;

  /// Neighbors of an agent can be cached so to avoid consecutive
  /// searches. This of course only makes sense if there is more than one
  /// `ForEachNeighbor*` operation.\n
  /// Default value: `false`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     cache_neighbors = false
  bool cache_neighbors = false;

  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     use_bdm_mem_mgr = true
  bool use_bdm_mem_mgr = true;

  /// The BioDynaMo memory manager allocates N page aligned memory blocks.
  /// The bigger N, the lower the memory overhead due to metadata storage
  /// if a lot of memory is used.\n
  /// N must be a number of two.\n
  /// Therefore, this parameter specifies the shift for N. `N = 2 ^ shift`\n
  /// Default value: `5` `-> N = 32`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mem_mgr_aligned_pages_shift = 5
  uint64_t mem_mgr_aligned_pages_shift = 5;

  /// The BioDynaMo memory manager allocates memory in increasing sizes using
  /// a geometric series. This parameter specifies the growth rate.
  /// Default value: `2.0`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mem_mgr_growth_rate = 1.1
  real_t mem_mgr_growth_rate = 1.1;

  /// The BioDynaMo memory manager can migrate memory between thread pools
  /// to avoid memory leaks.\n
  /// This parameter influences the maximum memory size in bytes before
  /// migration happens.\n
  /// The size in bytes depends on the system's page size and the parameter
  /// `mem_mgr_aligned_pages_shift` and is calculated as follows:
  /// `PAGE_SIZE * 2 ^ mem_mgr_aligned_pages_shift *
  /// mem_mgr_max_mem_per_thread_factor`\n Default value: `1`\n TOML config
  /// file:
  ///
  ///     [performance]
  ///     mem_mgr_max_mem_per_thread_factor = 1
  uint64_t mem_mgr_max_mem_per_thread_factor = 1;

  /// This parameter is used inside `ResourceManager::LoadBalance`.
  /// If it is set to true, the function will reuse existing memory to rebalance
  /// agents to NUMA nodes. (A small amount of additional memory
  /// is still required.)\n
  /// If this parameter is set to false, the balancing function will first
  /// create new objects and delete the old ones in a second step. In the worst
  /// case this will real_t the required memory for agents for.
  /// Default value: `true`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     minimize_memory_while_rebalancing = true
  bool minimize_memory_while_rebalancing = true;

  /// MappedDataArrayMode options:
  ///   `kZeroCopy`: access agent data directly only if it is
  ///                requested. \n
  ///   `kCache`:    Like `kZeroCopy` but stores the results in contiguous
  ///                array, to speed up access if it is used again.\n
  ///   `kCopy`:     Copy all data elements to a contiguous array at
  ///                initialization time. Serves requests from the cache.
  enum MappedDataArrayMode { kZeroCopy = 0, kCopy, kCache };

  /// This parameter sets the operation mode in `bdm::MappedDataArray`.\n
  /// Allowed values are defined in `MappedDataArrayMode`\n
  /// Possible values: zero-copy, cache, copy\n
  /// Default value: `zero-copy`\n
  /// TOML config file:
  ///
  ///     [performance]
  ///     mapped_data_array_mode = "zero-copy"
  Param::MappedDataArrayMode mapped_data_array_mode =
      MappedDataArrayMode::kZeroCopy;

  // development values --------------------------------------------------------
  /// Statistics of profiling data; keeps track of the execution time of each
  /// operation at every timestep.\n
  /// If set to true it prints simulation data at the end of the simulation
  /// to std::cout and a file.\n
  /// Default Value: `false`\n
  /// TOML config file:
  ///
  ///     [development]
  ///     statistics = false
  bool statistics = false;

  /// Automatically track changes in the simulation and BioDynaMo repository.
  /// If set to true, BioDynaMo scans the simulation directory and the BioDynaMo
  /// repository for changes and saves the information of the git repositories
  /// in the output directory.\n
  /// Default Value: `true`\n
#ifdef USE_LIBGIT2
  bool track_git_changes = true;
#endif  // USE_LIBGIT2

  /// Output debugging info related to running on NUMA architecture.\n
  /// \see `ThreadInfo`, `ResourceManager::DebugNuma`
  /// Default Value: `false`\n
  /// TOML config file:
  ///
  ///     [development]
  ///     debug_numa = false
  bool debug_numa = false;

  /// Display the simulation step in the terminal output with a defined
  /// frequency.\nThe value `0` shows no output, a value of `1` prints all
  /// steps, a value of `2` prints every second step, and so on.\n
  /// Default value: `0`\n
  /// TOML config file:
  ///     [development]
  ///     show_simulation_step = 0
  uint64_t show_simulation_step = 0;

  /// Use a progress bar to visualize the simulation progress. The progress bar
  /// also gives an estimate of the remaining simulation time assuming that the
  /// following simulations steps are as computationally expensive as the
  /// previous ones. It is not recommended to use the ProgressBar when you
  /// write information to std::cout in Simulate() because the ProgressBar uses
  /// '\r' in its print statements.
  /// Default value: `false`\n
  /// TOML config file:
  ///     [development]
  ///     use_progress_bar = false
  bool use_progress_bar = false;

  /// Time unit of the progress bar. Possible values: "ms", "s", "min", "h"
  /// Default value: `"s"`\n
  /// TOML config file:
  ///     [development]
  ///     progress_bar_time_unit = "s"
  std::string progress_bar_time_unit = "s";

  // ---------------------------------------------------------------------------
  // experimental group

  /// Run the simulation partially on the GPU for improved performance.
  /// Possible values: "cpu", "cuda", "opencl"
  /// Default value: `"cpu"`\n
  /// TOML config file:
  ///     [experimental]
  ///     compute_target = false
  std::string compute_target = "cpu";

  /// Compile OpenCL kernels with debugging symbols, for debugging on CPU
  /// targets with GNU gdb.
  /// Default value: `false`\n
  /// TOML config file:
  ///     [experimental]
  ///     opencl_debug = false
  bool opencl_debug = false;

  /// Set the index of the preferred GPU you wish to use. In GpuHelper we
  /// set the default to whichever GPU supports double precision
  /// Default value: `0`\n
  /// TOML config file:
  ///     [experimental]
  ///     preferred_gpu = <GPU with double precision support>
  int preferred_gpu = -1;

  /// Determines if agents' memory layout plots should be generated
  /// during load balancing.
  bool plot_memory_layout = false;

  /// Assign values from config file to variables
  void AssignFromConfig(const std::shared_ptr<cpptoml::table>&);

 private:
  friend class DiffusionTest_CopyOldData_Test;
  static std::unordered_map<ParamGroupUid, std::unique_ptr<ParamGroup>>
      registered_groups_;
  std::unordered_map<ParamGroupUid, ParamGroup*> groups_;
  BDM_CLASS_DEF_NV(Param, 1);
};

}  // namespace bdm

#endif  // CORE_PARAM_PARAM_H_

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

#include "core/environment/uniform_grid_environment.h"
#include <sstream>
#include <string>
#include "core/agent/cell.h"
#include "core/environment/environment.h"
#include "core/functor.h"
#include "gtest/gtest.h"
#include "unit/core/count_neighbor_functor.h"
#include "unit/test_util/test_util.h"

namespace bdm {

class UniformGridEnvironmentDeathTest : public ::testing::Test {};

void CellFactory(ResourceManager* rm, size_t cells_per_dim) {
  const real_t space = 20;
  rm->Reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell* cell = new Cell({k * space, j * space, i * space});
        cell->SetDiameter(30);
        rm->AddAgent(cell);
      }
    }
  }
}

TEST(UniformGridEnvironmentTest, SetupGrid) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid =
      static_cast<UniformGridEnvironment*>(simulation.GetEnvironment());

  CellFactory(rm, 4);

  grid->Update();

  std::unordered_map<AgentUid, std::vector<AgentUid>> neighbors;
  neighbors.reserve(rm->GetNumAgents());

  // Lambda that fills a vector of neighbors for each cell (excluding itself)
  rm->ForEachAgent([&](Agent* agent) {
    auto uid = agent->GetUid();
    auto fill_neighbor_list = L2F([&](Agent* neighbor, real_t) {
      auto nuid = neighbor->GetUid();
      if (uid != nuid) {
        neighbors[uid].push_back(nuid);
      }
    });

    grid->ForEachNeighbor(fill_neighbor_list, *agent, 900);
  });

  std::vector<AgentUid> expected_0 = {AgentUid(1),  AgentUid(4),  AgentUid(5),
                                      AgentUid(16), AgentUid(17), AgentUid(20)};
  std::vector<AgentUid> expected_4 = {AgentUid(0),  AgentUid(1),  AgentUid(5),
                                      AgentUid(8),  AgentUid(9),  AgentUid(16),
                                      AgentUid(20), AgentUid(21), AgentUid(24)};
  std::vector<AgentUid> expected_42 = {
      AgentUid(22), AgentUid(25), AgentUid(26), AgentUid(27), AgentUid(30),
      AgentUid(37), AgentUid(38), AgentUid(39), AgentUid(41), AgentUid(43),
      AgentUid(45), AgentUid(46), AgentUid(47), AgentUid(54), AgentUid(57),
      AgentUid(58), AgentUid(59), AgentUid(62)};
  std::vector<AgentUid> expected_63 = {AgentUid(43), AgentUid(46),
                                       AgentUid(47), AgentUid(58),
                                       AgentUid(59), AgentUid(62)};

  std::sort(neighbors[AgentUid(0)].begin(), neighbors[AgentUid(0)].end());
  std::sort(neighbors[AgentUid(4)].begin(), neighbors[AgentUid(4)].end());
  std::sort(neighbors[AgentUid(42)].begin(), neighbors[AgentUid(42)].end());
  std::sort(neighbors[AgentUid(63)].begin(), neighbors[AgentUid(63)].end());

  EXPECT_EQ(expected_0, neighbors[AgentUid(0)]);
  EXPECT_EQ(expected_4, neighbors[AgentUid(4)]);
  EXPECT_EQ(expected_42, neighbors[AgentUid(42)]);
  EXPECT_EQ(expected_63, neighbors[AgentUid(63)]);
}

TEST(UniformGridEnvironmentTest, ForEachNeighborWithoutDistance) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid =
      static_cast<UniformGridEnvironment*>(simulation.GetEnvironment());

  CellFactory(rm, 4);

  grid->Update();

  std::unordered_map<AgentUid, std::vector<AgentUid>> neighbors;
  neighbors.reserve(rm->GetNumAgents());

  // Lambda that fills a vector of neighbors for each cell (excluding itself)
  rm->ForEachAgent([&](Agent* agent) {
    auto uid = agent->GetUid();
    auto fill_neighbor_list = L2F([&](Agent* neighbor) {
      auto nuid = neighbor->GetUid();
      neighbors[uid].push_back(nuid);
    });

    grid->ForEachNeighbor(fill_neighbor_list, *agent, nullptr);
  });

  std::vector<AgentUid> expected_0 = {
      AgentUid(1),  AgentUid(2),  AgentUid(4),  AgentUid(5),  AgentUid(6),
      AgentUid(8),  AgentUid(9),  AgentUid(10), AgentUid(16), AgentUid(17),
      AgentUid(18), AgentUid(20), AgentUid(21), AgentUid(22), AgentUid(24),
      AgentUid(25), AgentUid(26), AgentUid(32), AgentUid(33), AgentUid(34),
      AgentUid(36), AgentUid(37), AgentUid(38), AgentUid(40), AgentUid(41),
      AgentUid(42)};
  // expected_4 is symmetric.
  std::vector<AgentUid> expected_4 = expected_0;
  // remove itself and add AgentUid(0);
  expected_4[2] = AgentUid(0);
  std::sort(expected_4.begin(), expected_4.end());

  // expected_42: all agents are neighbors
  std::vector<AgentUid> expected_42;
  for (int i = 0; i < 64; ++i) {
    if (i != 42) {
      expected_42.push_back(AgentUid(i));
    }
  }

  std::vector<AgentUid> expected_63 = {AgentUid(42), AgentUid(43), AgentUid(46),
                                       AgentUid(47), AgentUid(58), AgentUid(59),
                                       AgentUid(62)};

  std::sort(neighbors[AgentUid(0)].begin(), neighbors[AgentUid(0)].end());
  std::sort(neighbors[AgentUid(4)].begin(), neighbors[AgentUid(4)].end());
  std::sort(neighbors[AgentUid(42)].begin(), neighbors[AgentUid(42)].end());
  std::sort(neighbors[AgentUid(63)].begin(), neighbors[AgentUid(63)].end());

  EXPECT_EQ(expected_0, neighbors[AgentUid(0)]);
  EXPECT_EQ(expected_4, neighbors[AgentUid(4)]);
  EXPECT_EQ(expected_42, neighbors[AgentUid(42)]);
  EXPECT_EQ(expected_63, neighbors[AgentUid(63)]);
}

void RunUpdateGridTest(Simulation* simulation) {
  auto* rm = simulation->GetResourceManager();
  auto* grid =
      static_cast<UniformGridEnvironment*>(simulation->GetEnvironment());

  // Update the grid
  grid->ForcedUpdate();

  std::unordered_map<AgentUid, std::vector<AgentUid>> neighbors;
  neighbors.reserve(rm->GetNumAgents());

  // Lambda that fills a vector of neighbors for each cell (excluding itself)
  rm->ForEachAgent([&](Agent* agent) {
    auto uid = agent->GetUid();
    auto fill_neighbor_list = L2F([&](Agent* neighbor, real_t) {
      auto nuid = neighbor->GetUid();
      if (uid != nuid) {
        neighbors[uid].push_back(nuid);
      }
    });

    grid->ForEachNeighbor(fill_neighbor_list, *agent, 900);
  });

  std::vector<AgentUid> expected_0 = {AgentUid(4), AgentUid(5), AgentUid(16),
                                      AgentUid(17), AgentUid(20)};
  std::vector<AgentUid> expected_5 = {AgentUid(0),  AgentUid(2),  AgentUid(4),
                                      AgentUid(6),  AgentUid(8),  AgentUid(9),
                                      AgentUid(10), AgentUid(17), AgentUid(20),
                                      AgentUid(21), AgentUid(22), AgentUid(25)};
  std::vector<AgentUid> expected_41 = {
      AgentUid(21), AgentUid(24), AgentUid(25), AgentUid(26), AgentUid(29),
      AgentUid(36), AgentUid(37), AgentUid(38), AgentUid(40), AgentUid(44),
      AgentUid(45), AgentUid(46), AgentUid(53), AgentUid(56), AgentUid(57),
      AgentUid(58), AgentUid(61)};
  std::vector<AgentUid> expected_61 = {
      AgentUid(41), AgentUid(44), AgentUid(45), AgentUid(46), AgentUid(56),
      AgentUid(57), AgentUid(58), AgentUid(60), AgentUid(62)};

  std::sort(neighbors[AgentUid(0)].begin(), neighbors[AgentUid(0)].end());
  std::sort(neighbors[AgentUid(5)].begin(), neighbors[AgentUid(5)].end());
  std::sort(neighbors[AgentUid(41)].begin(), neighbors[AgentUid(41)].end());
  std::sort(neighbors[AgentUid(61)].begin(), neighbors[AgentUid(61)].end());

  EXPECT_EQ(expected_0, neighbors[AgentUid(0)]);
  EXPECT_EQ(expected_5, neighbors[AgentUid(5)]);
  EXPECT_EQ(expected_41, neighbors[AgentUid(41)]);
  EXPECT_EQ(expected_61, neighbors[AgentUid(61)]);
}

// TODO(lukas) Add tests for UniformGridEnvironment::ForEachNeighbor

TEST(UniformGridEnvironmentTest, UpdateGrid) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();

  CellFactory(rm, 4);

  env->ForcedUpdate();

  // Remove cells 1 and 42
  rm->RemoveAgent(AgentUid(1));
  rm->RemoveAgent(AgentUid(42));

  EXPECT_EQ(62u, rm->GetNumAgents());

  RunUpdateGridTest(&simulation);
}

TEST(UniformGridEnvironmentTest, NoRaceConditionDuringUpdate) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();

  CellFactory(rm, 4);

  // make sure that there are multiple cells per box
  rm->GetAgent(AgentUid(0))->SetDiameter(60);

  env->ForcedUpdate();

  // Remove cells 1 and 42
  rm->RemoveAgent(AgentUid(1));
  rm->RemoveAgent(AgentUid(42));

  // run 100 times to increase possibility of race condition due to different
  // scheduling of threads
  for (uint16_t i = 0; i < 100; i++) {
    RunUpdateGridTest(&simulation);
  }
}

TEST(UniformGridEnvironmentTest, GetBoxIndex) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid =
      static_cast<UniformGridEnvironment*>(simulation.GetEnvironment());

  CellFactory(rm, 3);

  grid->Update();

  Real3 position_0 = {{0, 0, 0}};
  Real3 position_1 = {{1e-15, 1e-15, 1e-15}};
  Real3 position_2 = {{-1e-15, 1e-15, 1e-15}};

  size_t expected_idx_0 = 21;
  size_t expected_idx_1 = 21;
  size_t expected_idx_2 = 20;

  size_t idx_0 = grid->GetBoxIndex(position_0);
  size_t idx_1 = grid->GetBoxIndex(position_1);
  size_t idx_2 = grid->GetBoxIndex(position_2);

  EXPECT_EQ(expected_idx_0, idx_0);
  EXPECT_EQ(expected_idx_1, idx_1);
  EXPECT_EQ(expected_idx_2, idx_2);
}

TEST(UniformGridEnvironmentTest, GridDimensions) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();

  CellFactory(rm, 3);

  env->ForcedUpdate();

  std::array<int32_t, 6> expected_dim_0 = {{-30, 90, -30, 90, -30, 90}};
  auto dim_0 = env->GetDimensions();

  EXPECT_EQ(expected_dim_0, dim_0);

  rm->GetAgent(AgentUid(0))->SetPosition({{100, 0, 0}});
  env->ForcedUpdate();
  std::array<int32_t, 6> expected_dim_1 = {{-30, 150, -30, 90, -30, 90}};
  auto dim_1 = env->GetDimensions();

  EXPECT_EQ(expected_dim_1, dim_1);
}

TEST(UniformGridEnvironmentTest, GetBoxCoordinates) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid =
      static_cast<UniformGridEnvironment*>(simulation.GetEnvironment());

  CellFactory(rm, 3);

  // expecting a 4 * 4 * 4 grid
  grid->Update();

  EXPECT_ARR_EQ({3, 0, 0}, grid->GetBoxCoordinates(3));
  EXPECT_ARR_EQ({1, 2, 0}, grid->GetBoxCoordinates(9));
  EXPECT_ARR_EQ({1, 2, 3}, grid->GetBoxCoordinates(57));
}

TEST(UniformGridEnvironmentTest, NonEmptyBoundedTestThresholdDimensions) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 1;
    param->max_bound = 99;
  };

  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();

  rm->AddAgent(new Cell(10));

  env->ForcedUpdate();

  auto max_dimensions = env->GetDimensionThresholds();
  EXPECT_EQ(1, max_dimensions[0]);
  EXPECT_EQ(99, max_dimensions[1]);
}

struct TestFunctor : public Functor<void, Agent*, real_t> {
  void operator()(Agent* neighbor, real_t squared_distance) override {}
};

TEST(UniformGridEnvironmentTest, CustomBoxLength) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* env =
      dynamic_cast<UniformGridEnvironment*>(simulation.GetEnvironment());

  auto cell = new Cell(10);
  rm->AddAgent(cell);

  env->ForcedUpdate();
  EXPECT_EQ(10, env->GetBoxLength());

  env->SetBoxLength(15);
  EXPECT_EQ(15, env->GetBoxLength());

  env->ForcedUpdate();
  EXPECT_EQ(15, env->GetBoxLength());

  rm->AddAgent(new Cell(20));
  env->ForcedUpdate();
  EXPECT_EQ(15, env->GetBoxLength());
}

TEST(UniformGridEnvironmentDeathTest, CustomBoxLength) {
  ASSERT_DEATH(
      {
        Simulation simulation(TEST_NAME);
        auto* rm = simulation.GetResourceManager();
        auto* ctxt = simulation.GetExecutionContext();
        auto* env =
            dynamic_cast<UniformGridEnvironment*>(simulation.GetEnvironment());

        auto cell = new Cell(10);
        rm->AddAgent(cell);

        env->ForcedUpdate();
        EXPECT_EQ(10, env->GetBoxLength());

        env->SetBoxLength(15);
        EXPECT_EQ(15, env->GetBoxLength());

        env->ForcedUpdate();
        EXPECT_EQ(15, env->GetBoxLength());

        rm->AddAgent(new Cell(20));
        env->ForcedUpdate();
        EXPECT_EQ(15, env->GetBoxLength());

        auto tf = TestFunctor();
        // This call should fail because the default search radius is set to the
        // largest object (20), which is larger than the custom box length (15)
        ctxt->ForEachNeighbor(tf, *cell, env->GetLargestAgentSizeSquared());
      },
      "");
}

struct ZOrderCallback : Functor<void, const AgentHandle&> {
  std::vector<std::set<AgentUid>> zorder;
  uint64_t box_cnt = 0;
  uint64_t cnt = 0;
  ResourceManager* rm;
  AgentUid ref_uid;

  ZOrderCallback(ResourceManager* rm, AgentUid ref_uid)
      : rm(rm), ref_uid(ref_uid) {
    zorder.resize(8);
  }

  void operator()(const AgentHandle& ah) {
    if (cnt == 8 || cnt == 12 || cnt == 16 || cnt == 18 || cnt == 22 ||
        cnt == 24 || cnt == 26) {
      box_cnt++;
    }
    auto* agent = rm->GetAgent(ah);
    zorder[box_cnt].insert(agent->GetUid() - ref_uid);
    cnt++;
  }
};

// Tests if ForEachNeighbor of the respective environment finds the correct
// number of neighbors. The same test is implemented for kdtree and octree
// environments.
TEST(UniformGridEnvironmentTest, FindAllNeighbors) {
  // Create simulation with uniform-grid environment
  auto set_param = [](auto* param) {
    param->environment = "uniform_grid";
    param->unschedule_default_operations = {"load balancing",
                                            "mechanical forces"};
  };
  Simulation simulation(TEST_NAME, set_param);

  // Please consult the definition of the fuction for more information.
  TestNeighborSearch(simulation);
}

// Tests if ForEachNeighbor of the respective environment finds the correct
// number of neighbors. The same test is implemented for kdtree and octree
// environments. Important: In contrast to the previous test, load balancing
// must be active here.
TEST(UniformGridEnvironmentTest, FindAllNeighborsLoadBalanced) {
  // Create simulation with uniform-grid environment
  auto set_param = [](auto* param) {
    param->environment = "uniform_grid";
    param->unschedule_default_operations = {"mechanical forces"};
  };
  Simulation simulation(TEST_NAME, set_param);

  // Check if load balancing is active.
  std::stringstream buffer;
  simulation.GetScheduler()->PrintInfo(buffer);
  EXPECT_TRUE(buffer.str().find("load balancing") != std::string::npos);

  // Please consult the definition of the fuction for more information.
  TestNeighborSearch(simulation);
}

}  // namespace bdm

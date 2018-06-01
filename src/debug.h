// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef DEBUG_H_
#define DEBUG_H_

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>

inline void Print(const std::array<double, 3>& a, int precision = 10) {
  std::cout << std::setprecision(precision) << a[0] << ", " << a[1] << ", "
            << a[2] << std::endl;
}

#endif  // DEBUG_H_

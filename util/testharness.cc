//  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "util/testharness.h"
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "port/stack_trace.h"

namespace rocksdb {
namespace test {

namespace {
struct Test {
  const char* base;
  const char* name;
  void (*func)();
};
std::vector<Test>* tests;
}

bool RegisterTest(const char* base, const char* name, void (*func)()) {
  if (tests == nullptr) {
    tests = new std::vector<Test>;
  }
  Test t;
  t.base = base;
  t.name = name;
  t.func = func;
  tests->push_back(t);
  return true;
}

int RunAllTests() {
  port::InstallStackTraceHandler();

  const char* one_matcher = getenv("ROCKSDB_TESTS");
  const char* from_matcher = getenv("ROCKSDB_TESTS_FROM");

  int num = 0;
  bool tests_on = (one_matcher == nullptr && from_matcher == nullptr);
  if (tests != nullptr) {
    for (unsigned int i = 0; i < tests->size(); i++) {
      const Test& t = (*tests)[i];
      if (tests_on == false) {
        if (one_matcher != nullptr || from_matcher != nullptr) {
          std::string name = t.base;
          name.push_back('.');
          name.append(t.name);
          if (from_matcher != nullptr &&
              strstr(name.c_str(), from_matcher) != nullptr) {
            tests_on = true;
          }
          if (!tests_on) {
            if (one_matcher == nullptr ||
                strstr(name.c_str(), one_matcher) == nullptr) {
              continue;
            }
          }
        }
      }
      fprintf(stderr, "==== Test %s.%s\n", t.base, t.name);
      (*t.func)();
      ++num;
    }
  }
  fprintf(stderr, "==== PASSED %d tests\n", num);
  return 0;
}

std::string TmpDir(Env* env) {
  std::string dir;
  Status s = env->GetTestDirectory(&dir);
  EXPECT_TRUE(s.ok()) << s.ToString();
  return dir;
}

int RandomSeed() {
  const char* env = getenv("TEST_RANDOM_SEED");
  int result = (env != nullptr ? atoi(env) : 301);
  if (result <= 0) {
    result = 301;
  }
  return result;
}

}  // namespace test
}  // namespace rocksdb

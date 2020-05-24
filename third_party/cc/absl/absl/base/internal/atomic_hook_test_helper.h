// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABSL_BASE_ATOMIC_HOOK_TEST_HELPER_H_
#define ABSL_BASE_ATOMIC_HOOK_TEST_HELPER_H_

#include "third_party/cc/absl/absl/base/internal/atomic_hook.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace atomic_hook_internal {

using VoidF = void (*)();
extern absl::base_internal::AtomicHook<VoidF> func;
extern int default_func_calls;
void DefaultFunc();
void RegisterFunc(VoidF func);

}  // namespace atomic_hook_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_BASE_ATOMIC_HOOK_TEST_HELPER_H_
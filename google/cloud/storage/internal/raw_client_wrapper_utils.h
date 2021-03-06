// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_RAW_CLIENT_WRAPPER_UTILS_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_RAW_CLIENT_WRAPPER_UTILS_H_

#include "google/cloud/storage/internal/raw_client.h"
#include <type_traits>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace internal {
/**
 * Define types to wrap RawClient function calls.
 *
 * We have a couple of classes that basically wrap every function in RawClient
 * with some additional behavior (LoggingClient logs every call, and RetryClient
 * retries every call).  Instead of hand-coding every wrapped function we use
 * a helper to wrap it, and in turn those helpers use the meta-functions defined
 * here.
 */
namespace raw_client_wrapper_utils {
/**
 * The signature for a RawClient member function.
 */
template <typename Request, typename Response>
using DesiredSignature = std::pair<google::cloud::storage::Status, Response> (
    google::cloud::storage::internal::RawClient::*)(Request const&);

/**
 * Determine if @p T is a pointer to member function with the expected
 * signature for a `RawClient` member function.
 *
 * This is the generic case, where the type does not match the expected
 * signature.  The class derives from `std::false_type`, so
 * `CheckSignature<T>::%value` is `false`.
 *
 * @tparam T the type to check against the expected signature.
 */
template <typename T>
struct CheckSignature : public std::false_type {
  /// Must define ReturnType because it is used in std::enable_if<>.
  using ReturnType = void;
};

/**
 * Determine if a type is a pointer to member function with the expected
 * signature.
 *
 * This is the case where the type actually matches the expected signature.
 * This class derives from `std::true_type`, so `CheckSignature<T>::%value` is
 * `true`.  The class also extracts the request and response types used in the
 * implementation of `CallWithRetry()`.
 *
 * @tparam Request the RPC request type.
 * @tparam Response the RPC response type.
 */
template <typename Request, typename Response>
struct CheckSignature<DesiredSignature<Request, Response>>
    : public std::true_type {
  using ResponseType = Response;
  using RequestType = Request;
  using MemberFunctionType = DesiredSignature<Request, Response>;
  using ReturnType = std::pair<google::cloud::storage::Status, ResponseType>;
};
}  // namespace raw_client_wrapper_utils
}  // namespace internal
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_STORAGE_INTERNAL_RAW_CLIENT_WRAPPER_UTILS_H_

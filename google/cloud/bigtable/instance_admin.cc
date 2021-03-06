// Copyright 2018 Google Inc.
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

#include "google/cloud/bigtable/instance_admin.h"
#include "google/cloud/bigtable/internal/grpc_error_delegate.h"
#include "google/cloud/bigtable/internal/unary_client_utils.h"
#include "google/cloud/internal/throw_delegate.h"
#include <google/longrunning/operations.grpc.pb.h>
#include <google/protobuf/text_format.h>
#include <type_traits>

namespace btadmin = ::google::bigtable::admin::v2;

namespace google {
namespace cloud {
namespace bigtable {
inline namespace BIGTABLE_CLIENT_NS {
static_assert(std::is_copy_assignable<bigtable::InstanceAdmin>::value,
              "bigtable::InstanceAdmin must be CopyAssignable");

std::vector<btadmin::Instance> InstanceAdmin::ListInstances() {
  grpc::Status status;
  auto result = impl_.ListInstances(status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

std::future<google::bigtable::admin::v2::Instance>
InstanceAdmin::CreateInstance(InstanceConfig instance_config) {
  return std::async(std::launch::async, &InstanceAdmin::CreateInstanceImpl,
                    this, std::move(instance_config));
}

std::future<google::bigtable::admin::v2::Cluster> InstanceAdmin::CreateCluster(
    ClusterConfig cluster_config, bigtable::InstanceId const& instance_id,
    bigtable::ClusterId const& cluster_id) {
  return std::async(std::launch::async, &InstanceAdmin::CreateClusterImpl, this,
                    std::move(cluster_config), instance_id, cluster_id);
}

google::bigtable::admin::v2::Instance InstanceAdmin::CreateInstanceImpl(
    InstanceConfig instance_config) {
  // Copy the policies in effect for the operation.
  auto rpc_policy = impl_.rpc_retry_policy_->clone();
  auto backoff_policy = impl_.rpc_backoff_policy_->clone();

  // Build the RPC request, try to minimize copying.
  auto request = instance_config.as_proto_move();
  request.set_parent(project_name());
  for (auto& kv : *request.mutable_clusters()) {
    kv.second.set_location(project_name() + "/locations/" +
                           kv.second.location());
  }

  using ClientUtils =
      bigtable::internal::noex::UnaryClientUtils<InstanceAdminClient>;

  grpc::Status status;
  auto operation = ClientUtils::MakeCall(
      *impl_.client_, *rpc_policy, *backoff_policy,
      impl_.metadata_update_policy_, &InstanceAdminClient::CreateInstance,
      request, "InstanceAdmin::CreateInstance", status, false);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status,
                                      "unrecoverable error in MakeCall()");
  }

  auto result = impl_.PollLongRunningOperation<btadmin::Instance>(
      operation, "InstanceAdmin::CreateInstance", status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(
        status, "while polling operation in InstanceAdmin::CreateInstance");
  }
  return result;
}

std::future<google::bigtable::admin::v2::Instance>
InstanceAdmin::UpdateInstance(InstanceUpdateConfig instance_update_config) {
  return std::async(std::launch::async, &InstanceAdmin::UpdateInstanceImpl,
                    this, std::move(instance_update_config));
}

google::bigtable::admin::v2::Instance InstanceAdmin::UpdateInstanceImpl(
    InstanceUpdateConfig instance_update_config) {
  // Copy the policies in effect for the operation.
  auto rpc_policy = impl_.rpc_retry_policy_->clone();
  auto backoff_policy = impl_.rpc_backoff_policy_->clone();

  MetadataUpdatePolicy metadata_update_policy(instance_update_config.GetName(),
                                              MetadataParamTypes::NAME);

  auto request = instance_update_config.as_proto_move();

  using ClientUtils =
      bigtable::internal::noex::UnaryClientUtils<InstanceAdminClient>;

  grpc::Status status;
  auto operation = ClientUtils::MakeCall(
      *impl_.client_, *rpc_policy, *backoff_policy,
      impl_.metadata_update_policy_, &InstanceAdminClient::UpdateInstance,
      request, "InstanceAdmin::UpdateInstance", status, false);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status,
                                      "unrecoverable error in MakeCall()");
  }

  auto result = impl_.PollLongRunningOperation<btadmin::Instance>(
      operation, "InstanceAdmin::UpdateInstance", status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(
        status, "while polling operation in InstanceAdmin::UpdateInstance");
  }
  return result;
}

btadmin::Instance InstanceAdmin::GetInstance(std::string const& instance_id) {
  grpc::Status status;
  auto result = impl_.GetInstance(instance_id, status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

void InstanceAdmin::DeleteInstance(std::string const& instance_id) {
  grpc::Status status;
  impl_.DeleteInstance(instance_id, status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status, status.error_message());
  }
}

btadmin::Cluster InstanceAdmin::GetCluster(
    bigtable::InstanceId const& instance_id,
    bigtable::ClusterId const& cluster_id) {
  grpc::Status status;
  auto result = impl_.GetCluster(instance_id, cluster_id, status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

std::vector<btadmin::Cluster> InstanceAdmin::ListClusters() {
  return ListClusters("-");
}

std::vector<btadmin::Cluster> InstanceAdmin::ListClusters(
    std::string const& instance_id) {
  grpc::Status status;
  auto result = impl_.ListClusters(instance_id, status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

std::future<google::bigtable::admin::v2::Cluster> InstanceAdmin::UpdateCluster(
    ClusterConfig cluster_config) {
  return std::async(std::launch::async, &InstanceAdmin::UpdateClusterImpl, this,
                    std::move(cluster_config));
}

google::bigtable::admin::v2::Cluster InstanceAdmin::UpdateClusterImpl(
    ClusterConfig cluster_config) {
  // Copy the policies in effect for the operation.
  auto rpc_policy = impl_.rpc_retry_policy_->clone();
  auto backoff_policy = impl_.rpc_backoff_policy_->clone();

  MetadataUpdatePolicy metadata_update_policy(cluster_config.GetName(),
                                              MetadataParamTypes::NAME);

  auto request = cluster_config.as_proto_move();

  using ClientUtils =
      bigtable::internal::noex::UnaryClientUtils<InstanceAdminClient>;

  grpc::Status status;
  auto operation = ClientUtils::MakeCall(
      *impl_.client_, *rpc_policy, *backoff_policy,
      impl_.metadata_update_policy_, &InstanceAdminClient::UpdateCluster,
      request, "InstanceAdmin::UpdateCluster", status, false);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status,
                                      "unrecoverable error in MakeCall()");
  }

  auto result = impl_.PollLongRunningOperation<btadmin::Cluster>(
      operation, "InstanceAdmin::UpdateCluster", status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(
        status, "while polling operation in InstanceAdmin::UpdateCluster");
  }
  return result;
}
void InstanceAdmin::DeleteCluster(bigtable::InstanceId const& instance_id,
                                  bigtable::ClusterId const& cluster_id) {
  grpc::Status status;
  impl_.DeleteCluster(instance_id, cluster_id, status);
  if (not status.ok()) {
    internal::RaiseRpcError(status, status.error_message());
  }
}

btadmin::AppProfile InstanceAdmin::CreateAppProfile(
    bigtable::InstanceId const& instance_id, AppProfileConfig config) {
  grpc::Status status;
  auto result = impl_.CreateAppProfile(instance_id, std::move(config), status);
  if (not status.ok()) {
    internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

btadmin::AppProfile InstanceAdmin::GetAppProfile(
    bigtable::InstanceId const& instance_id,
    bigtable::AppProfileId const& profile_id) {
  grpc::Status status;
  auto result = impl_.GetAppProfile(instance_id, profile_id, status);
  if (not status.ok()) {
    internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

std::future<btadmin::AppProfile> InstanceAdmin::UpdateAppProfile(
    bigtable::InstanceId instance_id, bigtable::AppProfileId profile_id,
    AppProfileUpdateConfig config) {
  return std::async(std::launch::async, &InstanceAdmin::UpdateAppProfileImpl,
                    this, std::move(instance_id), std::move(profile_id),
                    std::move(config));
}

std::vector<btadmin::AppProfile> InstanceAdmin::ListAppProfiles(
    std::string const& instance_id) {
  grpc::Status status;
  auto result = impl_.ListAppProfiles(instance_id, status);
  if (not status.ok()) {
    internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

void InstanceAdmin::DeleteAppProfile(bigtable::InstanceId const& instance_id,
                                     bigtable::AppProfileId const& profile_id,
                                     bool ignore_warnings) {
  grpc::Status status;
  impl_.DeleteAppProfile(instance_id, profile_id, ignore_warnings, status);
  if (not status.ok()) {
    internal::RaiseRpcError(status, status.error_message());
  }
}

google::bigtable::admin::v2::Cluster InstanceAdmin::CreateClusterImpl(
    ClusterConfig const& cluster_config,
    bigtable::InstanceId const& instance_id,
    bigtable::ClusterId const& cluster_id) {
  // Copy the policies in effect for the operation.
  auto rpc_policy = impl_.rpc_retry_policy_->clone();
  auto backoff_policy = impl_.rpc_backoff_policy_->clone();

  // Build the RPC request, try to minimize copying.
  auto cluster = cluster_config.as_proto_move();
  cluster.set_location(project_name() + "/locations/" + cluster.location());

  btadmin::CreateClusterRequest request;
  request.mutable_cluster()->Swap(&cluster);
  request.set_parent(project_name() + "/instances/" + instance_id.get());
  request.set_cluster_id(cluster_id.get());

  using ClientUtils =
      bigtable::internal::noex::UnaryClientUtils<InstanceAdminClient>;

  grpc::Status status;
  auto operation = ClientUtils::MakeCall(
      *impl_.client_, *rpc_policy, *backoff_policy,
      impl_.metadata_update_policy_, &InstanceAdminClient::CreateCluster,
      request, "InstanceAdmin::CreateCluster", status, false);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status,
                                      "unrecoverable error in MakeCall()");
  }

  auto result = impl_.PollLongRunningOperation<btadmin::Cluster>(
      operation, "InstanceAdmin::CreateCluster", status);
  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(
        status, "while polling operation in InstanceAdmin::CreateCluster");
  }
  return result;
}

btadmin::AppProfile InstanceAdmin::UpdateAppProfileImpl(
    bigtable::InstanceId instance_id, bigtable::AppProfileId profile_id,
    AppProfileUpdateConfig config) {
  grpc::Status status;
  auto operation = impl_.UpdateAppProfile(
      std::move(instance_id), std::move(profile_id), std::move(config), status);
  if (not status.ok()) {
    internal::RaiseRpcError(status, status.error_message());
  }

  auto result = impl_.PollLongRunningOperation<btadmin::AppProfile>(
      operation, "InstanceAdmin::UpdateAppProfileImpl", status);
  if (not status.ok()) {
    internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

google::cloud::IamPolicy InstanceAdmin::GetIamPolicy(
    std::string const& instance_id) {
  grpc::Status status;
  auto result = impl_.GetIamPolicy(instance_id, status);

  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

google::cloud::IamPolicy InstanceAdmin::SetIamPolicy(
    std::string const& instance_id,
    google::cloud::IamBindings const& iam_bindings, std::string const& etag) {
  grpc::Status status;
  auto result = impl_.SetIamPolicy(instance_id, iam_bindings, etag, status);

  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status, status.error_message());
  }
  return result;
}

std::vector<std::string> InstanceAdmin::TestIamPermissions(
    std::string const& instance_id,
    std::vector<std::string> const& permissions) {
  grpc::Status status;
  auto result = impl_.TestIamPermissions(instance_id, permissions, status);

  if (not status.ok()) {
    bigtable::internal::RaiseRpcError(status, status.error_message());
  }

  return result;
}

}  // namespace BIGTABLE_CLIENT_NS
}  // namespace bigtable
}  // namespace cloud
}  // namespace google

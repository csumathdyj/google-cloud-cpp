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

#include "google/cloud/storage/client.h"
#include "google/cloud/storage/retry_policy.h"
#include "google/cloud/storage/testing/canonical_errors.h"
#include "google/cloud/storage/testing/mock_client.h"
#include "google/cloud/storage/testing/retry_tests.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace {
using ::testing::_;
using ::testing::HasSubstr;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::canonical_errors::TransientError;

/**
 * Test the functions in Storage::Client related to 'Buckets: *'.
 *
 * In general, this file should include for the APIs listed in:
 *
 * https://cloud.google.com/storage/docs/json_api/v1/buckets
 */
class BucketTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mock = std::make_shared<testing::MockClient>();
    EXPECT_CALL(*mock, client_options())
        .WillRepeatedly(ReturnRef(client_options));
    client.reset(new Client{std::shared_ptr<internal::RawClient>(mock)});
  }
  void TearDown() override {
    client.reset();
    mock.reset();
  }

  std::shared_ptr<testing::MockClient> mock;
  std::unique_ptr<Client> client;
  ClientOptions client_options = ClientOptions(CreateInsecureCredentials());
};

TEST_F(BucketTest, CreateBucket) {
  std::string text = R"""({
      "kind": "storage#bucket",
      "id": "test-bucket-name",
      "selfLink": "https://www.googleapis.com/storage/v1/b/test-bucket-name",
      "projectNumber": "123456789",
      "name": "test-bucket-name",
      "timeCreated": "2018-05-19T19:31:14Z",
      "updated": "2018-05-19T19:31:24Z",
      "metageneration": 7,
      "location": "US",
      "storageClass": "STANDARD",
      "etag": "XYZ="
})""";
  auto expected = BucketMetadata::ParseFromString(text);

  ClientOptions mock_options(CreateInsecureCredentials());
  mock_options.set_project_id("test-project-name");

  EXPECT_CALL(*mock, client_options()).WillRepeatedly(ReturnRef(mock_options));
  EXPECT_CALL(*mock, CreateBucket(_))
      .WillOnce(Return(std::make_pair(TransientError(), BucketMetadata{})))
      .WillOnce(Invoke([&expected](internal::CreateBucketRequest const& r) {
        EXPECT_EQ("test-bucket-name", r.metadata().name());
        EXPECT_EQ("US", r.metadata().location());
        EXPECT_EQ("STANDARD", r.metadata().storage_class());
        EXPECT_EQ("test-project-name", r.project_id());
        return std::make_pair(Status(), expected);
      }));
  Client client{std::shared_ptr<internal::RawClient>(mock),
                LimitedErrorCountRetryPolicy(2)};

  auto actual = client.CreateBucket(
      "test-bucket-name",
      BucketMetadata().set_location("US").set_storage_class("STANDARD"));
  EXPECT_EQ(expected, actual);
}

TEST_F(BucketTest, CreateBucketTooManyFailures) {
  testing::TooManyFailuresTest<BucketMetadata>(
      mock, EXPECT_CALL(*mock, CreateBucket(_)),
      [](Client& client) {
        client.CreateBucketForProject("test-bucket-name", "test-project-name",
                                      BucketMetadata());
      },
      "CreateBucket");
}

TEST_F(BucketTest, CreateBucketPermanentFailure) {
  testing::PermanentFailureTest<BucketMetadata>(
      *client, EXPECT_CALL(*mock, CreateBucket(_)),
      [](Client& client) {
        client.CreateBucketForProject("test-bucket-name", "test-project-name",
                                      BucketMetadata());
      },
      "CreateBucket");
}

TEST_F(BucketTest, GetBucketMetadata) {
  std::string text = R"""({
      "kind": "storage#bucket",
      "id": "foo-bar-baz",
      "selfLink": "https://www.googleapis.com/storage/v1/b/foo-bar-baz",
      "projectNumber": "123456789",
      "name": "foo-bar-baz",
      "timeCreated": "2018-05-19T19:31:14Z",
      "updated": "2018-05-19T19:31:24Z",
      "metageneration": "4",
      "location": "US",
      "storageClass": "STANDARD",
      "etag": "XYZ="
})""";
  auto expected = BucketMetadata::ParseFromString(text);

  EXPECT_CALL(*mock, GetBucketMetadata(_))
      .WillOnce(Return(std::make_pair(TransientError(), BucketMetadata{})))
      .WillOnce(
          Invoke([&expected](internal::GetBucketMetadataRequest const& r) {
            EXPECT_EQ("foo-bar-baz", r.bucket_name());
            return std::make_pair(Status(), expected);
          }));
  Client client{std::shared_ptr<internal::RawClient>(mock),
                LimitedErrorCountRetryPolicy(2)};

  auto actual = client.GetBucketMetadata("foo-bar-baz");
  EXPECT_EQ(expected, actual);
}

TEST_F(BucketTest, GetMetadataTooManyFailures) {
  testing::TooManyFailuresTest<BucketMetadata>(
      mock, EXPECT_CALL(*mock, GetBucketMetadata(_)),
      [](Client& client) { client.GetBucketMetadata("test-bucket-name"); },
      "GetBucketMetadata");
}

TEST_F(BucketTest, GetMetadataPermanentFailure) {
  testing::PermanentFailureTest<BucketMetadata>(
      *client, EXPECT_CALL(*mock, GetBucketMetadata(_)),
      [](Client& client) { client.GetBucketMetadata("test-bucket-name"); },
      "GetBucketMetadata");
}

TEST_F(BucketTest, DeleteBucket) {
  EXPECT_CALL(*mock, DeleteBucket(_))
      .WillOnce(
          Return(std::make_pair(TransientError(), internal::EmptyResponse{})))
      .WillOnce(Invoke([](internal::DeleteBucketRequest const& r) {
        EXPECT_EQ("foo-bar-baz", r.bucket_name());
        return std::make_pair(Status(), internal::EmptyResponse{});
      }));
  Client client{std::shared_ptr<internal::RawClient>(mock),
                LimitedErrorCountRetryPolicy(2)};

  client.DeleteBucket("foo-bar-baz");
  SUCCEED();
}

TEST_F(BucketTest, DeleteBucketTooManyFailures) {
  testing::TooManyFailuresTest<internal::EmptyResponse>(
      mock, EXPECT_CALL(*mock, DeleteBucket(_)),
      [](Client& client) { client.DeleteBucket("test-bucket-name"); },
      "DeleteBucket");
}

TEST_F(BucketTest, DeleteBucketPermanentFailure) {
  testing::PermanentFailureTest<internal::EmptyResponse>(
      *client, EXPECT_CALL(*mock, DeleteBucket(_)),
      [](Client& client) { client.DeleteBucket("test-bucket-name"); },
      "DeleteBucket");
}

TEST_F(BucketTest, UpdateBucket) {
  std::string text = R"""({
      "kind": "storage#bucket",
      "id": "test-bucket-name",
      "selfLink": "https://www.googleapis.com/storage/v1/b/test-bucket-name",
      "projectNumber": "123456789",
      "name": "test-bucket-name",
      "timeCreated": "2018-05-19T19:31:14Z",
      "updated": "2018-05-19T19:31:24Z",
      "metageneration": 7,
      "location": "US",
      "storageClass": "STANDARD",
      "etag": "XYZ="
})""";
  auto expected = BucketMetadata::ParseFromString(text);

  EXPECT_CALL(*mock, UpdateBucket(_))
      .WillOnce(Return(std::make_pair(TransientError(), BucketMetadata{})))
      .WillOnce(Invoke([&expected](internal::UpdateBucketRequest const& r) {
        EXPECT_EQ("test-bucket-name", r.metadata().name());
        EXPECT_EQ("US", r.metadata().location());
        EXPECT_EQ("STANDARD", r.metadata().storage_class());
        return std::make_pair(Status(), expected);
      }));
  Client client{std::shared_ptr<internal::RawClient>(mock),
                LimitedErrorCountRetryPolicy(2)};

  auto actual = client.UpdateBucket(
      "test-bucket-name",
      BucketMetadata().set_location("US").set_storage_class("STANDARD"));
  EXPECT_EQ(expected, actual);
}

TEST_F(BucketTest, UpdateBucketTooManyFailures) {
  testing::TooManyFailuresTest<BucketMetadata>(
      mock, EXPECT_CALL(*mock, UpdateBucket(_)),
      [](Client& client) {
        client.UpdateBucket("test-bucket-name", BucketMetadata());
      },
      "UpdateBucket");
}

TEST_F(BucketTest, UpdateBucketPermanentFailure) {
  testing::PermanentFailureTest<BucketMetadata>(
      *client, EXPECT_CALL(*mock, UpdateBucket(_)),
      [](Client& client) {
        client.UpdateBucket("test-bucket-name", BucketMetadata());
      },
      "UpdateBucket");
}

TEST_F(BucketTest, PatchBucket) {
  std::string text = R"""({
      "kind": "storage#bucket",
      "id": "test-bucket-name",
      "selfLink": "https://www.googleapis.com/storage/v1/b/test-bucket-name",
      "projectNumber": "123456789",
      "name": "test-bucket-name",
      "timeCreated": "2018-05-19T19:31:14Z",
      "updated": "2018-05-19T19:31:24Z",
      "metageneration": 7,
      "location": "US",
      "storageClass": "STANDARD",
      "etag": "XYZ="
})""";
  auto expected = BucketMetadata::ParseFromString(text);

  EXPECT_CALL(*mock, PatchBucket(_))
      .WillOnce(Return(std::make_pair(TransientError(), BucketMetadata{})))
      .WillOnce(Invoke([&expected](internal::PatchBucketRequest const& r) {
        EXPECT_EQ("test-bucket-name", r.bucket());
        EXPECT_THAT(r.payload(), HasSubstr("STANDARD"));
        return std::make_pair(Status(), expected);
      }));
  Client client{std::shared_ptr<internal::RawClient>(mock),
                LimitedErrorCountRetryPolicy(2)};

  auto actual = client.PatchBucket(
      "test-bucket-name",
      BucketMetadataPatchBuilder().SetStorageClass("STANDARD"));
  EXPECT_EQ(expected, actual);
}

TEST_F(BucketTest, PatchBucketTooManyFailures) {
  testing::TooManyFailuresTest<BucketMetadata>(
      mock, EXPECT_CALL(*mock, PatchBucket(_)),
      [](Client& client) {
        client.PatchBucket("test-bucket-name", BucketMetadataPatchBuilder());
      },
      "PatchBucket");
}

TEST_F(BucketTest, PatchBucketPermanentFailure) {
  testing::PermanentFailureTest<BucketMetadata>(
      *client, EXPECT_CALL(*mock, PatchBucket(_)),
      [](Client& client) {
        client.PatchBucket("test-bucket-name", BucketMetadataPatchBuilder());
      },
      "PatchBucket");
}

}  // namespace
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google

#include "gtest/gtest.h"
#include "c2017/queue_manager/queue_manager.h"

TEST(QueueManager, Initalizes) { c2017::QueueManager::GetInstance(); }

TEST(QueueManager, QueueWorks) {
  muan::proto::StackProto<PdpStatus, 1024> p;
  c2017::QueueManager::GetInstance().pdp_status_queue().WriteMessage(p);

  auto pdp_status_reader = c2017::QueueManager::GetInstance().pdp_status_queue().MakeReader();

  // Test that it reads one message (the message we sent above), then that it
  // doesn't have any new messages (because we just read the only message).
  ASSERT_TRUE(pdp_status_reader.ReadMessage());
  ASSERT_FALSE(pdp_status_reader.ReadMessage());
}

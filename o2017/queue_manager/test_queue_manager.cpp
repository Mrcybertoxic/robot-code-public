#include "gtest/gtest.h"
#include "o2017/queue_manager/queue_manager.h"

TEST(QueueManager, Initalizes) { o2017::QueueManager::GetInstance(); }

TEST(QueueManager, QueueWorks) {
  muan::proto::StackProto<PdpStatus, 512> p;
  o2017::QueueManager::GetInstance()->pdp_status_queue().WriteMessage(p);

  auto pdp_status_reader =
      o2017::QueueManager::GetInstance()->pdp_status_queue().MakeReader();

  // Test that it reads one message (the message we sent above), then that it
  // doesn't have any new messages (because we just read the only message).
  ASSERT_TRUE(pdp_status_reader.ReadMessage());
  ASSERT_FALSE(pdp_status_reader.ReadMessage());
}

// Concord
//
// Copyright (c) 2018-2020 VMware, Inc. All Rights Reserved.
//
// This product is licensed to you under the Apache 2.0 license (the "License"). You may not use this product except in
// compliance with the Apache 2.0 License.
//
// This product may include a number of subcomponents with separate copyright notices and license terms. Your use of
// these subcomponents is subject to the terms and conditions of the subcomponent's license, as noted in the LICENSE
// file.

#include "gtest/gtest.h"

#include <tuple>
#include "helper.hpp"
#include "DigestType.h"
#include "ViewsManager.hpp"
#include "ReplicasInfo.hpp"
#include "SigManager.hpp"
#include "messages/MsgCode.hpp"
#include "messages/ViewChangeMsg.hpp"
#include "bftengine/ClientMsgs.hpp"
#include "bftengine/ReplicaConfig.hpp"

using namespace bftEngine;
using namespace bftEngine::impl;

ReplicaConfig& config = createReplicaConfig();
const char rawSpanContext[] = {"span_\0context"};

void ViewChangeMsgTests(bool bAddElements, bool bAddComplaints, const std::string& spanContext = "") {
  ReplicaId senderId = 1u;
  ViewNum viewNum = 2u;
  SeqNum seqNum = 3u;
  ReplicasInfo replicaInfo(config, true, true);
  SigManager sigManager(config.replicaId,
                        config.numReplicas + config.numOfClientProxies,
                        config.replicaPrivateKey,
                        config.publicKeysOfReplicas);
  ViewsManager manager(&replicaInfo, &sigManager, CryptoManager::instance().thresholdVerifierForSlowPathCommit());
  ViewChangeMsg msg(senderId, viewNum, seqNum, concordUtils::SpanContext{spanContext});
  EXPECT_EQ(msg.idOfGeneratedReplica(), senderId);
  EXPECT_EQ(msg.newView(), viewNum);
  EXPECT_EQ(msg.lastStable(), seqNum);
  EXPECT_EQ(msg.numberOfElements(), 0u);
  viewNum++;
  msg.setNewViewNumber(viewNum);
  EXPECT_EQ(msg.newView(), viewNum);
  testMessageBaseMethods(msg, MsgCode::ViewChange, senderId, spanContext);

  typedef std::tuple<SeqNum, Digest, ViewNum, bool, ViewNum, size_t, char*> InputTuple;
  std::vector<InputTuple> inputData;
  if (bAddElements) {
    Digest digest1(1);
    auto originalViewNum1 = viewNum;
    auto viewNum1 = ++viewNum;
    char certificate1[DIGEST_SIZE] = {1};
    auto seqNum1 = ++seqNum;
    inputData.push_back(
        std::make_tuple(seqNum1, digest1, viewNum1, true, originalViewNum1, sizeof(certificate1), certificate1));
    msg.addElement(seqNum1, digest1, viewNum1, true, originalViewNum1, sizeof(certificate1), certificate1);
    Digest digest2(2);
    auto originalViewNum2 = viewNum;
    auto viewNum2 = ++viewNum;
    char certificate2[DIGEST_SIZE] = {2};
    auto seqNum2 = ++seqNum;
    inputData.push_back(
        std::make_tuple(seqNum2, digest2, viewNum2, true, originalViewNum2, sizeof(certificate2), certificate2));
    msg.addElement(seqNum2, digest2, viewNum2, true, originalViewNum2, sizeof(certificate2), certificate2);
    EXPECT_EQ(msg.numberOfElements(), 2);
  }
  msg.setNewViewNumber(++viewNum);

  uint32_t totalSizeOfComplaints = 0;
  uint32_t numberOfComplaints = 0;
  if (bAddComplaints) {
    for (ReplicaId sender = 1; sender < 4; sender++) {
      std::unique_ptr<ReplicaAsksToLeaveViewMsg> msg_complaint(
          ReplicaAsksToLeaveViewMsg::create(sender,
                                            viewNum,
                                            ReplicaAsksToLeaveViewMsg::Reason::ClientRequestTimeout,
                                            concordUtils::SpanContext{spanContext}));
      EXPECT_EQ(msg_complaint->idOfGeneratedReplica(), sender);
      EXPECT_EQ(msg_complaint->viewNumber(), viewNum);
      EXPECT_EQ(msg_complaint->reason(), ReplicaAsksToLeaveViewMsg::Reason::ClientRequestTimeout);

      testMessageBaseMethods(*msg_complaint.get(), MsgCode::ReplicaAsksToLeaveView, sender, spanContext);

      EXPECT_NO_THROW(msg_complaint->validate(replicaInfo));

      msg.addComplaint(msg_complaint.get());

      totalSizeOfComplaints += sizeof(decltype(msg_complaint->size()));
      totalSizeOfComplaints += msg_complaint->size();
      numberOfComplaints++;
    }
    EXPECT_EQ(msg.numberOfComplaints(), numberOfComplaints);
    EXPECT_EQ(msg.sizeOfAllComplaints(), totalSizeOfComplaints);
  }

  msg.finalizeMessage();
  EXPECT_EQ(msg.numberOfElements(), bAddElements ? 2 : 0);
  EXPECT_EQ(msg.numberOfComplaints(), numberOfComplaints);
  EXPECT_EQ(msg.sizeOfAllComplaints(), totalSizeOfComplaints);
  EXPECT_NO_THROW(msg.validate(replicaInfo));

  {
    uint32_t packedComplaints = 0;
    ViewChangeMsg::ComplaintsIterator iter(&msg);
    char* complaint = nullptr;
    MsgSize size = 0;
    while (iter.getAndGoToNext(complaint, size)) {
      auto Msg = MessageBase(msg.senderId(), (MessageBase::Header*)complaint, size, false);
      auto msg_complaint = std::make_unique<ReplicaAsksToLeaveViewMsg>(&Msg);
      EXPECT_NO_THROW(msg_complaint->validate(replicaInfo));
      packedComplaints++;
    }
    EXPECT_EQ(packedComplaints, numberOfComplaints);
  }

  testMessageBaseMethods(msg, MsgCode::ViewChange, senderId, spanContext);

  if (bAddElements) {
    {
      ViewChangeMsg::ElementsIterator iter(&msg);
      for (size_t i = 0; !iter.end(); ++i) {
        ViewChangeMsg::Element* currentElement = nullptr;
        iter.getCurrent(currentElement);
        ViewChangeMsg::Element* element = nullptr;
        EXPECT_TRUE(iter.getAndGoToNext(element));
        EXPECT_EQ(element, currentElement);
        EXPECT_EQ(element->hasPreparedCertificate, true);
        EXPECT_EQ(element->originView, std::get<2>(inputData[i]));
        EXPECT_EQ(element->hasPreparedCertificate, std::get<3>(inputData[i]));
        EXPECT_EQ(element->prePrepareDigest, std::get<1>(inputData[i]));
        EXPECT_EQ(element->seqNum, std::get<0>(inputData[i]));
      }
    }
    {
      ViewChangeMsg::ElementsIterator iter(&msg);
      size_t i = 0;
      for (; !iter.end(); ++i) {
        iter.gotoNext();
      }
      EXPECT_EQ(i, msg.numberOfElements());
    }
    {
      ViewChangeMsg::ElementsIterator iter(&msg);
      ViewChangeMsg::Element* element = nullptr;
      iter.getCurrent(element);
    }
    {
      ViewChangeMsg::ElementsIterator iter(&msg);
      for (const auto& t : inputData) {
        EXPECT_TRUE(iter.goToAtLeast(std::get<0>(t)));
        ViewChangeMsg::Element* element = nullptr;
        iter.getCurrent(element);
        EXPECT_EQ(element->hasPreparedCertificate, true);
        EXPECT_EQ(element->originView, std::get<2>(t));
        EXPECT_EQ(element->hasPreparedCertificate, std::get<3>(t));
        EXPECT_EQ(element->prePrepareDigest, std::get<1>(t));
        EXPECT_EQ(element->seqNum, std::get<0>(t));
      }
    }
    {
      ViewChangeMsg::ElementsIterator iter(&msg);
      EXPECT_FALSE(iter.goToAtLeast(0xFFFF));
    }
  }
}

TEST(ViewChangeMsg, base_methods_no_span) { ViewChangeMsgTests(false, false); }

TEST(ViewChangeMsg, add_elements_no_span) { ViewChangeMsgTests(true, false); }

TEST(ViewChangeMsg, add_complaints_no_span) { ViewChangeMsgTests(false, true); }

TEST(ViewChangeMsg, add_elements_and_complaints_no_span) { ViewChangeMsgTests(true, true); }

TEST(ViewChangeMsg, base_methods_with_span) { ViewChangeMsgTests(false, false, rawSpanContext); }

TEST(ViewChangeMsg, add_elements_with_span) { ViewChangeMsgTests(true, false, rawSpanContext); }

TEST(ViewChangeMsg, add_complaints_with_span) { ViewChangeMsgTests(false, true, rawSpanContext); }

TEST(ViewChangeMsg, add_elements_and_complaints_with_span) { ViewChangeMsgTests(true, true, rawSpanContext); }

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

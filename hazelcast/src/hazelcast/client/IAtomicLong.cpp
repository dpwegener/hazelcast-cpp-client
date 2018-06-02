/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "hazelcast/client/IAtomicLong.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/AtomicLongAddAndGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongCompareAndSetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongDecrementAndGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetAndAddCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetAndSetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongIncrementAndGetCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongGetAndIncrementCodec.h"
#include "hazelcast/client/protocol/codec/AtomicLongSetCodec.h"

#include "hazelcast/client/proxy/ProxyImpl.h"

namespace hazelcast {
    namespace client {
        IAtomicLong::IAtomicLong(const std::string& objectName, spi::ClientContext *context)
        : proxy::ProxyImpl("hz:impl:atomicLongService", objectName, context) {
            serialization::pimpl::Data keyData = context->getSerializationService().toData<std::string>(&objectName);
            partitionId = getPartitionId(keyData);
        }

        int64_t IAtomicLong::addAndGet(int64_t delta) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongAddAndGetCodec::encodeRequest(getName(), delta);

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongAddAndGetCodec::ResponseParameters>(std::move(request), partitionId);
        }

        bool IAtomicLong::compareAndSet(int64_t expect, int64_t update) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongCompareAndSetCodec::encodeRequest(getName(), expect, update);

            return invokeAndGetResult<bool, protocol::codec::AtomicLongCompareAndSetCodec::ResponseParameters>(std::move(request), partitionId);
        }

        int64_t IAtomicLong::decrementAndGet() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongDecrementAndGetCodec::encodeRequest(getName());

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongDecrementAndGetCodec::ResponseParameters>(std::move(request), partitionId);
        }

        int64_t IAtomicLong::get() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetCodec::encodeRequest(getName());

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetCodec::ResponseParameters>(std::move(request), partitionId);
        }

        int64_t IAtomicLong::getAndAdd(int64_t delta) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetAndAddCodec::encodeRequest(getName(), delta);

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndAddCodec::ResponseParameters>(std::move(request), partitionId);
        }

        int64_t IAtomicLong::getAndSet(int64_t newValue) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetAndSetCodec::encodeRequest(getName(), newValue);

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndSetCodec::ResponseParameters>(std::move(request), partitionId);
        }

        int64_t IAtomicLong::incrementAndGet() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongIncrementAndGetCodec::encodeRequest(getName());

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongIncrementAndGetCodec::ResponseParameters>(std::move(request), partitionId);
        }

        int64_t IAtomicLong::getAndIncrement() {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongGetAndIncrementCodec::encodeRequest(getName());

            return invokeAndGetResult<int64_t, protocol::codec::AtomicLongGetAndIncrementCodec::ResponseParameters>(std::move(request), partitionId);
        }

        void IAtomicLong::set(int64_t newValue) {
            std::unique_ptr<protocol::ClientMessage> request =
                    protocol::codec::AtomicLongSetCodec::encodeRequest(getName(), newValue);

            invokeOnPartition(std::move(request), partitionId);
        }
    }
}

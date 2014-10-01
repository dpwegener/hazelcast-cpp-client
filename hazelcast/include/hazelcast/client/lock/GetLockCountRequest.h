//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetLockCountRequest
#define HAZELCAST_GetLockCountRequest

#include "hazelcast/client/impl/ClientRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        namespace lock {
            class GetLockCountRequest : public impl::ClientRequest {
            public:
                GetLockCountRequest(serialization::pimpl::Data& key);

                int getClassId() const;

                int getFactoryId() const;

                void write(serialization::PortableWriter& writer) const;

                bool isRetryable() const;

            private:

                serialization::pimpl::Data& key;
            };
        }
    }
}


#endif //HAZELCAST_GetLockCountRequest


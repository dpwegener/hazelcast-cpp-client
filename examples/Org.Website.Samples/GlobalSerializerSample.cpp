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
#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;

class GlobalSerializer : public serialization::StreamSerializer {
public:
    virtual int32_t getHazelcastTypeId() const {
        return 20;
    }

    virtual void write(serialization::ObjectDataOutput &out, const void *object) {
        // out.write(MyFavoriteSerializer.serialize(object))
    }

    virtual void *read(serialization::ObjectDataInput &in) {
        // return MyFavoriteSerializer.deserialize(in);
        return NULL;
    }
};

int main() {
    // Start the Hazelcast Client and connect to an already running Hazelcast Cluster on 127.0.0.1
    ClientConfig clientConfig;
    clientConfig.getSerializationConfig().setGlobalSerializer(
            boost::shared_ptr<serialization::StreamSerializer>(new GlobalSerializer()));

    HazelcastClient hz(clientConfig);

    return 0;
}

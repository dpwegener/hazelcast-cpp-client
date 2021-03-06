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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_

#include <stdint.h>
#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/client/spi/ClientListenerService.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        namespace connection {
            class Connection;
        }
        namespace protocol {
            class ClientMessage;
        }
        namespace spi {
            class ClientContext;

            namespace impl {
                namespace listener {
                    class HAZELCAST_API AbstractClientListenerService : public ClientListenerService {
                    public:
                        virtual ~AbstractClientListenerService();

                        AbstractClientListenerService(ClientContext &clientContext, int32_t eventThreadCount,
                                                      int32_t eventQueueCapacity);

                        virtual void start();

                        void shutdown();

                        void addEventHandler(int64_t callId,
                                             const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        void handleClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                 const boost::shared_ptr<connection::Connection> &connection);

                    protected:
                        class ClientEventProcessor : public util::StripedRunnable {
                            friend class AbstractClientListenerService;
                        public:
                            virtual ~ClientEventProcessor();

                            virtual void run();

                            virtual const std::string getName() const;

                            virtual int32_t getKey();

                        private:
                            ClientEventProcessor(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                 const boost::shared_ptr<connection::Connection> &connection,
                                                 util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > &eventHandlerMap,
                                                 util::ILogger &logger);

                            const boost::shared_ptr<protocol::ClientMessage> clientMessage;
                            util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > &eventHandlerMap;
                            util::ILogger &logger;
                        };

                        class RegisterListenerTask : public util::Callable<std::string> {
                        public:
                            RegisterListenerTask(const std::string &taskName,
                                                 const boost::shared_ptr<AbstractClientListenerService> &listenerService,
                                                 const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                                                 const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                            virtual std::string call();

                            virtual const std::string getName() const;

                        private:
                            std::string taskName;
                            boost::shared_ptr<AbstractClientListenerService> listenerService;
                            boost::shared_ptr<impl::ListenerMessageCodec> listenerMessageCodec;
                            boost::shared_ptr<EventHandler<protocol::ClientMessage> > handler;
                        };

                        class DeregisterListenerTask : public util::Callable<bool> {
                        public:
                            DeregisterListenerTask(const std::string &taskName,
                                                   const boost::shared_ptr<AbstractClientListenerService> &listenerService,
                                                   const std::string &registrationId);

                            virtual bool call();

                            virtual const std::string getName() const;

                        private:
                            std::string taskName;
                            boost::shared_ptr<AbstractClientListenerService> listenerService;
                            std::string registrationId;
                        };

                        class ConnectionAddedTask : public util::Runnable {
                        public:
                            ConnectionAddedTask(const std::string &taskName,
                                                const boost::shared_ptr<AbstractClientListenerService> &listenerService,
                                                const boost::shared_ptr<connection::Connection> &connection);

                            virtual const std::string getName() const;

                            virtual void run();

                        private:
                            std::string taskName;
                            boost::shared_ptr<AbstractClientListenerService> listenerService;
                            const boost::shared_ptr<connection::Connection> connection;
                        };

                        void removeEventHandler(int64_t callId);

                        virtual std::string registerListenerInternal(
                                const boost::shared_ptr<ListenerMessageCodec> &listenerMessageCodec,
                                const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler) = 0;

                        virtual bool deregisterListenerInternal(const std::string &registrationId) = 0;

                        virtual void
                        connectionAddedInternal(const boost::shared_ptr<connection::Connection> &connection) = 0;

                        util::SynchronizedMap<int64_t, EventHandler<protocol::ClientMessage> > eventHandlerMap;
                        ClientContext &clientContext;
                        serialization::pimpl::SerializationService &serializationService;
                        util::ILogger &logger;
                        util::impl::SimpleExecutorService eventExecutor;
                        util::impl::SimpleExecutorService registrationExecutor;
                    };
                }
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_ABSTRACTCLIENTLISTERNERSERVICE_H_

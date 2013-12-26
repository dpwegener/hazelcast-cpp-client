//
// Created by sancar koyunlu on 8/14/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#include "hazelcast/util/AtomicPointer.h"
#include <boost/thread.hpp>
#include <memory>

#ifndef HAZELCAST_Future
#define HAZELCAST_Future

namespace hazelcast {
    namespace client {

        class HAZELCAST_API FutureStatus {
        public:
            enum Status {
                READY,
                TIMEOUT
            } value;

            FutureStatus(Status status)
            :value(status) {

            };

            bool operator ==(Status status) const {
                return this->value == status;
            };

            bool operator !=(Status status) const {
                return !(*this == status);
            };
        };

        namespace pImpl {
            template<typename R>
            class HAZELCAST_API FutureBase {
            public:
                FutureBase()
                :result(NULL)
                , exception(NULL)
                , lock(mutex)
                , isValid(false) {

                };

                R& get() {
                    wait();
                    if (exception.get() != NULL)
                        throw *exception;
                    return *result;
                };

                bool valid() const {
                    return isValid;
                };

                void wait() {
                    if (isValid)
                        return;
                    condition.wait(lock);
                };

                FutureStatus wait_for(long timeInMillis) {
                    if (isValid)
                        return FutureStatus::READY;
                    boost::cv_status status = condition.wait_for(lock, boost::chrono::milliseconds(timeInMillis));
                    if (status == boost::cv_status::timeout)
                        return FutureStatus::TIMEOUT;
                    else
                        return FutureStatus::READY;
                };


                void setValue(R *value) {
                    result.reset(value);
                    isValid = true;
                    condition.notify_all();
                };

                void setException(std::exception *exception) {
                    this->exception.reset(exception);
                    isValid = true;
                    condition.notify_all();
                }

            private:
                boost::atomic<bool> isValid;
                std::auto_ptr< R > result;
                std::auto_ptr< std::exception> exception;
                boost::mutex mutex;
                boost::condition_variable condition;
                boost::unique_lock< boost::mutex > lock;

            };

        }
        template<typename R>
        class HAZELCAST_API Future {
        public:
            Future():
            basePtr(new pImpl::FutureBase<R>, rand()) {

            };

            pImpl::FutureBase<R>& accessInternal() {
                return *(basePtr.get());
            }

            R& get() {
                return basePtr->get();
            };

            bool valid() const {
                return basePtr->valid();
            };

            void wait() {
                basePtr->wait();
            };

            FutureStatus wait_for(long timeInMillis) {
                return basePtr->wait_for(timeInMillis);
            };

        private:
            util::AtomicPointer<pImpl::FutureBase<R> > basePtr;
        };
    }
}

#endif //HAZELCAST_Future
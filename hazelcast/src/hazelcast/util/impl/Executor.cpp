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

#include <cmath>
#include <boost/foreach.hpp>

#include "hazelcast/util/impl/SimpleExecutorService.h"
#include "hazelcast/util/HashUtil.h"

namespace hazelcast {
    namespace util {
        namespace impl {
            int32_t SimpleExecutorService::DEFAULT_EXECUTOR_QUEUE_CAPACITY = INT32_MAX;

            SimpleExecutorService::SimpleExecutorService(ILogger &logger, const std::string &threadNamePrefix,
                                                         int threadCount,
                                                         int32_t maximumQueueCapacity)
                    : logger(logger), threadNamePrefix(threadNamePrefix), threadCount(threadCount), live(true),
                      threadIdGenerator(0), workers(threadCount), maximumQueueCapacity(maximumQueueCapacity) {
                startWorkers();
            }

            SimpleExecutorService::SimpleExecutorService(ILogger &logger, const std::string &threadNamePrefix,
                                                         int threadCount)
                    : logger(logger), threadNamePrefix(threadNamePrefix), threadCount(threadCount), live(true),
                      threadIdGenerator(0), workers(threadCount),
                      maximumQueueCapacity(DEFAULT_EXECUTOR_QUEUE_CAPACITY) {
                startWorkers();
            }

            void SimpleExecutorService::startWorkers() {
                // `maximumQueueCapacity` is the given max capacity for this executor. Each worker in this executor should consume
                // only a portion of that capacity. Otherwise we will have `threadCount * maximumQueueCapacity` instead of
                // `maximumQueueCapacity`.
                int32_t perThreadMaxQueueCapacity = static_cast<int32_t>(ceil(
                        (double) 1.0 * maximumQueueCapacity / threadCount));
                for (int i = 0; i < threadCount; i++) {
                    workers[i].reset(new Worker(*this, perThreadMaxQueueCapacity));
                    workers[i]->start();
                }
            }

            void SimpleExecutorService::execute(const boost::shared_ptr<Runnable> &command) {
                if (command.get() == NULL) {
                    throw client::exception::NullPointerException("SimpleExecutor::execute", "command can't be null");
                }

                if (!live) {
                    throw client::exception::RejectedExecutionException("SimpleExecutor::execute",
                                                                        "Executor is terminated!");
                }

                boost::shared_ptr<Worker> worker = getWorker(command);
                worker->schedule(command);
            }

            boost::shared_ptr<SimpleExecutorService::Worker>
            SimpleExecutorService::getWorker(const boost::shared_ptr<Runnable> &runnable) {
                int32_t key;
                if (runnable->isStriped()) {
                    key = boost::static_pointer_cast<StripedRunnable>(runnable)->getKey();
                } else {
                    key = (int32_t) rand();
                }
                int index = HashUtil::hashToIndex(key, threadCount);
                return workers[index];
            }

            void SimpleExecutorService::shutdown() {
                if (!live.compareAndSet(true, false)) {
                    return;
                }

                BOOST_FOREACH(boost::shared_ptr<Worker> &worker, workers) {
                                worker->shutdown();
                            }

                boost::shared_ptr<util::Thread> runner;
                while ((runner = delayedRunners.poll()).get()) {
                    boost::static_pointer_cast<DelayedRunner>(runner->getTarget())->shutdown();
                    runner->cancel();
                    runner->join();
                }
            }

            SimpleExecutorService::~SimpleExecutorService() {
                shutdown();
            }

            void SimpleExecutorService::schedule(const boost::shared_ptr<util::Runnable> &command,
                                                 int64_t initialDelayInMillis) {
                if (command.get() == NULL) {
                    throw client::exception::NullPointerException("SimpleExecutor::schedule", "command can't be null");
                }

                if (!live) {
                    throw client::exception::RejectedExecutionException("SimpleExecutor::schedule",
                                                                        "Executor is terminated!");
                }

                boost::shared_ptr<DelayedRunner> delayedRunner(new DelayedRunner(command, initialDelayInMillis));
                boost::shared_ptr<util::Thread> thread(new util::Thread(delayedRunner));
                delayedRunner->setStartTimeMillis(thread.get());
                thread->start();
                delayedRunners.offer(thread);
            }

            void SimpleExecutorService::scheduleAtFixedRate(const boost::shared_ptr<util::Runnable> &command,
                                                            int64_t initialDelayInMillis, int64_t periodInMillis) {
                if (command.get() == NULL) {
                    throw client::exception::NullPointerException("SimpleExecutor::scheduleAtFixedRate",
                                                                  "command can't be null");
                }

                if (!live) {
                    throw client::exception::RejectedExecutionException("SimpleExecutor::scheduleAtFixedRate",
                                                                        "Executor is terminated!");
                }

                boost::shared_ptr<RepeatingRunner> repeatingRunner(
                        new RepeatingRunner(command, initialDelayInMillis, periodInMillis));
                boost::shared_ptr<util::Thread> thread(new util::Thread(repeatingRunner));
                repeatingRunner->setStartTimeMillis(thread.get());
                thread->start();
                delayedRunners.offer(thread);
            }

            void SimpleExecutorService::Worker::run() {
                boost::shared_ptr<Runnable> task;
                while (executorService.live || !workQueue.isEmpty()) {
                    try {
                        task = workQueue.pop();
                        if (task.get()) {
                            task->run();
                        }
                    } catch (client::exception::InterruptedException &) {
                        executorService.logger.finest() << getName() << " is interrupted .";
                    } catch (client::exception::IException &t) {
                        executorService.logger.warning() << getName() << " caused an exception" << t;
                    }
                }

                assert(workQueue.isEmpty());
            }

            SimpleExecutorService::Worker::~Worker() {
            }

            void SimpleExecutorService::Worker::schedule(const boost::shared_ptr<Runnable> &runnable) {
                workQueue.push(runnable);
            }

            void SimpleExecutorService::Worker::start() {
                thread.start();
            }

            const std::string SimpleExecutorService::Worker::getName() const {
                return name;
            }

            std::string SimpleExecutorService::Worker::generateThreadName(const std::string &prefix) {
                std::ostringstream out;
                out << prefix << (++executorService.threadIdGenerator);
                return out.str();
            }

            void SimpleExecutorService::Worker::shutdown() {
                workQueue.interrupt();
                thread.cancel();
                thread.join();
            }

            SimpleExecutorService::Worker::Worker(SimpleExecutorService &executorService, int32_t maximumQueueCapacity)
                    :
                    executorService(executorService), name(generateThreadName(executorService.threadNamePrefix)),
                    workQueue((size_t) maximumQueueCapacity),
                    thread(boost::shared_ptr<util::Runnable>(new util::RunnableDelegator(*this))) {
            }

            SimpleExecutorService::RepeatingRunner::RepeatingRunner(
                    const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                    int64_t periodInMillis) : DelayedRunner(command, initialDelayInMillis, periodInMillis) {
            }

            const std::string SimpleExecutorService::RepeatingRunner::getName() const {
                return command->getName();
            }

            SimpleExecutorService::DelayedRunner::DelayedRunner(
                    const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis) : command(
                    command), initialDelayInMillis(initialDelayInMillis), periodInMillis(-1), live(true),
                                                                                                      startTimeMillis(
                                                                                                              0),
                                                                                                      runnerThread(
                                                                                                              NULL) {
            }

            SimpleExecutorService::DelayedRunner::DelayedRunner(
                    const boost::shared_ptr<util::Runnable> &command, int64_t initialDelayInMillis,
                    int64_t periodInMillis) : command(command), initialDelayInMillis(initialDelayInMillis),
                                              periodInMillis(periodInMillis), live(true), startTimeMillis(0),
                                              runnerThread(NULL) {}

            void SimpleExecutorService::DelayedRunner::shutdown() {
                live = false;
            }

            void SimpleExecutorService::DelayedRunner::run() {
                bool isNotRepeating = periodInMillis < 0;
                while (live || isNotRepeating) {
                    if (live) {
                        int64_t waitTimeMillis = startTimeMillis - util::currentTimeMillis();
                        if (waitTimeMillis > 0) {
                            assert(runnerThread != NULL);
                            runnerThread->interruptibleSleepMillis(waitTimeMillis);
                        }
                    }


                    try {
                        command->run();
                    } catch (client::exception::IException &e) {
                        util::ILogger::getLogger().warning() << "Repeated runnable " << getName()
                                                             << " run method caused exception:" << e;
                    }

                    if (isNotRepeating) {
                        return;
                    }

                    startTimeMillis += periodInMillis;
                }
            }

            const std::string SimpleExecutorService::DelayedRunner::getName() const {
                return command->getName();
            }

            void SimpleExecutorService::DelayedRunner::setStartTimeMillis(Thread *pThread) {
                runnerThread = pThread;
                startTimeMillis = util::currentTimeMillis() + initialDelayInMillis;
            }

        }

        boost::shared_ptr<ExecutorService> Executors::newSingleThreadExecutor(const std::string &name) {
            return boost::shared_ptr<ExecutorService>(
                    new impl::SimpleExecutorService(util::ILogger::getLogger(), name, 1));
        }

    }
}


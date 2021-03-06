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
//
// Created by sancar koyunlu on 15/05/14.
//

#ifndef HAZELCAST_TestHelperFunctions
#define HAZELCAST_TestHelperFunctions

#define ASSERT_EQ_EVENTUALLY(expected, actual) do{              \
            bool result = false;                                \
            for(int i = 0 ; i < 120 * 5 && !result ; i++ ) {    \
                if (expected == actual) {                       \
                    result = true;                              \
                } else {                                        \
                    util::sleepmillis(200);                     \
                }                                               \
            }                                                   \
            ASSERT_TRUE(result);                                \
      }while(0)                                                 \

#define WAIT_TRUE_EVENTUALLY(expression) do{                    \
            for(int i = 0 ; i < 5 * 120 && !(expression) ; i++ ) { \
                util::sleepmillis(200);                         \
            }                                                   \
      }while(0)                                                 \

#define WAIT_FALSE_EVENTUALLY(expression) WAIT_TRUE_EVENTUALLY(false == expression)
#define WAIT_EQ_EVENTUALLY(expected, expression) WAIT_TRUE_EVENTUALLY(expected == expression)
#define WAIT_NE_EVENTUALLY(expected, expression) WAIT_NE_EVENTUALLY(expected != expression)

#define ASSERT_NE_EVENTUALLY(expected, actual) do{              \
            bool result = false;                                \
            for(int i = 0 ; i < 120 * 5 && !result ; i++ ) {    \
                if (expected != actual) {                       \
                    result = true;                              \
                } else {                                        \
                    util::sleepmillis(200);                     \
                }                                               \
            }                                                   \
            ASSERT_TRUE(result);                                \
      }while(0)                                                 \


#define ASSERT_NULL(msg, value, type) ASSERT_EQ((type *) NULL, value) << msg
#define ASSERT_NOTNULL(value, type) ASSERT_NE((type *) NULL, value)
#define ASSERT_TRUE_EVENTUALLY(value) ASSERT_EQ_EVENTUALLY(value, true)
#define ASSERT_FALSE_EVENTUALLY(value) ASSERT_EQ_EVENTUALLY(value, false)
#define ASSERT_NULL_EVENTUALLY(value, type) ASSERT_EQ_EVENTUALLY((type *) NULL, value)
#define ASSERT_NOTNULL_EVENTUALLY(value) ASSERT_NE_EVENTUALLY(value, NULL)
#define ASSERT_OPEN_EVENTUALLY(latch) ASSERT_TRUE(latch.await(120))

#endif //HAZELCAST_TestHelperFunctions

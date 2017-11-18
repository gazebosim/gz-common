/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef IGNITION_COMMON_DETAIL_SUPPRESSWARNING_HH_
#define IGNITION_COMMON_DETAIL_SUPPRESSWARNING_HH_

#define DETAIL_IGN_COMMON_STRINGIFY(x) #x

/* cppcheck-suppress */

// BEGIN / FINISH Macros
#if defined __GNUC__


  #define DETAIL_IGN_COMMON_BEGIN_WARN_SUP_PUSH \
    _Pragma("GCC diagnostic push")


  #define DETAIL_IGN_COMMON_WARN_SUP_HELPER_2(w) \
    DETAIL_IGN_COMMON_STRINGIFY(GCC diagnostic ignored w)


  #define DETAIL_IGN_COMMON_WARN_SUP_HELPER(w) \
    _Pragma(DETAIL_IGN_COMMON_WARN_SUP_HELPER_2(w))


  #define DETAIL_IGN_COMMON_FINISH_WARNING_SUPPRESSION(warning_token) \
    _Pragma("GCC diagnostic pop")


#elif defined __clang__


  #define DETAIL_IGN_COMMON_BEGIN_WARN_SUP_PUSH \
    _Pragma("clang diagnostic push")


  #define DETAIL_IGN_COMMON_WARN_SUP_HELPER_2(w) \
    DETAIL_IGN_COMMON_STRINGIFY(clang diagnostic ignored w)


  #define DETAIL_IGN_COMMON_WARN_SUP_HELPER(w) \
    _Pragma(DETAIL_IGN_COMMON_WARN_SUP_HELPER_2(w))


  #define DETAIL_IGN_COMMON_FINISH_WARNING_SUPPRESSION(warning_token) \
    _Pragma("clang diagnostic pop")


#elif defined _MSC_VER


  #define DETAIL_IGN_COMMON_BEGIN_WARN_SUP_PUSH \
    __pragma(warning(push))


  #define DETAIL_IGN_COMMON_WARN_SUP_HELPER(w) \
    __pragma(warning(disable: w))


  #define DETAIL_IGN_COMMON_FINISH_WARNING_SUPPRESSION(warning_token) \
    __pragma(warning(pop))


#else

  // Make these into no-ops if we don't know the type of compiler

  #define DETAIL_IGN_COMMON_BEGIN_WARN_SUP_PUSH


  #define DETAIL_IGN_COMMON_WARN_SUP_HELPER(w)


  #define DETAIL_IGN_COMMON_FINISH_WARNING_SUPPRESSION(warning_token)


#endif


#define DETAIL_IGN_COMMON_BEGIN_WARNING_SUPPRESSION(warning_token) \
  DETAIL_IGN_COMMON_BEGIN_WARN_SUP_PUSH \
  DETAIL_IGN_COMMON_WARN_SUP_HELPER(warning_token)



// Warning Tokens
#if defined __GNUC__ || defined __clang__

  #define DETAIL_IGN_COMMON_DELETE_NON_VIRTUAL_DESTRUCTOR \
    "-Wdelete-non-virtual-dtor"


#elif defined _MSC_VER

  #define DETAIL_IGN_COMMON_DELETE_NON_VIRTUAL_DESTRUCTOR 4265


#else

  #define DETAIL_IGN_COMMON_DELETE_NON_VIRTUAL_DESTRUCTOR


#endif


#endif

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


#ifndef IGNITION_COMMON_DETAIL_SPECIALIZEDPLUGINPTRMACROS_HH_
#define IGNITION_COMMON_DETAIL_SPECIALIZEDPLUGINPTRMACROS_HH_

/// \brief Create constructors and assignment operators for the specialized
/// plugin types. This allows us to implement and maintain these functions in
/// one location. This is used in the various template specializations of
/// the SpecializedPluginPtr class.
#define DETAIL_IGN_COMMON_PLUGIN_CONSTRUCT_DESTRUCT_ASSIGN(ClassName)\
  public: virtual ~ClassName() = default;\
  public: ClassName(const PluginPtr &_other)\
            : ClassName()\
          {\
            static_cast<PluginPtr&>(*this) = _other;\
          }\
  public: ClassName(PluginPtr &&_other)\
            : ClassName()\
          {\
            static_cast<PluginPtr&>(*this) = std::move(_other);\
          }\
  public: ClassName& operator=(const PluginPtr &_other)\
          {\
            static_cast<PluginPtr&>(*this) = _other;\
            return *this;\
          }\
  public: ClassName& operator=(PluginPtr &&_other)\
          {\
            static_cast<PluginPtr&>(*this) = std::move(_other);\
            return *this;\
          }


/// \brief Implement functions whose only role is to dispatch its functionality
/// between two base classes, depending on which base is specialized for the
/// template type. This must only be called within the ComposePlugin class.
#define DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH_IMPL(\
              ReturnType, Function, Suffix, Args)\
  public:\
  template <class T>\
  ReturnType Function Suffix\
  {\
    if(Base1::template IsSpecializedFor<T>())\
      return Base1::template Function <T> Args ;\
  \
    return Base2::template Function <T> Args ;\
  }

#endif // IGNITION_COMMON_DETAIL_SPECIALIZEDPLUGINPTRMACROS_HH_

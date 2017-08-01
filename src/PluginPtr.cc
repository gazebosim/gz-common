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

#include <map>

#include "ignition/common/PluginPtr.hh"
#include "ignition/common/PluginInfo.hh"
#include "ignition/common/Console.hh"
#include "PluginUtils.hh"


namespace ignition
{
  namespace common
  {
    class PluginPtrPrivate
    {
      /// \brief Constructor that uses a PluginInfo instance
      public: PluginPtrPrivate(const PluginInfo *_info)
                : pluginInstancePtr(nullptr)
              {
                Initialize(_info);
              }

      /// \brief Constructor that uses another PluginPtrPrivate instance
      public: PluginPtrPrivate(const PluginPtrPrivate *_other)
              {
                Initialize(_other);
              }


      /// \brief Map from interface names to their locations within the plugin
      /// instance
      //
      // Dev Note (MXG): We use std::map here instead of std::unordered_map
      // because iterators to a std::map are not invalidated by the insertion
      // operation (whereas all iterators to a std::unordered_map are
      // potentially invalidated each time an insertion is performed on the
      // std::unordered_map). Holding onto valid iterators allows us to do
      // optimizations with template magic to provide direct access to
      // interfaces whose availability we can anticipate at run time.
      //
      // It is also worth noting that ordered vs unordered lookup time is very
      // similar on sets where the strings have relatively small lengths (5-20
      // characters) and a relatively small number of entries in the set (5-20
      // entries). Those conditions match our expected use case here. In fact,
      // ordered lookup can sometimes outperform unordered in these conditions.
      public: PluginPtr::InterfaceMap interfaces;

      /// \brief shared_ptr which manages the lifecycle of the plugin instance.
      std::shared_ptr<void> pluginInstancePtr;

      /// \brief Clear this PluginPtrPrivate without invaliding any map entry
      /// iterators.
      public: void Clear()
              {
                this->pluginInstancePtr.reset();

                // Dev note (MXG): We must NOT call clear() on the InterfaceMap
                // or remove ANY of the map entries, because that would
                // potentially invalidate all of the iterators that are pointing
                // to map entries. This would break any specialized plugins that
                // provide instant access to specialized interfaces. Instead, we
                // simply overwrite the map entries with a nullptr.
                for(auto& entry : this->interfaces)
                  entry.second = nullptr;
              }

      /// \brief Initialize this PluginPtrPrivate using some PluginInfo instance
      public: void Initialize(const PluginInfo *_info)
              {
                Clear();

                if(!_info)
                  return;

                this->pluginInstancePtr =
                    std::shared_ptr<void>(_info->factory(), _info->deleter);

                void * const instance = this->pluginInstancePtr.get();
                if(this->pluginInstancePtr)
                {
                  for(const auto &entry : _info->interfaces)
                  {
                    // entry.first:  name of the interface
                    // entry.second: function which casts the pluginInstance
                    //               pointer to the correct location of the
                    //               interface within the plugin
                    this->interfaces[entry.first] = entry.second(instance);
                  }
                }
              }

      /// \brief Initialize this PluginPtrPrivate using another instance
      public: void Initialize(const PluginPtrPrivate *_other)
              {
                Clear();

                if(!_other)
                {
                  ignerr << "Received a nullptr _other in the constructor "
                         << "which uses `const PluginPtrPrivate*`. This should "
                         << "not be possible! Please report this bug."
                         << std::endl;
                  assert(false);
                }

                this->pluginInstancePtr = _other->pluginInstancePtr;

                if(this->pluginInstancePtr)
                {
                  for(const auto &entry : _other->interfaces)
                  {
                    // entry.first:  name of the interface
                    // entry.second: pointer to the location of that interface
                    //               within the plugin instance
                    this->interfaces[entry.first] = entry.second;
                  }
                }
              }
    };

    PluginPtr::~PluginPtr()
    {
      delete dataPtr;
    }

    PluginPtr::PluginPtr()
      : dataPtr(new PluginPtrPrivate(
                  static_cast<const PluginInfo*>(nullptr)))
    {
      // Do nothing

      // Dev note (MXG): We static_cast to a `const PluginInfo*` to ensure that
      // the constructor which expects a `const PluginInfo*` gets used. That
      // constructor is designed to accept a nullptr, whereas the constructor
      // which expects a `const PluginPtrPrivate*` should never receive a
      // nullptr because that would be indicative of a bug.
    }

    PluginPtr::PluginPtr(const PluginPtr &_other)
      : dataPtr(new PluginPtrPrivate(_other.dataPtr))
    {
      // Do nothing
    }

    PluginPtr& PluginPtr::operator =(const PluginPtr &_other)
    {
      this->dataPtr->Initialize(_other.dataPtr);
      return *this;
    }

    PluginPtr::PluginPtr(PluginPtr &&_other)
      : dataPtr(new PluginPtrPrivate(_other.dataPtr))
    {
      // Do nothing

      // Dev note (MXG): Our options for performing a move operation are
      // somewhat limited, because we cannot do anything that would invalidate
      // the entries in this->dataPtr->interfaces. We could do a move operation
      // on this->dataPtr->pluginInstancePtr, but I think the benefits would be
      // negligible compared to the extra machinery needed to perform that move.
    }

    PluginPtr& PluginPtr::operator =(PluginPtr &&_other)
    {
      this->dataPtr->Initialize(_other.dataPtr);
      return *this;

      // Same note as the move constructor above.
    }

    #define IGN_COMMON_PLUGINPTR_IMPLEMENT_OPERATOR( op )\
      bool PluginPtr::operator op (const PluginPtr &_other) const\
      {\
        return (this->dataPtr->pluginInstancePtr op \
                _other.dataPtr->pluginInstancePtr);\
      }

    IGN_COMMON_PLUGINPTR_IMPLEMENT_OPERATOR(==)
    IGN_COMMON_PLUGINPTR_IMPLEMENT_OPERATOR(<)
    IGN_COMMON_PLUGINPTR_IMPLEMENT_OPERATOR(>)
    IGN_COMMON_PLUGINPTR_IMPLEMENT_OPERATOR(!=)
    IGN_COMMON_PLUGINPTR_IMPLEMENT_OPERATOR(<=)
    IGN_COMMON_PLUGINPTR_IMPLEMENT_OPERATOR(>=)

    std::size_t PluginPtr::Hash() const
    {
      return std::hash<std::shared_ptr<void>>()(
                   this->dataPtr->pluginInstancePtr);
    }

    bool PluginPtr::HasInterface(const std::string &_interfaceName) const
    {
      const std::string interfaceName = NormalizeName(_interfaceName);
      return (this->dataPtr->interfaces.count(interfaceName) != 0);
    }

    bool PluginPtr::IsValid() const
    {
      return (nullptr != this->dataPtr->pluginInstancePtr);
    }

    void PluginPtr::Clear()
    {
      this->dataPtr->Clear();
    }

    void *PluginPtr::PrivateGetInterface(const std::string &_interfaceName) const
    {
      const std::string interfaceName = NormalizeName(_interfaceName);
      const auto &it = this->dataPtr->interfaces.find(interfaceName);
      if(this->dataPtr->interfaces.end() == it)
        return nullptr;

      return it->second;
    }

    PluginPtr::PluginPtr(const PluginInfo *info)
      : dataPtr(new PluginPtrPrivate(info))
    {
      // Do nothing
    }

    PluginPtr::InterfaceMap::iterator PluginPtr::PrivateGetOrCreateIterator(
        const std::string &_interfaceName)
    {
      // We want to use the insert function here to avoid accidentally
      // overwriting a value which might exist at the desired map key.
      return this->dataPtr->interfaces.insert(
            std::make_pair(NormalizeName(_interfaceName), nullptr)).first;
    }
  }
}

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


#ifndef IGNITION_COMMON_DETAIL_SPECIALIZEDPLUGINPTR_HH_
#define IGNITION_COMMON_DETAIL_SPECIALIZEDPLUGINPTR_HH_

#include "ignition/common/SpecializedPluginPtr.hh"

// This preprocessor token should only be used by the unittest that is
// responsible for checking that the specialized routines are being used to
// access specialized plugin interfaces.
#ifdef IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS
bool usedSpecializedInterfaceAccess;
#endif


namespace ignition
{
  namespace common
  {
    /////////////////////////////////////////////////
    template <class SpecInterface>
    SpecializedPluginPtr<SpecInterface>::SpecializedPluginPtr()
      : SpecializedPluginPtr(nullptr)
    {
      // Do nothing
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    Interface *SpecializedPluginPtr<SpecInterface>::GetInterface()
    {
      return this->PrivateGetSpecInterface(type<Interface>());
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    const Interface *SpecializedPluginPtr<SpecInterface>::GetInterface() const
    {
      return this->PrivateGetSpecInterface(type<Interface>());
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    bool SpecializedPluginPtr<SpecInterface>::HasInterface() const
    {
      return this->PrivateHasSpecInterface(type<Interface>());
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    constexpr bool SpecializedPluginPtr<SpecInterface>::IsSpecializedFor()
    {
      return PrivateIsSpecializedFor(type<Interface>());
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    Interface *SpecializedPluginPtr<SpecInterface>::PrivateGetSpecInterface(
        type<Interface>)
    {
      return this->PluginPtr::GetInterface<Interface>();
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    SpecInterface *SpecializedPluginPtr<SpecInterface>::PrivateGetSpecInterface(
        type<SpecInterface>)
    {
      #ifdef IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS
      usedSpecializedInterfaceAccess = true;
      #endif
      return static_cast<SpecInterface*>(
            this->privateSpecInterfaceIterator->second);
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    const Interface *SpecializedPluginPtr<SpecInterface>::
    PrivateGetSpecInterface(type<Interface>) const
    {
      return this->PluginPtr::GetInterface<Interface>();
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    const SpecInterface *SpecializedPluginPtr<SpecInterface>::
    PrivateGetSpecInterface(type<SpecInterface>) const
    {
      #ifdef IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS
      usedSpecializedInterfaceAccess = true;
      #endif
      return static_cast<SpecInterface*>(
            this->privateSpecInterfaceIterator->second);
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    bool SpecializedPluginPtr<SpecInterface>::PrivateHasSpecInterface(
        type<Interface>) const
    {
      return this->PluginPtr::HasInterface<Interface>();
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    bool SpecializedPluginPtr<SpecInterface>::PrivateHasSpecInterface(
        type<SpecInterface>) const
    {
      #ifdef IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS
      usedSpecializedInterfaceAccess = true;
      #endif
      return (nullptr != this->privateSpecInterfaceIterator->second);
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    constexpr bool SpecializedPluginPtr<SpecInterface>::PrivateIsSpecializedFor(
        type<Interface>)
    {
      return false;
    }

    template <class SpecInterface>
    constexpr bool SpecializedPluginPtr<SpecInterface>::PrivateIsSpecializedFor(
        type<SpecInterface>)
    {
      return true;
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    SpecializedPluginPtr<SpecInterface>::SpecializedPluginPtr(
        const PluginInfo *_info)
      : TemplatePluginPtr(_info),
        privateSpecInterfaceIterator(
          this->PrivateGetOrCreateIterator(SpecInterface::InterfaceName))
    {
      // Do nothing. We are delegating to the default constructor in the
      // initialization list.
    }


    namespace detail
    {
      template <class... OtherBases>
      class ComposePlugin
      {
        public: virtual ~ComposePlugin() = default;
      };

      template <class Base1>
      class ComposePlugin<Base1> : public virtual Base1
      {
        // Declare friendship
        template <class...> friend class SpecializedPluginPtr;
        template <class...> friend class ComposePlugin;

        /// \brief Default destructor
        public: virtual ~ComposePlugin() = default;

        public: ComposePlugin() = default;
      };


      template <class Base1, class Base2>
      class ComposePlugin<Base1, Base2> :
          public virtual Base1,
          public virtual Base2
      {
        // Declare friendship
        template <class...> friend class SpecializedPluginPtr;
        template <class...> friend class ComposePlugin;

        /// \brief Default destructor
        public: virtual ~ComposePlugin() = default;

        // Inherit function overloads
        using TemplatePluginPtr::GetInterface;
        using TemplatePluginPtr::HasInterface;

        // Implement the various functions that need to be dispatched to the
        // base classes.
DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH(T*, GetInterface, (), ())
DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH(const T*, GetInterface, () const, ())
DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH(bool, HasInterface, () const, ())

        public: template<class T>
                static constexpr bool IsSpecializedFor()
        {
          return (Base1::template IsSpecializedFor<T>()
                  || Base2::template IsSpecializedFor<T>());
        }

        public: ComposePlugin() = default;
      };

      template <class Base1, class Base2, class... OtherBases>
      class ComposePlugin<Base1, Base2, OtherBases...> :
          public virtual ComposePlugin<
            Base1, ComposePlugin<Base2, OtherBases...> >
      {
        // Declare friendship
        template <class...> friend class SpecializedPluginPtr;
        template <class...> friend class ComposePlugin;

        /// \brief Default destructor
        public: virtual ~ComposePlugin() = default;

        using Base = ComposePlugin<Base1, ComposePlugin<Base2, OtherBases...>>;

        public: ComposePlugin() = default;
      };
    } // namespace detail

    template <class SpecInterface1, class... OtherSpecInterfaces>
    class SpecializedPluginPtr<SpecInterface1, OtherSpecInterfaces...> :
        public virtual detail::ComposePlugin<
          SpecializedPluginPtr<SpecInterface1>,
          SpecializedPluginPtr<OtherSpecInterfaces...> >
    {
      // Declare friendship
      template <class...> friend class SpecializedPluginPtr;
      template <class...> friend class detail::ComposePlugin;
      friend class PluginLoader;

      using Base = detail::ComposePlugin<
                      SpecializedPluginPtr<SpecInterface1>,
                      SpecializedPluginPtr<OtherSpecInterfaces...> >;

      /// \brief Default constructor
      public: SpecializedPluginPtr() = default;

      DETAIL_IGN_COMMON_PLUGIN_CONSTRUCT_DESTRUCT_ASSIGN(SpecializedPluginPtr)

      private: explicit SpecializedPluginPtr(const PluginInfo *_info)
                 : TemplatePluginPtr(_info),
                   Base()
                {
                  // Do nothing
                }
    };
  }
}

#endif // IGNITION_COMMON_DETAIL_SPECIALIZEDPLUGINPTR_HH_

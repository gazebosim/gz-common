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


#ifndef IGNITION_COMMON_DETAIL_SPECIALIZEDPLUGIN_HH_
#define IGNITION_COMMON_DETAIL_SPECIALIZEDPLUGIN_HH_

#include "ignition/common/SpecializedPlugin.hh"
#include <memory>

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
    // the following is a false positive with cppcheck 1.82 fixed in 1.83
    // cppcheck-suppress syntaxError
    template <class Interface>
    Interface *SpecializedPlugin<SpecInterface>::QueryInterface()
    {
      return this->PrivateQueryInterface(type<Interface>());
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    const Interface *SpecializedPlugin<SpecInterface>::QueryInterface() const
    {
      return this->PrivateQueryInterface(type<Interface>());
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    std::shared_ptr<Interface>
    SpecializedPlugin<SpecInterface>::QueryInterfaceSharedPtr()
    {
      Interface *ptr = this->QueryInterface<Interface>();
      if (ptr)
        return std::shared_ptr<Interface>(this->PrivateGetInstancePtr(), ptr);

      return nullptr;
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    std::shared_ptr<const Interface>
    SpecializedPlugin<SpecInterface>::QueryInterfaceSharedPtr() const
    {
      const Interface *ptr = this->QueryInterface<Interface>();
      if (ptr)
        return std::shared_ptr<Interface>(this->PrivateGetInstancePtr(), ptr);

      return nullptr;
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    bool SpecializedPlugin<SpecInterface>::HasInterface() const
    {
      return this->PrivateHasInterface(type<Interface>());
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    Interface *SpecializedPlugin<SpecInterface>::PrivateQueryInterface(
        type<Interface>)
    {
      return this->Plugin::QueryInterface<Interface>();
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    SpecInterface *SpecializedPlugin<SpecInterface>::PrivateQueryInterface(
        type<SpecInterface>)
    {
      #ifdef IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS
      usedSpecializedInterfaceAccess = true;
      #endif
      return static_cast<SpecInterface*>(
            this->privateSpecializedInterfaceIterator->second);
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    const Interface *SpecializedPlugin<SpecInterface>::
    PrivateQueryInterface(type<Interface>) const
    {
      return this->Plugin::QueryInterface<Interface>();
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    const SpecInterface *SpecializedPlugin<SpecInterface>::
    PrivateQueryInterface(type<SpecInterface>) const
    {
      #ifdef IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS
      usedSpecializedInterfaceAccess = true;
      #endif
      return static_cast<SpecInterface*>(
            this->privateSpecializedInterfaceIterator->second);
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    template <class Interface>
    bool SpecializedPlugin<SpecInterface>::PrivateHasInterface(
        type<Interface>) const
    {
      return this->Plugin::HasInterface<Interface>();
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    bool SpecializedPlugin<SpecInterface>::PrivateHasInterface(
        type<SpecInterface>) const
    {
      #ifdef IGNITION_UNITTEST_SPECIALIZED_PLUGIN_ACCESS
      usedSpecializedInterfaceAccess = true;
      #endif
      return (nullptr != this->privateSpecializedInterfaceIterator->second);
    }

    /////////////////////////////////////////////////
    template <class SpecInterface>
    SpecializedPlugin<SpecInterface>::SpecializedPlugin()
      : privateSpecializedInterfaceIterator(
          this->PrivateGetOrCreateIterator(
            SpecInterface::IGNCOMMONInterfaceName))
    {
      // Do nothing
    }

    namespace detail
    {
      /// \brief This template provides an implementation of
      /// SelectSpecializerIfAvailable by having two template specializations
      /// to choose between at compile time.
      ///
      /// If specialized is true, then this will provide the specializer for
      /// Interface as `Specializer`.
      template <typename Interface, bool specialized>
      struct SelectSpecalizerIfAvailableImpl
      {
        using Specializer = SpecializedPlugin<Interface>;
      };

      /// \brief This template specialization will be invoked when
      /// Specialization is not specialized for Interface, and therefore return
      /// the generic Plugin type.
      template <typename Interface>
      struct SelectSpecalizerIfAvailableImpl<Interface, false>
      {
        using Specializer = Plugin;
      };

      /// \brief If Specialization contains a leaf specializer for Interface,
      /// i.e. SpecializedPlugin<Interface>, then this will provide that type
      /// under the name `Specializer`. Otherwise, this will simply provide the
      /// generic Plugin type.
      template <typename Interface, typename Specialization>
      struct SelectSpecalizerIfAvailable
      {
        using Specializer = typename SelectSpecalizerIfAvailableImpl<Interface,
            std::is_base_of<SpecializedPlugin<Interface>, Specialization>::value
            >::Specializer;
      };

      /// \brief ComposePlugin provides a way for a multi-specialized Plugin
      /// type to find its specializations within itself each time an
      /// interface-querying function is called. The macro
      /// DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH accomplishes this for each
      /// of the different functions by doing a compile-time check on whether
      /// Base2 contains the specialization, and then picks Base1 if it does
      /// not.
      template <class Base1, class Base2>
      class ComposePlugin : public virtual Base1, public virtual Base2
      {
        /// \brief Default destructor
        public: virtual ~ComposePlugin() = default;

        // Inherit function overloads
        using Plugin::QueryInterface;
        using Plugin::QueryInterfaceSharedPtr;
        using Plugin::HasInterface;

        // Used for template metaprogramming
        using Specialization = ComposePlugin<Base1, Base2>;

        /// \brief Implement functions whose only roles are to dispatch their
        /// functionalities between two base classes, depending on which base is
        /// specialized for the template type. This must only be called within
        /// the ComposePlugin class.
        ///
        /// The dispatch is performed by casting this object to the type that
        /// specializes for the requested Interface, if such a type is availabe
        /// within its inheritance structure. Otherwise, we cast to the generic
        /// Plugin type.
        #define DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH( \
                      ReturnType, Function, Suffix, CastTo, Args) \
          public: /* NOLINT(*) */ \
          template <class T> \
          ReturnType Function Suffix \
          { \
            using Specializer = typename detail::SelectSpecalizerIfAvailable< \
                    T, Specialization>::Specializer; \
            return static_cast<CastTo*>(this)->template Function <T> Args; \
          }


        DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH(
            T*, QueryInterface, (), Specializer, ())

        DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH(
            const T*, QueryInterface, () const, const Specializer, ())

        DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH(
            std::shared_ptr<T>, QueryInterfaceSharedPtr, (), Specializer, ())

        DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH(
            std::shared_ptr<const T>, QueryInterfaceSharedPtr,
            () const, const Specializer, ())

        DETAIL_IGN_COMMON_COMPOSEPLUGIN_DISPATCH(
            bool, HasInterface, () const, const Specializer, ())


        // Declare friendship
        template <class...> friend class ignition::common::SpecializedPlugin;
        template <class, class> friend class ComposePlugin;

        private: ComposePlugin() = default;
      };
    }

    /// \brief Construct an unbalanced binary tree of specializations by
    /// convoluting SpecializedPlugin types using ComposePlugin.
    template <class SpecInterface1, class... OtherSpecInterfaces>
    class SpecializedPlugin<SpecInterface1, OtherSpecInterfaces...> :
        public virtual detail::ComposePlugin<
          SpecializedPlugin<SpecInterface1>,
          SpecializedPlugin<OtherSpecInterfaces...> >
    {
      // Declare friendship
      template <class...> friend class SpecializedPlugin;
      template <class, class> friend class detail::ComposePlugin;
      template <class> friend class TemplatePluginPtr;

      /// \brief Virtual destructor
      public: virtual ~SpecializedPlugin() = default;

      /// \brief Default constructor
      private: SpecializedPlugin() = default;
    };
  }
}

#endif

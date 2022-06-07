/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_EVENT_HH_
#define GZ_COMMON_EVENT_HH_

#include <any>
#include <atomic>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include <gz/common/config.hh>
#include <gz/common/events/Export.hh>
#include <gz/common/events/Types.hh>
#include <gz/utils/SuppressWarning.hh>

namespace gz
{
  namespace common
  {
    /// \class Event Event.hh gz/common/Event.hh
    /// \brief Base class for all events
    class GZ_COMMON_EVENTS_VISIBLE Event
    {
      /// \brief Constructor
      public: Event();

      /// \brief Destructor
      // WARNING: This should never be changed to virtual. See the class
      // documentation for EventT. Otherwise, segfaults may occur in downstream
      // libraries.
      public: ~Event();

      /// \brief Disconnect a callback to this event.
      /// \param[in] _id The id of the connection to disconnect.
      public: virtual void Disconnect(int _id);

      /// \brief Get the number of connections.
      /// \return Number of connections.
      unsigned int ConnectionCount() const
      {
        return this->connections.size();
      }

      /// \brief Get whether this event has been signaled.
      /// \return True if the event has been signaled.
      public: bool Signaled() const;

      /// \brief Set whether this event has been signaled.
      /// \param[in] _sig True if the event has been signaled.
      public: void SetSignaled(bool _sig);

      /// \internal
      /// \brief Removes queued connections.
      /// We assume that this function is called from a Signal function.
      protected: void Cleanup();

      /// \brief True if the event has been signaled.
      private: bool signaled;

      /// \brief A private helper class used in maintaining connections.
      protected: class EventConnection
      {
        /// \brief Constructor
       public: EventConnection(bool _on, const std::any &_cb,
                        const ConnectionPtr &_publicConn)
            : callback(_cb), publicConnection(_publicConn)
        {
          // Windows Visual Studio 2012 does not have atomic_bool constructor,
          // so we have to set "on" using operator=
          this->on = _on;
        }

        /// \brief On/off value for the event callback
        public: std::atomic_bool on;

        /// \brief Callback function
        public: std::any callback;

        /// \brief A weak pointer to the Connection pointer returned by Connect.
        /// This is used to clear the Connection's Event pointer during
        /// destruction of an Event.
        public: std::weak_ptr<Connection> publicConnection;
      };
IGN_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \def EvtConnectionMap
      /// \brief Event Connection map typedef.
      protected: typedef std::map<int, std::unique_ptr<EventConnection>>
                    EvtConnectionMap;

      /// \brief Array of connection callbacks.
      protected: EvtConnectionMap connections;

      /// \brief A thread lock.
      private: std::mutex mutex;

      /// \brief List of connections to remove
      private: std::list<typename EvtConnectionMap::const_iterator>
              connectionsToRemove;
IGN_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };

    /// \brief A class that encapsulates a connection.
    class GZ_COMMON_EVENTS_VISIBLE Connection
    {
      /// \brief Constructor.
      /// \param[in] _e Event pointer to connect with.
      /// \param[in] _i Unique id.
      public: Connection(Event *_e, int _i);

      /// \brief Destructor.
      public: ~Connection();

      /// \brief Get the id of this connection.
      /// \return The id of this connection.
      public: int Id() const;

      /// \brief the event for this connection
      private: Event *event = nullptr;

      /// \brief the id set in the constructor
      private: int id = -1;

#ifdef _WIN32
// Disable warning C4251
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
      /// \brief set during the constructor
      private: std::chrono::time_point<std::chrono::system_clock> creationTime;
#ifdef _WIN32
#pragma warning(pop)
#endif

      /// \brief Friend class.
      public: template<typename T, typename N> friend class EventT;
      public: friend class Event;
    };

    /// \brief A class for event processing.
    /// \tparam T function event callback function signature
    /// \tparam N optional additional type to disambiguate events with same
    ///   function signature
    //
    // WARNING: This class should not have any data members. Instead all data
    // members should go into the base class `Event`. This is to ensure that
    // this class behaves as expected when used with shared libraries (plugins)
    // that get unloaded after emitting an event.
    //
    // Explanation: TODO
    //
    template<typename T, typename N = void>
    class EventT final : public Event
    {
      public: using CallbackT = std::function<T>;
      static_assert(std::is_same<typename CallbackT::result_type, void>::value,
          "Event callback must have void return type");

      /// \brief Constructor.
      public: EventT() = default;

      /// \brief Connect a callback to this event.
      /// \param[in] _subscriber Pointer to a callback function.
      /// \return A Connection object, which will automatically call
      /// Disconnect when it goes out of scope.
      public: ConnectionPtr Connect(const CallbackT &_subscriber);

      /// \brief Access the signal.
      public: template<typename ... Args>
              void operator()(Args && ... args)
      {
        this->Signal(std::forward<Args>(args)...);
      }

      /// \brief Signal the event for all subscribers.
      public: template <typename ... Args>
              void Signal(Args && ... args)
      {
        this->Cleanup();

        this->SetSignaled(true);
        for (const auto &iter : this->connections)
        {
          if (iter.second->on)
          {
            auto callback = std::any_cast<CallbackT>(iter.second->callback);
            callback(std::forward<Args>(args)...);
          }
        }
      }
    };

    /// \brief Adds a connection.
    /// \param[in] _subscriber the subscriber to connect.
    template<typename T, typename N>
    ConnectionPtr EventT<T, N>::Connect(const CallbackT &_subscriber)
    {
      int index = 0;
      if (!this->connections.empty())
      {
        auto const &iter = this->connections.rbegin();
        index = iter->first + 1;
      }
      auto connection = ConnectionPtr(new Connection(this, index));
      this->connections[index].reset(
          new EventConnection(true, _subscriber, connection));
      return connection;
    }
  }
}
#endif

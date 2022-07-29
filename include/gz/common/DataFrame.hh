/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_DATAFRAME_HH_
#define GZ_COMMON_DATAFRAME_HH_

#include <array>
#include <numeric>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unordered_map>

#include <gz/common/CSVStreams.hh>
#include <gz/common/Io.hh>

#include <gz/math/TimeVaryingVolumetricGrid.hh>

namespace gz
{
  namespace common
  {
    template <typename K, typename V>
    class DataFrame
    {
      public: bool Has(const K &_key) const
      {
        return this->storage.count(_key) > 0;
      }

      public: V &operator[](const K &_key)
      {
        return this->storage[_key];
      }

      public: const V &operator[](const K &_key) const
      {
        return this->storage.at(_key);
      }

      private: std::unordered_map<K, V> storage;
    };

    template <typename K, typename T, typename V, typename P>
    struct IO<DataFrame<K, math::InMemoryTimeVaryingVolumetricGrid<T, V, P>>>
    {
      static DataFrame<K, math::InMemoryTimeVaryingVolumetricGrid<T, V, P>>
      ReadFrom(CSVIStreamIterator _begin,
               CSVIStreamIterator _end,
               const std::string &_timeColumn,
               const std::array<std::string, 3> &_coordinateColumns)
      {
        if (_begin == _end)
        {
          throw std::invalid_argument("CSV data stream is empty");
        }
        const std::vector<std::string> &header = *_begin;
        if (header.empty())
        {
          throw std::invalid_argument("CSV data stream has no header");
        }

        auto it = std::find(header.begin(), header.end(), _timeColumn);
        if (it == header.end())
        {
          std::stringstream sstream;
          sstream << "CSV data stream has no '"
                  << _timeColumn << "' column";
          throw std::invalid_argument(sstream.str());
        }
        const size_t timeIndex = it - header.begin();

        std::array<size_t, 3> coordinateIndices;
        for (size_t i = 0; i < _coordinateColumns.size(); ++i)
        {
          it = std::find(header.begin(), header.end(), _coordinateColumns[i]);
          if (it == header.end())
          {
            std::stringstream sstream;
            sstream << "CSV data stream has no '"
                    << _coordinateColumns[i] << "' column";
            throw std::invalid_argument(sstream.str());
          }
          coordinateIndices[i] = it - header.begin();
        }

        return ReadFrom(_begin, _end, timeIndex, coordinateIndices);
      }

      static DataFrame<K, math::InMemoryTimeVaryingVolumetricGrid<T, V, P>>
      ReadFrom(CSVIStreamIterator _begin,
               CSVIStreamIterator _end,
               const size_t &_timeIndex = 0,
               const std::array<size_t, 3> &_coordinateIndices = {1, 2, 3})
      {
        if (_begin == _end)
        {
          throw std::invalid_argument("CSV data stream is empty");
        }
        std::vector<size_t> dataIndices(_begin->size());
        std::iota(dataIndices.begin(), dataIndices.end(), 0);
        auto last = dataIndices.end();
        for (size_t index : {_timeIndex, _coordinateIndices[0],
            _coordinateIndices[1], _coordinateIndices[2]})
        {
          auto it = std::find(dataIndices.begin(), last, index);
          if (it == last)
          {
            std::stringstream sstream;
            sstream << "Column index " << index << " is"
                    << "out of range for CSV data stream";
            throw std::invalid_argument(sstream.str());
          }
          *it = *(--last);
        }
        dataIndices.erase(last, dataIndices.end());

        using FactoryT =
            math::InMemoryTimeVaryingVolumetricGridFactory<T, V>;
        std::vector<FactoryT> factories(dataIndices.size());
        for (auto it = _begin; it != _end; ++it)
        {
          const T time = IO<T>::ReadFrom(it->at(_timeIndex));
          const math::Vector3<P> position{
            IO<P>::ReadFrom(it->at(_coordinateIndices[0])),
            IO<P>::ReadFrom(it->at(_coordinateIndices[1])),
            IO<P>::ReadFrom(it->at(_coordinateIndices[2]))};

          for (size_t i = 0; i < dataIndices.size(); ++i)
          {
            const V value = IO<V>::ReadFrom(it->at(dataIndices[i]));
            factories[i].AddPoint(time, position, value);
          }
        }

        DataFrame<K, math::InMemoryTimeVaryingVolumetricGrid<T, V, P>> df;
        for (size_t i = 0; i < dataIndices.size(); ++i)
        {
          const std::string key = !_begin->empty() ?
              _begin->at(dataIndices[i]) :
              "var" + std::to_string(dataIndices[i]);
          df[IO<K>::ReadFrom(key)] = factories[i].Build();
        }
        return df;
      }
    };
  }
}
#endif

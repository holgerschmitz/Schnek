
/*
 * registration.hpp
 *
 * Created on: 20 Nov 2025
 * Author: Holger Schmitz
 * Email: holger@notjustphysics.com
 *
 * Copyright 2025 Holger Schmitz
 *
 * This file is part of Schnek.
 *
 * Schnek is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Schnek is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Schnek.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <memory>

namespace schnek {
    namespace internal {
        class FieldRegistrationInterface {
          public:
            virtual ~FieldRegistrationInterface() {}
        };
    }

    class FieldRegistration {
      private:
        std::shared_ptr<internal::FieldRegistrationInterface> regsitration;
      
    };

    namespace internal {
        template<typename FieldType>
        class FieldRegistrationImpl : public FieldRegistrationInterface {
          private:
            FieldRegistration *owner;
          public:
            FieldRegistrationImpl(const FieldRegistrationImpl &other) default;
            FieldRegistrationImpl &operator=(const FieldRegistrationImpl &other) default;
        };
    } // namespace internal



} // namespace schnek
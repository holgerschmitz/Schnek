#ifndef SCHNEK_DECOMPOSITION_DETAIL_TYPE_INDEXED_VISITOR_HPP_
#define SCHNEK_DECOMPOSITION_DETAIL_TYPE_INDEXED_VISITOR_HPP_

#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "../../util/exceptions.hpp"

namespace schnek {
  namespace internal {

    template<typename Derived, typename... HandlerArgs>
    class TypeIndexedVisitor {
      public:
        template<typename HandlerType>
        void registerHandler() {
          const auto key = std::type_index(typeid(HandlerType));
          dispatchers.emplace(key, [](Derived &visitor, void *handlerPtr, HandlerArgs... args) {
            visitor.template handle<HandlerType>(*static_cast<HandlerType *>(handlerPtr), args...);
          });
        }

        template<typename HandlerType>
        bool hasHandler() const {
          return dispatchers.find(std::type_index(typeid(HandlerType))) != dispatchers.end();
        }

        bool empty() const { return dispatchers.empty(); }

      protected:
        void dispatch(
            const std::type_info &type, void *handlerPtr, const char *missingHandlerMessage, HandlerArgs... args
        ) {
          const auto it = dispatchers.find(std::type_index(type));
          if (it == dispatchers.end()) {
            SCHNECK_FAIL(missingHandlerMessage << type.name());
          }
          it->second(static_cast<Derived &>(*this), handlerPtr, args...);
        }

      private:
        using Dispatcher = std::function<void(Derived &, void *, HandlerArgs...)>;
        std::unordered_map<std::type_index, Dispatcher> dispatchers;
    };

  }  // namespace internal
}  // namespace schnek

#endif  // SCHNEK_DECOMPOSITION_DETAIL_TYPE_INDEXED_VISITOR_HPP_

#pragma once

#include <any>
#include <functional>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace nmhit
{

class Node; // forward declaration — TypeRegistry.h must not include Node.h

// ─── type trait ──────────────────────────────────────────────────────────────

/// Detect whether T is a std::vector<U> for any U.
template <typename T>
struct _is_vec : std::false_type
{};
template <typename T>
struct _is_vec<std::vector<T>> : std::true_type
{};

// ─── TypeRegistry ─────────────────────────────────────────────────────────────

/// Runtime registry for scalar type parsers.
///
/// Built-in types (bool, int, unsigned int, int64_t, float, double, std::string)
/// are pre-registered at startup.  Attempting to register them again throws hit::Error.
///
/// Usage (context-unaware, most common):
///   hit::TypeRegistry::register_parser<MyEnum>(
///     [](const std::string & s) -> MyEnum { ... });
///
/// Usage (context-aware, enables source-location in error messages):
///   hit::TypeRegistry::register_parser<MyEnum>(
///     [](const std::string & s, const hit::Node * n) -> MyEnum { ... });
///
/// After registration, param<MyEnum>(), param<std::vector<MyEnum>>(), and
/// param<std::vector<std::vector<MyEnum>>>() all work automatically.
///
/// Thread safety: the Meyers-singleton map is initialised once (thread-safe in
/// C++11).  Concurrent reads after initialisation are safe.  Do NOT call
/// register_parser() concurrently with param() calls.
class TypeRegistry
{
public:
  /// Register a scalar parser for type T (with source-location context).
  /// The callable receives the raw, unquoted, brace-expanded token string and
  /// an optional Node pointer for source-location-enriched error messages.
  /// Throws hit::Error if T is already registered.
  template <typename T>
  static void
  register_parser(std::function<T(const std::string &, const Node *)> fn)
  {
    auto & m = _store();
    auto key = std::type_index(typeid(T));
    if (m.count(key))
      _throw_already_registered(typeid(T).name());
    m[key] = [fn](const std::string & s, const Node * n) -> std::any { return fn(s, n); };
  }

  /// Register a scalar parser for type T (context-unaware convenience overload).
  /// The source-location context is silently dropped; use the two-argument overload
  /// if you need to attach file/line information to parse errors.
  template <typename T>
  static void
  register_parser(std::function<T(const std::string &)> fn)
  {
    register_parser<T>([fn](const std::string & s, const Node *) { return fn(s); });
  }

  /// Parse raw string as T using the registered parser.
  /// ctx (optional) is passed to the parser for source-location-enriched errors.
  /// Throws hit::Error if T has not been registered.
  template <typename T>
  static T
  dispatch(const std::string & raw, const Node * ctx = nullptr)
  {
    auto & m = _store();
    auto it = m.find(std::type_index(typeid(T)));
    if (it == m.end())
      _throw_unregistered(typeid(T).name());
    return std::any_cast<T>(it->second(raw, ctx));
  }

private:
  /// Throws hit::Error — defined in Node.cpp where hit::Error is available.
  static void _throw_unregistered(const char * type_name);
  static void _throw_already_registered(const char * type_name);

  /// Meyers singleton map — defined in Node.cpp (pre-populated with built-in parsers).
  static std::unordered_map<std::type_index,
                            std::function<std::any(const std::string &, const Node *)>> &
  _store();
};

} // namespace nmhit

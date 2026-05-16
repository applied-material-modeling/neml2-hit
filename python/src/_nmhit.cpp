#include <nanobind/nanobind.h>
#include <nanobind/stl/filesystem.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/unique_ptr.h>
#include <nanobind/stl/vector.h>

#include "nmhit/nmhit.h"

// GCC incorrectly reports Node and its subclasses as copy/move-constructible
// because of a quirk with the protected `= default` copy constructor in Node
// combined with GCC's implicit-move-constructor fall-through rules.
// The actual instantiation fails (vector<unique_ptr<Node>> is non-copyable).
//
// Fix for copy: use nanobind's specializable detail::is_copy_constructible.
// Fix for move: specialize std::is_move_constructible (allowed for user types).

namespace nanobind::detail
{
template <> struct is_copy_constructible<nmhit::Node>    : std::false_type {};
template <> struct is_copy_constructible<nmhit::Root>    : std::false_type {};
template <> struct is_copy_constructible<nmhit::Section> : std::false_type {};
template <> struct is_copy_constructible<nmhit::Field>   : std::false_type {};
template <> struct is_copy_constructible<nmhit::Comment> : std::false_type {};
template <> struct is_copy_constructible<nmhit::Blank>   : std::false_type {};
} // namespace nanobind::detail

namespace std
{
template <> struct is_move_constructible<nmhit::Node>    : false_type {};
template <> struct is_move_constructible<nmhit::Root>    : false_type {};
template <> struct is_move_constructible<nmhit::Section> : false_type {};
template <> struct is_move_constructible<nmhit::Field>   : false_type {};
template <> struct is_move_constructible<nmhit::Comment> : false_type {};
template <> struct is_move_constructible<nmhit::Blank>   : false_type {};
} // namespace std

namespace nb = nanobind;
using namespace nmhit;

// Holds the Python nmhit.Error class, used by the custom exception translator
// to attach a .messages attribute on every raised exception instance.
static PyObject * _error_type = nullptr;

NB_MODULE(_nmhit, m)
{
  m.doc() = "nmhit: NEML2-flavored HIT parser";

  // ── ErrorMessage ────────────────────────────────────────────────────────────

  nb::class_<ErrorMessage>(m, "ErrorMessage")
    .def_ro("line",    &ErrorMessage::line)
    .def_ro("column",  &ErrorMessage::column)
    .def_ro("message", &ErrorMessage::message)
    .def_prop_ro("filename",
                 [](const ErrorMessage & e) -> std::filesystem::path { return e.filename; })
    .def("__str__",  &ErrorMessage::str)
    .def("__repr__", [](const ErrorMessage & e) { return "<ErrorMessage " + e.str() + ">"; });

  // ── Error exception ─────────────────────────────────────────────────────────
  //
  // nb::exception<T> creates a Python exception class and registers a default
  // translator (raises with what()). We register an additional translator
  // (LIFO → tried first) that also attaches the structured .messages list.

  {
    auto exc = nb::exception<nmhit::Error>(m, "Error", PyExc_RuntimeError);
    _error_type = exc.ptr();
    Py_INCREF(_error_type);
  }

  nb::register_exception_translator([](const std::exception_ptr & ep, void *) {
    try
    {
      std::rethrow_exception(ep);
    }
    catch (const nmhit::Error & e)
    {
      if (!_error_type)
        return;
      PyObject * msg_str = PyUnicode_FromString(e.what());
      PyObject * args    = PyTuple_New(1);
      PyTuple_SetItem(args, 0, msg_str); // steals ref to msg_str
      PyObject * inst = PyObject_Call(_error_type, args, nullptr);
      Py_DECREF(args);
      if (inst)
      {
        PyObject * msgs = PyList_New(0);
        for (auto msg : e.messages) // copy by value for move into cast
        {
          nb::object py_msg = nb::cast(std::move(msg));
          PyList_Append(msgs, py_msg.ptr());
        }
        PyObject_SetAttrString(inst, "messages", msgs);
        Py_DECREF(msgs);
        PyErr_SetObject(_error_type, inst);
        Py_DECREF(inst);
      }
    }
  });

  // ── NodeType enum ───────────────────────────────────────────────────────────

  nb::enum_<NodeType>(m, "NodeType")
    .value("All",     NodeType::All)
    .value("Root",    NodeType::Root)
    .value("Section", NodeType::Section)
    .value("Field",   NodeType::Field)
    .value("Comment", NodeType::Comment)
    .value("Blank",   NodeType::Blank)
    .export_values();

  // ── Node base class ─────────────────────────────────────────────────────────
  //
  // Tree-navigation methods that return raw Node* use rv_policy::reference_internal
  // with an explicit parent handle so nanobind keeps `self` alive as long as
  // any returned child object is alive.  This prevents use-after-free when the
  // root Python wrapper is dropped while a child reference is still held.

  nb::class_<Node>(m, "Node")
    // identity
    .def("type",          &Node::type)
    .def("path",          &Node::path)
    .def("fullpath",      &Node::fullpath)
    // source location
    .def("line",          &Node::line)
    .def("column",        &Node::column)
    .def_prop_ro("filename",
                 [](const Node & n) -> std::filesystem::path { return n.filename(); })
    .def("file_location", &Node::file_location)
    // tree navigation
    .def("parent",
         [](nb::object self) -> nb::object {
           Node * p = nb::cast<Node &>(self).parent();
           if (!p)
             return nb::none();
           return nb::cast(p, nb::rv_policy::reference_internal, self);
         })
    .def("root_node",
         [](nb::object self) -> nb::object {
           Node * r = nb::cast<Node &>(self).root();
           if (!r)
             return nb::none();
           return nb::cast(r, nb::rv_policy::reference_internal, self);
         })
    .def("children",
         [](nb::object self, NodeType filter) -> nb::list {
           nb::list result;
           for (Node * child : nb::cast<Node &>(self).children(filter))
             result.append(nb::cast(child, nb::rv_policy::reference_internal, self));
           return result;
         },
         nb::arg("filter") = NodeType::All)
    .def("find",
         [](nb::object self, const std::string & relpath) -> nb::object {
           Node * found = nb::cast<Node &>(self).find(relpath);
           if (!found)
             return nb::none();
           return nb::cast(found, nb::rv_policy::reference_internal, self);
         },
         nb::arg("relpath"))
    // typed param accessors
    .def("param_bool",
         [](const Node & n, const std::string & p) { return n.param<bool>(p); },
         nb::arg("relpath") = "")
    .def("param_int",
         [](const Node & n, const std::string & p) { return n.param<int64_t>(p); },
         nb::arg("relpath") = "")
    .def("param_float",
         [](const Node & n, const std::string & p) { return n.param<double>(p); },
         nb::arg("relpath") = "")
    .def("param_str",
         [](const Node & n, const std::string & p) { return n.param<std::string>(p); },
         nb::arg("relpath") = "")
    .def("param_list_bool",
         [](const Node & n, const std::string & p) { return n.param<std::vector<bool>>(p); },
         nb::arg("relpath") = "")
    .def("param_list_int",
         [](const Node & n, const std::string & p) { return n.param<std::vector<int64_t>>(p); },
         nb::arg("relpath") = "")
    .def("param_list_float",
         [](const Node & n, const std::string & p) { return n.param<std::vector<double>>(p); },
         nb::arg("relpath") = "")
    .def("param_list_str",
         [](const Node & n, const std::string & p) { return n.param<std::vector<std::string>>(p); },
         nb::arg("relpath") = "")
    .def("param_list_list_int",
         [](const Node & n, const std::string & p) {
           return n.param<std::vector<std::vector<int64_t>>>(p);
         },
         nb::arg("relpath") = "")
    .def("param_list_list_float",
         [](const Node & n, const std::string & p) {
           return n.param<std::vector<std::vector<double>>>(p);
         },
         nb::arg("relpath") = "")
    .def("param_list_list_str",
         [](const Node & n, const std::string & p) {
           return n.param<std::vector<std::vector<std::string>>>(p);
         },
         nb::arg("relpath") = "")
    // optional variants
    .def("param_optional_bool",
         [](const Node & n, const std::string & p, bool d) {
           return n.param_optional<bool>(p, d);
         },
         nb::arg("relpath"),
         nb::arg("default_val"))
    .def("param_optional_int",
         [](const Node & n, const std::string & p, int64_t d) {
           return n.param_optional<int64_t>(p, d);
         },
         nb::arg("relpath"),
         nb::arg("default_val"))
    .def("param_optional_float",
         [](const Node & n, const std::string & p, double d) {
           return n.param_optional<double>(p, d);
         },
         nb::arg("relpath"),
         nb::arg("default_val"))
    .def("param_optional_str",
         [](const Node & n, const std::string & p, std::string d) {
           return n.param_optional<std::string>(p, std::move(d));
         },
         nb::arg("relpath"),
         nb::arg("default_val"))
    // mutation
    //
    // Python-constructed nodes (e.g. Field("k","1")) are embedded inside their
    // Python allocation and cannot be consumed via unique_ptr ownership transfer.
    // These methods therefore clone the supplied node into a C++-owned unique_ptr
    // and add the clone to the tree.  The caller's Python object is unchanged.
    .def("add_child",
         [](Node & parent, const Node & child) { parent.add_child(child.clone()); },
         nb::arg("child"))
    .def("insert_child",
         [](Node & parent, std::size_t idx, const Node & child) {
           parent.insert_child(idx, child.clone());
         },
         nb::arg("idx"),
         nb::arg("child"))
    // remove_child accepts a slash-separated relative path.  The lookup is done
    // through C++ (not the Python find() method) to avoid the nanobind instance
    // cache, which would otherwise conflict with transferring ownership back to
    // the caller as a unique_ptr.  Returns the removed node as a new Python
    // object; the caller is responsible for keeping or discarding it.
    .def("remove_child",
         [](Node & parent, const std::string & relpath) -> std::unique_ptr<Node> {
           Node * child = parent.find(relpath);
           if (!child)
             throw nmhit::Error("remove_child: no node at path '" + relpath + "'");
           return parent.remove_child(child);
         },
         nb::arg("relpath"))
    // render / clone
    .def("render",   &Node::render, nb::arg("indent") = 0, nb::arg("indent_text") = "  ")
    .def("clone",    &Node::clone)
    .def("__repr__", [](const Node & n) { return "<Node path=" + n.fullpath() + ">"; });

  // ── Concrete node types ─────────────────────────────────────────────────────
  //
  // Must be registered as subclasses of Node so that raw Node* returned by
  // find() / children() are dispatched to the correct Python subtype at runtime.

  nb::class_<Root, Node>(m, "Root")
    .def(nb::init<>())
    .def("__repr__", [](const Root &) { return "<Root>"; });

  nb::class_<Section, Node>(m, "Section")
    .def(nb::init<const std::string &>(), nb::arg("name"))
    .def("__repr__", [](const Section & s) { return "<Section " + s.path() + ">"; });

  nb::class_<Field, Node>(m, "Field")
    .def(nb::init<const std::string &, const std::string &>(), nb::arg("name"), nb::arg("raw_value"))
    .def("raw_val", &Field::raw_val)
    .def("set_val", &Field::set_val, nb::arg("value"))
    .def("__repr__", [](const Field & f) { return "<Field " + f.path() + "=" + f.raw_val() + ">"; });

  nb::class_<Comment, Node>(m, "Comment")
    .def(nb::init<const std::string &, bool>(), nb::arg("text"), nb::arg("is_inline") = false)
    .def("text",       &Comment::text)
    .def("is_inline",  &Comment::is_inline)
    .def("set_inline", &Comment::set_inline, nb::arg("v"))
    .def("__repr__", [](const Comment & c) { return "<Comment " + c.text() + ">"; });

  nb::class_<Blank, Node>(m, "Blank")
    .def(nb::init<>())
    .def("__repr__", [](const Blank &) { return "<Blank>"; });

  // ── Free functions ──────────────────────────────────────────────────────────

  m.def("parse_file",
        &nmhit::parse_file,
        nb::arg("fname"),
        nb::arg("pre")  = std::vector<std::string>{},
        nb::arg("post") = std::vector<std::string>{});

  m.def("parse_text",
        &nmhit::parse_text,
        nb::arg("input"),
        nb::arg("pre")  = std::vector<std::string>{},
        nb::arg("post") = std::vector<std::string>{});

  m.def("parse_bool",
        [](const std::string & s, const Node * ctx) { return nmhit::parse_bool(s, ctx); },
        nb::arg("s"),
        nb::arg("ctx") = static_cast<const Node *>(nullptr));

  m.def("parse_int",
        [](const std::string & s, const Node * ctx) { return nmhit::parse_int(s, ctx); },
        nb::arg("s"),
        nb::arg("ctx") = static_cast<const Node *>(nullptr));

  m.def("parse_double",
        [](const std::string & s, const Node * ctx) { return nmhit::parse_double(s, ctx); },
        nb::arg("s"),
        nb::arg("ctx") = static_cast<const Node *>(nullptr));

  m.def("parse_float",
        [](const std::string & s, const Node * ctx) { return nmhit::parse_float(s, ctx); },
        nb::arg("s"),
        nb::arg("ctx") = static_cast<const Node *>(nullptr));
}

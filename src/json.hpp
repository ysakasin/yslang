#pragma once

#include <cassert>
#include <sstream>
#include <string>
#include <vector>

namespace yslang {

class json_array;
class json_object;

class json {
public:
  static json array() {
    return json(new json_array());
  }

public:
  using json_array = std::vector<json>;
  using json_object = std::vector<std::pair<std::string, json>>;

  json() {
    kind = Kind::Nothing;
    element.array = nullptr;
  }

  json(int64_t number) {
    kind = Kind::Number;
    element.number = number;
  }

  json(const std::string &str) {
    kind = Kind::String;
    element.string = new std::string(str);
  }

  json(const json &j) {
    copy_from(j);
  }

  json(json_array *array) {
    assert(array != nullptr);
    kind = Kind::Array;
    element.array = array;
  }

  ~json() {
    safe_delete();
  }

  int64_t operator=(int64_t number) {
    safe_delete();
    kind = Kind::Number;
    return element.number = number;
  }

  std::string operator=(const std::string &str) {
    safe_delete();
    kind = Kind::String;
    element.string = new std::string(str);
    return *element.string;
  }

  json operator=(const json &j) {
    safe_delete();
    copy_from(j);
    return j;
  }

  json &operator[](int64_t i) {
    if (kind == Kind::Array) {
      throw "hoge";
    }
    return (*element.array)[i];
  }

  json &operator[](std::string key) {
    if (kind == Kind::Nothing) {
      kind = Kind::Object;
      element.object = new json_object();
    } else if (kind != Kind::Object) {
      throw "hoge";
    }

    for (auto &item : *element.object) {
      if (item.first == key) {
        return item.second;
      }
    }

    element.object->emplace_back(key, json());
    return element.object->back().second;
  }

  void push_back(int64_t number) {
    if (kind != Kind::Array) {
      throw "hoge";
    }
    assert(element.array != nullptr);
    return element.array->emplace_back(number);
  }

  void push_back(const std::string &str) {
    if (kind != Kind::Array) {
      throw "hoge";
    }
    assert(element.array != nullptr);
    element.array->emplace_back(str);
  }

  void push_back(const json &j) {
    if (kind != Kind::Array) {
      throw "hoge";
    }
    assert(element.array != nullptr);
    element.array->push_back(j);
  }

  int64_t get_number() const {
    if (kind != Kind::Number) {
      return 0;
    }

    return element.number;
  }

  std::string get_string() const {
    if (kind != Kind::String) {
      return "";
    }

    return *element.string;
  }

  std::string to_string() const {
    if (kind == Kind::Array) {
      return dump_array();
    } else if (kind == Kind::Object) {
      return dump_object();
    } else if (kind == Kind::String) {
      return dump_string();
    } else {
      return dump_number();
    }
  }

  void init_array() {
    kind = Kind::Array;
    element.array = new json_array();
    assert(element.array != nullptr);
  }

private:
  void safe_delete() {
    if (kind == Kind::Array) {
      delete element.array;
    } else if (kind == Kind::Object) {
      delete element.object;
    } else if (kind == Kind::String) {
      delete element.string;
    }

    kind = Kind::Nothing;
    element.array = nullptr;
  }

  void copy_from(const json &dest) {
    safe_delete();

    kind = dest.kind;

    if (kind == Kind::Array) {
      element.array = new json_array(*dest.element.array);
    } else if (kind == Kind::Object) {
      element.object = new json_object(*dest.element.object);
    } else if (kind == Kind::String) {
      element.string = new std::string(*dest.element.string);
    } else {
      element = dest.element;
    }
  }

  std::string dump_number() const {
    return std::to_string(element.number);
  }

  std::string dump_array() const {
    std::vector<std::string> v;
    v.reserve(element.array->size());

    for (const auto &item : *element.array) {
      v.emplace_back(item.to_string());
    }

    std::stringstream ss;
    return "[" + join(v) + "]";
  }

  std::string dump_object() const {
    std::vector<std::string> v;
    v.reserve(element.object->size());

    for (const auto &item : *element.object) {
      v.emplace_back(escapeJsonString(item.first) + ": " +
                     item.second.to_string());
    }

    std::stringstream ss;
    return "{" + join(v) + "}";
  }

  std::string dump_string() const {
    return escapeJsonString(*element.string);
  }

  static std::string escapeJsonString(const std::string &input) {
    std::ostringstream ss;

    ss << '"';

    for (auto iter = input.cbegin(); iter != input.cend(); iter++) {
      switch (*iter) {
      case '\\':
        ss << "\\\\";
        break;
      case '"':
        ss << "\\\"";
        break;
      case '/':
        ss << "\\/";
        break;
      case '\b':
        ss << "\\b";
        break;
      case '\f':
        ss << "\\f";
        break;
      case '\n':
        ss << "\\n";
        break;
      case '\r':
        ss << "\\r";
        break;
      case '\t':
        ss << "\\t";
        break;
      default:
        ss << *iter;
        break;
      }
    }

    ss << '"';

    return ss.str();
  }

  std::string join(std::vector<std::string> v) const {
    if (v.size() == 0) {
      return "";
    }

    std::stringstream ss;
    ss << v[0];

    for (int i = 1; i < v.size(); i++) {
      ss << ", " << v[i];
    }

    return ss.str();
  }

private:
  enum class Kind {
    Array,
    Object,
    String,
    Number,
    Nothing,
  };

  union Element {
    json_array *array;
    json_object *object;
    std::string *string;
    int64_t number;
  };

  Kind kind = Kind::Nothing;
  Element element;
};
} // namespace yslang

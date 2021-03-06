#include <bunsan/utility/executor.hpp>

#include <bunsan/log/trivial.hpp>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

namespace bunsan {
namespace utility {

executor::executor(const std::string &command)
    : positional(0), next_positional(0) {
  BUNSAN_LOG_TRACE << "Creating executor object from \"" << command << "\"";
  arguments.push_back(string(1, token(command)));
}

executor::executor(const boost::filesystem::path &command)
    : executor(command.string()) {}
executor::executor(const char *const command)
    : executor(std::string(command)) {}

void executor::process(
    string &arg, const boost::property_tree::ptree::value_type &arg_value) {
  token tk;
  if (arg_value.first == "t" || arg_value.first == "text") {
    std::string txt = arg_value.second.get_value<std::string>();
    tk = txt;
    BUNSAN_LOG_TRACE << "text type \"" << txt << "\"";
  } else if (arg_value.first == "p" || arg_value.first == "positional") {
    size_t ph = arg_value.second.get_value<size_t>();
    BUNSAN_LOG_TRACE << "positional placeholder \"" << ph << "\"";
    reference pos = ph;
    if (positional.size() <= ph) positional.resize(ph + 1);
    tk = pos;
  } else if (arg_value.first == "n" || arg_value.first == "named") {
    std::string ph = arg_value.second.get_value<std::string>();
    BUNSAN_LOG_TRACE << "named placeholder \"" << ph << "\"";
    reference nmd = ph;
    tk = nmd;
  } else {
    BUNSAN_LOG_TRACE << "unknown type \"" << arg_value.first << "\"";
    BOOST_THROW_EXCEPTION(
        executor_unknown_placeholder_type_error()
        << executor_unknown_placeholder_type_error::type(arg_value.first));
  }
  arg.push_back(tk);
}

executor::executor(const boost::property_tree::ptree &command)
    : next_positional(0) {
  BUNSAN_LOG_TRACE
      << "creating executor object from boost::property_tree::ptree";
  for (const auto &arg_value : command) {
    string arg;
    if (arg_value.first == "c" || arg_value.first == "complex") {
      BUNSAN_LOG_TRACE << "complex type";
      for (const auto &arg_subvalue : arg_value.second) {
        process(arg, arg_subvalue);
      }
      BUNSAN_LOG_TRACE << "end of complex type";
    } else if (arg_value.first == "d" || arg_value.first == "definition") {
      for (const auto &arg_subvalue : arg_value.second) {
        named[arg_subvalue.first] =
            arg_subvalue.second.get_value<std::string>();
      }
      continue;
    } else {
      process(arg, arg_value);
    }
    arguments.push_back(arg);
  }
  BUNSAN_LOG_TRACE << "created";
}

void executor::operator()() const {
  BUNSAN_LOG_TRACE << "trying to execute";
  bunsan::process::check_sync_execute(context());
}

executor &executor::add_argument(const std::string &arg) {
  BUNSAN_LOG_TRACE << "adding argument \"" << arg << "\"";
  if (next_positional < positional.size())
    positional[next_positional++] = arg;
  else
    arguments.push_back(string(1, token(arg)));
  return *this;
}

executor &executor::set_argument(const std::string &key,
                                 const std::string &arg) {
  BUNSAN_LOG_TRACE << "setting named argument \"" << key << "\" to \"" << arg
                   << "\"";
  named[key] = arg;
  return *this;
}

executor &executor::current_path(const boost::filesystem::path &cwd) {
  BUNSAN_LOG_TRACE << "setting cwd to " << cwd;
  named[current_path_key] = cwd.string();
  return *this;
}

executor &executor::executable(const boost::filesystem::path &exec_) {
  BUNSAN_LOG_TRACE << "setting executable to " << exec_;
  named[executable_key] = exec_.string();
  return *this;
}

class executor::ref_visitor : public boost::static_visitor<std::string> {
 public:
  ref_visitor(const std::vector<string_opt> *positional_, const dict *named_)
      : positional(positional_), named(named_) {}
  std::string operator()(const std::string &key) {
    BUNSAN_LOG_TRACE << "named: returning std::string=\"" << named->at(key)
                     << "\" from std::string=\"" << key << "\"";
    return named->at(key);
  }
  std::string operator()(size_t index) {
    BUNSAN_LOG_TRACE << "positional: returning std::string=\""
                     << positional->at(index).get() << "\" from size_t=\""
                     << index << "\"";
    return positional->at(index).get();
  }

 private:
  const std::vector<string_opt> *positional;
  const dict *named;
};

class executor::token_visitor : public boost::static_visitor<std::string> {
 public:
  token_visitor(const std::vector<string_opt> *positional_, const dict *named_)
      : positional(positional_), named(named_), rvisitor(positional_, named_) {}
  std::string operator()(const std::string &str) {
    BUNSAN_LOG_TRACE << "text: returning std::string from std::string=\"" << str
                     << "\"";
    return str;
  }
  std::string operator()(const reference &ref) {
    return boost::apply_visitor(rvisitor, ref);
  }

 private:
  const std::vector<string_opt> *positional;
  const dict *named;
  ref_visitor rvisitor;
};

void executor::prepare(std::vector<std::string> &args,
                       token_visitor &visitor) const {
  for (size_t i = 0; i < arguments.size(); ++i) {
    args[i].clear();
    for (const auto &j : arguments[i]) {
      args[i] += boost::apply_visitor(visitor, j);
    }
  }
}

bunsan::process::context executor::context() const {
  BUNSAN_LOG_TRACE << "trying to create context";
  std::vector<std::string> args(arguments.size());
  token_visitor visitor(&positional, &named);
  prepare(args, visitor);
  bunsan::process::context ctx;
  ctx.arguments(args);
  auto iter = named.find(executable_key);
  if (iter != named.end()) ctx.executable(iter->second);
  iter = named.find(current_path_key);
  if (iter != named.end()) ctx.current_path(iter->second);
  iter = named.find(use_path_key);
  if (iter != named.end())
    ctx.use_path(iter->second == "true" || iter->second == "yes" ||
                 iter->second == "1");  // TODO bad code
  return ctx;
}

int executor::sync() const {
  BUNSAN_LOG_TRACE << "trying to execute";
  return bunsan::process::sync_execute(context());
}

}  // namespace utility
}  // namespace bunsan

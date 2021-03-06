#pragma once

#include <bunsan/utility/error.hpp>

#include <bunsan/process/execute.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/variant.hpp>

#include <string>
#include <unordered_map>

namespace bunsan {
namespace utility {

struct executor_error : virtual error {};

struct executor_unknown_placeholder_type_error : virtual executor_error {
  using type = boost::error_info<struct tag_type, std::string>;
};

class executor {
 public:
  explicit executor(const std::string &command);
  explicit executor(const boost::filesystem::path &command);
  explicit executor(const char *const command);
  explicit executor(const boost::property_tree::ptree &command);
  executor(const executor &) = default;
  executor(executor &&) = default;
  executor &operator=(const executor &) = default;
  executor &operator=(executor &&) = default;

  /*!
   * \brief run command with specified arguments
   * \throws bunsan::process::non_zero_exit_status
   */
  void operator()() const;

  /*!
   * \return executing context
   */
  bunsan::process::context context() const;

  /*!
   * \brief run command synchronous with specified arguments
   * \return return code
   */
  int sync() const;

  /*!
   * \brief add positional argument
   * \return *this
   */
  executor &add_argument(const std::string &arg);

  executor &add_argument(const boost::filesystem::path &arg) {
    return add_argument(arg.string());
  }

  executor &add_argument(const char *arg) {
    return add_argument(std::string(arg));
  }

  /*!
   * \brief set named argument
   */
  executor &set_argument(const std::string &key, const std::string &arg);

  executor &set_argument(const std::string &key,
                         const boost::filesystem::path &arg) {
    return set_argument(key, arg.string());
  }

  executor &set_argument(const std::string &key, const char *arg) {
    return set_argument(key, std::string(arg));
  }

  /*!
   * \brief set current work directory for process
   */
  executor &current_path(const boost::filesystem::path &cwd);

  /*!
   * \brief set executable file (without 0 argument will be used)
   */
  executor &executable(const boost::filesystem::path &exec_);

  template <typename T1, typename T2, typename... Args>
  executor &add_argument(const T1 &arg1, const T2 &arg2, Args &&... args) {
    add_argument(arg1);
    return add_argument(arg2, std::forward<Args>(args)...);
  }

  template <typename... Args>
  int sync(Args &&... args) const {
    executor tmp(*this);
    return tmp.add_argument(std::forward<Args>(args)...).sync();
  }

  template <typename... Args>
  void operator()(Args &&... args) const {
    executor tmp(*this);
    tmp.add_argument(std::forward<Args>(args)...)();
  }

  template <typename T, typename... Args>
  static int run_sync_from(const boost::filesystem::path &cwd, const T &command,
                           Args &&... args) {
    executor exc(command);
    return exc.current_path(cwd).sync(std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  static int run_sync(const T &command, Args &&... args) {
    executor exc(command);
    return exc.sync(std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  static void exec_from(const boost::filesystem::path &cwd, const T &command,
                        Args &&... args) {
    executor exc(command);
    exc.current_path(cwd)(std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  static void exec(const T &command, Args &&... args) {
    executor exc(command);
    exc(std::forward<Args>(args)...);
  }

 public:
  static constexpr const char *current_path_key = "current_path";
  static constexpr const char *executable_key = "executable";
  static constexpr const char *use_path_key = "use_path";
  using reference = boost::variant<size_t, std::string>;
  using token = boost::variant<reference, std::string>;
  using string = std::vector<token>;
  using string_opt = boost::optional<std::string>;
  using dict = std::unordered_map<std::string, std::string>;

 private:
  std::vector<string> arguments;
  std::vector<string_opt> positional;
  size_t next_positional;
  dict named;
  class token_visitor;
  class ref_visitor;
  void prepare(std::vector<std::string> &args, token_visitor &visitor) const;
  void process(string &arg,
               const boost::property_tree::ptree::value_type &arg_value);
};

}  // namespace utility
}  // namespace bunsan

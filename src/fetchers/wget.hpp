#pragma once

#include <bunsan/utility/fetcher.hpp>

namespace bunsan {
namespace utility {
namespace fetchers {

class wget : public fetcher {
 public:
  explicit wget(const boost::filesystem::path &exe);

  void fetch(const std::string &uri,
             const boost::filesystem::path &dst) override;

 private:
  const boost::filesystem::path m_exe;
};

}  // namespace fetchers
}  // namespace utility
}  // namespace bunsan

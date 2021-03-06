#pragma once

#include <bunsan/utility/fetcher.hpp>

namespace bunsan {
namespace utility {
namespace fetchers {

class copy : public fetcher {
 public:
  copy() = default;
  void fetch(const std::string &uri,
             const boost::filesystem::path &dst) override;
};

}  // namespace fetchers
}  // namespace utility
}  // namespace bunsan

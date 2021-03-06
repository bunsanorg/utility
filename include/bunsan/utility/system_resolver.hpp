#pragma once

#include <bunsan/utility/resolver.hpp>

namespace bunsan {
namespace utility {

class system_resolver : public resolver {
 public:
  system_resolver() = default;

  boost::filesystem::path find_executable(
      const boost::filesystem::path &exe) override;

  boost::filesystem::path find_library(
      const boost::filesystem::path &lib) override;

  std::unique_ptr<resolver> clone() const override;
};

}  // namespace utility
}  // namespace bunsan

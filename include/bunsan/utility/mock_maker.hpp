#pragma once

#include <bunsan/utility/maker.hpp>

#include <turtle/mock.hpp>

namespace bunsan {
namespace utility {

MOCK_BASE_CLASS(mock_maker, maker) {
  MOCK_METHOD(exec, 2, void(const boost::filesystem::path &cwd,
                            const std::vector<std::string> &targets))
};

}  // namespace utility
}  // namespace bunsan

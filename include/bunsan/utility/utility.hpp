#pragma once

#include <bunsan/utility/error.hpp>

#include <boost/property_tree/ptree.hpp>

namespace bunsan {
namespace utility {

/*!
 * \brief Abstract class that specifies the way
 * of options transmission + virtual destructor
 */
class utility {
 public:
  /*!
   * \brief Set up utility configuration.
   *
   * You can find options documentation in utility docs.
   *
   * \throws error if !ptree.empty()
   */
  virtual void setup(const boost::property_tree::ptree &ptree);

  /*!
   * \brief Does nothing
   *
   * Utility can have own destructor,
   * we will provide such virtual destructor support
   */
  virtual ~utility();
};

}  // namespace utility
}  // namespace bunsan

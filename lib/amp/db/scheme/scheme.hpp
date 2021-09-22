#pragma once

#include <amp/db/pqxx_fix.hpp>
#include <pqxx/pqxx>

namespace amp
{
namespace db
{
namespace scheme
{
class Scheme
{
  public:
    // make table scheme classes inherit this function for transformation from rows in pqxx format
    virtual void fromRow(pqxx::row &row) = 0;
};
} // namespace scheme
} // namespace db
} // namespace amp

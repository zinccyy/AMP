#pragma once

#include <amp/context.hpp>

#include <memory>

namespace amp
{
namespace rpc
{

namespace service
{
class ServiceData
{
  protected:
    std::shared_ptr<amp::Context> mContext;
};
}
} // namespace rpc
}
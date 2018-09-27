#pragma once

#include "Application.hpp"
#include "Loader.hpp"
#include "Controller.hpp"
#include "api/SwitchFwd.hpp"

namespace runos {

using SwitchPtr = safe::shared_ptr<Switch>;

/*namespace matches {
    using eth_type = of13::EthType;
    using ip_proto = of13::IPProto;
    using eth_src = of13::EthSrc;
    using eth_dst = of13::EthDst;
    using in_port = of13::InPort;
}*/

class L2Switch : public Application
{
    Q_OBJECT
    SIMPLE_APPLICATION(L2Switch, "L2-switch")
public:
    void init(Loader* loader, const Config& config) override;

protected slots:
    void onSwitchUp(SwitchPtr sw);

private:
    OFMessageHandlerPtr handler_;
};

} // namespace runos

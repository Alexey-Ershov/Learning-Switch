#pragma once

#include "Application.hpp"
#include "Loader.hpp"
#include "Controller.hpp"
#include "SwitchManager.hpp"
#include "api/SwitchFwd.hpp"
#include "oxm/openflow_basic.hh"
#include "lib/switch_and_port.hpp"

#include <boost/optional.hpp>
#include <boost/thread.hpp>

namespace runos {

using SwitchPtr = safe::shared_ptr<Switch>;
namespace of13 = fluid_msg::of13;

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

    SwitchManager* switch_manager_;
    ethaddr src_mac_;
    ethaddr dst_mac_;
    uint64_t dpid_;
    uint32_t in_port_;

    void send_unicast(const uint32_t& target_switch_and_port, const of13::PacketIn& pi);
    void send_broadcast(const of13::PacketIn& pi);
};

class HostsDatabase
{
public:
    bool set_switch_and_port(uint64_t dpid, uint32_t in_port, ethaddr mac);
    boost::optional<switch_and_port> get_switch_and_port(ethaddr mac);

private:
    boost::shared_mutex mutex;
    std::unordered_map<ethaddr, switch_and_port> seen_ports;
};

} // namespace runos

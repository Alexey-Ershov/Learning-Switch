#include "L2Switch.hpp"

#include "PacketParser.hpp"
#include "api/Packet.hpp"
#include <runos/core/logging.hpp>

#include <unordered_map>
#include <sstream>

namespace runos {

REGISTER_APPLICATION(L2Switch, {"controller",
                                "switch-manager",
                                ""})

void L2Switch::init(Loader* loader, const Config& config)
{
    LOG(INFO) << "L2Switch is up";

    switch_manager_ = SwitchManager::get(loader);
    connect(switch_manager_, &SwitchManager::switchUp,
            this, &L2Switch::onSwitchUp);

    const auto ofb_in_port = oxm::in_port();
    const auto ofb_eth_src = oxm::eth_src();
    const auto ofb_eth_dst = oxm::eth_dst();

    std::unordered_map<uint64_t,
            std::unordered_map<ethaddr, uint32_t>> seen_port;

    /*seen_port[1][ethaddr("00:00:00:00:00:01")] = 1;
    seen_port[1][ethaddr("00:00:00:00:00:02")] = 2;
    seen_port[2][ethaddr("00:00:00:00:00:03")] = 1;
    seen_port[3][ethaddr("00:00:00:00:00:04")] = 1;
    for (auto& map: seen_port) {
        LOG(INFO) << "dpid_: " << map.first;
        for (auto& it: map.second) {
            LOG(INFO) << "mac: " << it.first << std::endl
                      << "port: " << it.second;
        }
    }*/

    handler_ = Controller::get(loader)->register_handler(
    [=](of13::PacketIn& pi, OFConnectionPtr ofconn) mutable -> bool
    {
        LOG(INFO) << "Catch PacketIn";

        PacketParser pp(pi);
        runos::Packet& pkt(pp);

        src_mac_ = pkt.load(ofb_eth_src);

        if (is_broadcast(src_mac_)) {
            DLOG(WARNING) << "Broadcast source address, dropping";
            return false;
        }

        dst_mac_ = pkt.load(ofb_eth_dst);
        in_port_ = pkt.load(ofb_in_port);
        dpid_ = ofconn->dpid();
        seen_port[dpid_][src_mac_] = in_port_;
        auto it = seen_port[dpid_].find(dst_mac_);

        LOG(INFO) << dpid_;
        LOG(INFO) << in_port_;
        LOG(INFO) << src_mac_;
        LOG(INFO) << dst_mac_;

        if (it != seen_port[dpid_].end()) {
            LOG(INFO) << "Founded host";
            send_unicast(it->second);

        } else {
            LOG(INFO) << "Broadcast";
            send_broadcast(pi);
        }

        return true;
    }, -5);
}

void L2Switch::onSwitchUp(SwitchPtr sw)
{
    LOG(INFO) << "Switch Up!";

    of13::FlowMod fm;
    fm.command(of13::OFPFC_ADD);
    fm.table_id(0);
    fm.priority(1);
    of13::ApplyActions applyActions;
    of13::OutputAction output_action(of13::OFPP_CONTROLLER, 0xFFFF);
    applyActions.add_action(output_action);
    fm.add_instruction(applyActions);
    sw->connection()->send(fm);
}

void L2Switch::send_unicast(const uint32_t& target_port)
{
    of13::FlowMod fm;
    fm.command(of13::OFPFC_ADD);
    fm.table_id(0);
    fm.priority(2);
    std::stringstream ss;
    fm.idle_timeout(uint64_t(60));
    fm.hard_timeout(uint64_t(1800));

    ss.str(std::string());
    ss.clear();
    ss << src_mac_;
    fm.add_oxm_field(new of13::EthSrc{
            fluid_msg::EthAddress(ss.str())});
    ss.str(std::string());
    ss.clear();
    ss << dst_mac_;
    fm.add_oxm_field(new of13::EthDst{
            fluid_msg::EthAddress(ss.str())});

    of13::ApplyActions applyActions;
    of13::OutputAction output_action(target_port,
            of13::OFPCML_NO_BUFFER);
    applyActions.add_action(output_action);
    fm.add_instruction(applyActions);
    switch_manager_->switch_(dpid_)->connection()->send(fm);
}

void L2Switch::send_broadcast(const of13::PacketIn& pi)
{
    of13::PacketOut po;
    po.data(pi.data(), pi.data_len());
    po.in_port(in_port_);
    of13::OutputAction output_action(of13::OFPP_ALL,
            of13::OFPCML_NO_BUFFER);
    po.add_action(output_action);
    switch_manager_->switch_(dpid_)->connection()->send(po);
}

} // namespace runos

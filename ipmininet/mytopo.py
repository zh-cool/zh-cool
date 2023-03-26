from ipmininet import DEBUG_FLAG
from ipmininet.iptopo import IPTopo
from ipmininet.ipnet import IPNet
from ipmininet.cli import IPCLI
from ipmininet.router.config import RouterConfig, STATIC, StaticRoute
from ipmininet.srv6 import enable_srv6
from mininet.log import lg

"""
10.0.1.2/24                        10.0.2.2/24
    h1<-----r1------r2------r3------->h2
            |       |       |
            |       |       |
            --------r4-------
                    |
                    |
                    h3
                10.0.3.2/24
    """


class MyTopo(IPTopo):
    def build(self, *args, **kwargs):
        h1 = self.addHost("h1")
        h2 = self.addHost("h2")
        h3 = self.addHost("h3")

        r1 = self.addRouter("r1", config=RouterConfig,
                            lo_addresses=["fc00:1::1/64"])
        r2 = self.addRouter("r2", config=RouterConfig,
                            lo_addresses=["fc00:2::2/64"])
        r3 = self.addRouter("r3", config=RouterConfig,
                            lo_addresses=["fc00:3::3/64"])
        r4 = self.addRouter("r4", config=RouterConfig,
                            lo_addresses=["fc00:4::4/64"])

        lh1r1 = self.addLink(h1, r1)
        lh1r1[h1].addParams(ip="10.0.1.2/24")
        lh1r1[r1].addParams(ip="10.0.1.1/24")

        lr1r2 = self.addLink(r1, r2)
        lr1r2[r1].addParams(ip=("2001:12::1/64"))
        lr1r2[r2].addParams(ip=("2001:12::2/64"))
        lr1r4 = self.addLink(r1, r4)
        lr1r4[r1].addParams(ip="2001:14::1/64")
        lr1r4[r4].addParams(ip="2001:14::2/64")

        lr2r3 = self.addLink(r2, r3)
        lr2r3[r2].addParams(ip=("2001:23::1/64"))
        lr2r3[r3].addParams(ip=("2001:23::2/64"))
        lr2r4 = self.addLink(r2, r4)
        lr2r4[r2].addParams(ip="2001:24::1/64")
        lr2r4[r4].addParams(ip="2001:24::2/64")

        lr3h2 = self.addLink(r3, h2)
        lr3h2[r3].addParams(ip="10.0.2.1/24")
        lr3h2[h2].addParams(ip="10.0.2.2/24")
        lr3r4 = self.addLink(r3, r4)
        lr3r4[r3].addParams(ip="2001:34::1/64")
        lr3r4[r4].addParams(ip="2001:34::2/64")

        lh3r4 = self.addLink(h3, r4)
        lh3r4[h3].addParams(ip="10.0.3.2/24")
        lh3r4[r4].addParams(ip="10.0.3.1/24")

        r1.addDaemon(STATIC, static_routes=[StaticRoute("fc00:2::/64", "2001:12::2"),
                                            StaticRoute("fc00:3::/64", "2001:12::2"),
                                            StaticRoute("fc00:4::/64", "2001:14::2")])

        r2.addDaemon(STATIC, static_routes=[StaticRoute("fc00:1::/64", "2001:12::1"),
                                            StaticRoute("fc00:3::/64", "2001:23::2"),
                                            StaticRoute("fc00:4::/64", "2001:24::2")])

        r3.addDaemon(STATIC, static_routes=[StaticRoute("fc00:1::/64", "2001:23::1"),
                                            StaticRoute("fc00:2::/64", "2001:23::1"),
                                            StaticRoute("fc00:4::/64", "2001:34::2")])

        r4.addDaemon(STATIC, static_routes=[StaticRoute("fc00:1::/64", "2001:14::1"),
                                            StaticRoute("fc00:2::/64", "2001:24::1"),
                                            StaticRoute("fc00:3::/64", "2001:34::1")])

    def post_build(self, net):
        for n in net.hosts + net.routers:
            enable_srv6(n)
        super().post_build(net)


if __name__ == '__main__':
    DEBUG_FLAG = True
    lg.setLogLevel("info")
    net = IPNet(topo=MyTopo(), use_v4=True, allocate_IPs=False)
    try:
        net.start()
        net["r1"].cmd("ip route add 10.0.3.0/24 encap seg6 mode encap segs fc00:4::bb dev r1-eth2")
        net["r1"].cmd("ip route add fc00:1::bb/128 encap seg6local action End.DX4 nh4 10.0.1.2 dev r1-eth0")

        net["r4"].cmd("ip route add 10.0.1.0/24 encap seg6 mode encap segs fc00:1::bb dev r4-eth0")
        net["r4"].cmd("ip route add fc00:4::bb/128 encap seg6local action End.DX4 nh4 10.0.3.2 dev r4-eth3")
        IPCLI(net)
    finally:
        net.stop()

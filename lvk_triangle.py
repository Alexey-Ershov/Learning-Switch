from mininet.topo import Topo

class MyTopo(Topo):

    def __init__(self):

        # Initialize topology
        Topo.__init__(self)

        # Add hosts and switches
        host0 = self.addHost('h0')
        host1 = self.addHost('h1')
        host2 = self.addHost('h2')
        host3 = self.addHost('h3')

        switch1 = self.addSwitch('s1')
        switch2 = self.addSwitch('s2')
        switch3 = self.addSwitch('s3')
        
        # Add links
        self.addLink(host0, switch1)
        self.addLink(host1, switch1)
        self.addLink(host2, switch2)
        self.addLink(host3, switch3)
        self.addLink(switch1, switch2)
        self.addLink(switch2, switch3)
        

topos = { 'mytopo': ( lambda: MyTopo() ) }

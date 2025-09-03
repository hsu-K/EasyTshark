import { Layout } from '@arco-design/web-react';
import "./style/global.css"
import { BrowserRouter as Router, Route, Link } from 'react-router-dom';
import { Menu } from '@arco-design/web-react';
import { useState, useEffect, useRef } from 'react';

import Navbar from './components/Navbar.tsx';
import DataPacketPage from './components/DataPacketPage.tsx';
import SessionPage from './components/SessionPage.tsx';
import IPStatsPage from './components/IPStatsPage.tsx';
import ProtoStatsPage from './components/ProtoStatsPage.tsx';


import {
  IconMenuFold,
  IconMenuUnfold,
  IconApps,
  IconBug,
  IconBulb,
  IconBook,
} from '@arco-design/web-react/icon';
const MenuItem = Menu.Item;
const SubMenu = Menu.SubMenu;

const Sider = Layout.Sider;
const Header = Layout.Header;
const Content = Layout.Content;

function PageLayout() {

  const DataPacketPageRef = useRef(null);

  const updateData = () => {
    DataPacketPageRef.current.reloadData();
  }

  return (
    <div className='layout-basic-demo'>
      <Router>
        <Layout style={{ height: '1000px' }}>
          <Header>
            <Navbar onUpdateData={updateData}></Navbar>
          </Header>
          <Layout>
            <Sider>
              <div className='sidebar-menu'>
                <Menu
                  style={{ width: 200, height: '100%' }}
                  hasCollapseButton
                  defaultOpenKeys={['0']}
                  defaultSelectedKeys={['0_1']}
                >
                  <SubMenu
                    key='0'
                    title={
                      <>
                        <IconApps /> 封包分析
                      </>
                    }
                  >
                    <MenuItem key='allPackets'><Link to="/data/dataPacket/all">全部封包</Link></MenuItem>
                    <MenuItem key='arpPackets'><Link to="/data/dataPacket/arp">ARP封包</Link></MenuItem>
                    <MenuItem key='icmpPackets'><Link to="/data/dataPacket/icmp">ICMP封包</Link></MenuItem>
                    <MenuItem key='icmpv6Packets'><Link to="/data/dataPacket/icmpv6">ICMPv6封包</Link></MenuItem>
                  </SubMenu>
                  <SubMenu
                    key='1'
                    title={
                      <>
                        <IconBug /> Session分析
                      </>
                    }
                  >
                    <MenuItem key='tcpSession'><Link to="/data/session/tcp">TCP Session</Link></MenuItem>
                    <MenuItem key='udpSession'><Link to="/data/session/udp">UDP Session</Link></MenuItem>
                    <MenuItem key='dnsSession'><Link to="/data/session/dns">DNS Session</Link></MenuItem>
                    <MenuItem key='httpSession'><Link to="/data/session/http">HTTP Session</Link></MenuItem>
                    <MenuItem key='tlsSession'><Link to="/data/session/tls">SSL/TLS Session</Link></MenuItem>
                    <MenuItem key='sshSession'><Link to="/data/session/ssh">SSH Session</Link></MenuItem>
                  </SubMenu>
                  <SubMenu
                    key='2'
                    title={
                      <>
                        <IconBulb /> 統計分析
                      </>
                    }
                  >
                    <MenuItem key='ipCount'><Link to="/data/statistics/ip">IP統計</Link></MenuItem>
                    <MenuItem key='protoCount'><Link to="/data/statistics/proto">協議統計</Link></MenuItem>
                  </SubMenu>
                </Menu>
              </div>
            </Sider>

            <Content style={{
              flex: 1,
              overflow: 'auto',
              padding: '16px',
            }}>
              <Route
                path="/data/dataPacket/:type"
                render={(props) => (
                  <DataPacketPage {...props} ref={DataPacketPageRef} />
                )}
              />
              <Route path="/data/session/:type" component={SessionPage} />
              <Route path="/data/statistics/ip" component={IPStatsPage} />
              <Route path="/data/statistics/proto" component={ProtoStatsPage} />
            </Content>
          </Layout>
        </Layout>
      </Router>
    </div>
  )
}

export default PageLayout;
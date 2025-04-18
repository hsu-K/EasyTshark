import { Layout, Menu } from '@arco-design/web-react';
import "./style/global.css";
import Navbar from './components/Navbar';
import DataPacketPage from './components/DataPacketPage';
import SessionPage from './components/SessionPage';
import IPStatsPage from './components/IPStatsPage';
import ProtoStatsPage from './components/ProtoStatsPage';
import { BrowserRouter as Router, Route, Link, Routes } from 'react-router-dom';
import {
  // IconMenuFold,
  // IconMenuUnfold,
  IconApps,
  IconBug,
  IconBulb,
  // IconBook,
} from '@arco-design/web-react/icon';
const MenuItem = Menu.Item;
const SubMenu = Menu.SubMenu;
// const MenuItemGroup = Menu.ItemGroup;

const Sider = Layout.Sider;
const Header = Layout.Header;
const Content = Layout.Content;

const PageLayout = () => {
  return (
    <div className='layout-basic-demo'>

        <Layout style={{ height: '95vh' }}>
          <Header>
            <Navbar />
          </Header>
          <Layout>
            <Sider>
              <div className='sidebar-menu'>
                <Menu
                  style={{ width: '100%', height: '100%' }}
                  hasCollapseButton
                  defaultOpenKeys={['0']}
                  defaultSelectedKeys={['0_1']}
                >
                  <SubMenu
                    key='0'
                    title={
                      <>
                        <IconApps /> 數據包分析
                      </>
                    }
                  >
                    <MenuItem key='allPackets'><Link to="/data/dataPacket/all">全部數據包</Link></MenuItem>
                    <MenuItem key='arpPackets'><Link to="/data/dataPacket/arp">ARP數據包</Link></MenuItem>
                    <MenuItem key='icmpPackets'><Link to="/data/dataPacket/icmp">ICMP數據包</Link></MenuItem>
                    <MenuItem key='icmpv6Packets'><Link to="/data/dataPacket/icmpv6">ICMPv6數據包</Link></MenuItem>
                  </SubMenu>
                  <SubMenu
                    key='1'
                    title={
                      <>
                        <IconBug /> 活動分析
                      </>
                    }
                  >
                    <MenuItem key='tcpSession'><Link to="/data/session/tcp">TCP活動</Link></MenuItem>
                    <MenuItem key='udpSession'><Link to="/data/session/udp">UDP活動</Link></MenuItem>
                    <MenuItem key='dnsSession'><Link to="/data/session/dns">DNS活動</Link></MenuItem>
                    <MenuItem key='httpSession'><Link to="/data/session/http">HTTP活動</Link></MenuItem>
                    <MenuItem key='tlsSession'><Link to="/data/session/tls">SSL/TLS活動</Link></MenuItem>
                    <MenuItem key='sshSession'><Link to="/data/session/ssh">SSH活動</Link></MenuItem>
                  </SubMenu>
                  <SubMenu
                    key='2'
                    title={
                      <>
                        <IconBulb /> 統計分析
                      </>
                    }
                  >
                    <MenuItem key='ipCount'><Link to="/data/ipStats">IP統計</Link></MenuItem>
                    <MenuItem key='protoCount'><Link to="/data/protoStats">協議統計</Link></MenuItem>
                  </SubMenu>
                </Menu>
              </div>
            </Sider>
            <Content>
              <Routes>
                <Route path="dataPacket/:type" element={<DataPacketPage />} />
                <Route path="session/:type" element={<SessionPage />} />
                <Route path="ipStats" element={<IPStatsPage />} />
                <Route path="protoStats" element={<ProtoStatsPage />} />
              </Routes>

            </Content>

          </Layout>
        </Layout>
    </div>
  )
};

export default PageLayout;
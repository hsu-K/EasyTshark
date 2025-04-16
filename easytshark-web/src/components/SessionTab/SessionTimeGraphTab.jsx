import React, { useEffect, useState, useRef } from 'react';
import { Card, Grid, Spin, Tabs, Tag, Typography } from '@arco-design/web-react';
import { calculateDuration } from '../../util.js'
import { apiFetchData, apiPost } from '../../Api.js';
import { useLocation } from 'react-router-dom';
import './styles/SessionDetailStyle.css';
import DataPacketPage from '../DataPacketPage.jsx';


const { Ellipsis } = Typography;



const SessionTimeGraphTab = ({ sessionId }) => {
  const [data, setData] = useState([]);
  const [currentRowId, setCurrentRowId] = useState([1]);
  const [record, setRecord] = useState(JSON.parse(localStorage.getItem(`row${sessionId}`)));

  const DataPacketPageRef = useRef(null);

  const loadTimeGraph = async () => {
    const _data = await apiPost('/api/getPacketList', {
      pageSize: 100,
      pageNum: 1,
      session_id: parseInt(sessionId)
    });

    setData(_data.data);
  }

  useEffect(() => {
    loadTimeGraph()
  }, []);
  return (
    <div style={{ margin: 20 }} className='details'>
      <div style={{ margin: '0 10px 10px 10px' }}>
        <Grid.Row gutter={24} style={{ flexFlow: 'nowrap' }}>
          <Typography.Paragraph style={{ minWidth: 450 }}>
            <Grid.Col span={7} className='details-col'>
              <div className="first-col" style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
                {/* 左側 IP */}
                <div style={{ width: 200, textAlign: 'right' }}>
                  <Typography.Ellipsis showTooltip className="text-[14px]">
                    {record.ip1}:{record.ip1_port}
                  </Typography.Ellipsis>
                </div>

                {/* 中間 箭頭+時間 垂直堆疊 */}
                <div style={{
                  display: 'flex',
                  flexDirection: 'column',
                  alignItems: 'center',
                  justifyContent: 'center',
                  minWidth: 100
                }}>
                  <div className="arrow">
                    <span className="proto">
                      <Tag bordered color="arcoblue" size="small">
                        {record.app_proto || record.trans_proto}
                      </Tag>
                    </span>
                    <span className="icon"></span>
                  </div>
                  <span>{calculateDuration(record.start_time, record.end_time)}</span>
                </div>

                {/* 右側 IP */}
                <div style={{ width: 200 }}>
                  <Typography.Ellipsis showTooltip className="text-[14px]">
                    {record.ip2}:{record.ip2_port}
                  </Typography.Ellipsis>
                </div>
              </div>


              {data.map((item, index) => {
                const timestampDiff = index === 0 ? 0 : ((item.timestamp * 1000000) - (data[0].timestamp * 1000000)) / 1000000;
                const isRight = item.src_ip === record.ip1;

                return (
                  <div
                    key={item.frame_number}
                    id={`box-${item.frame_number}`}
                    className={`box ${((currentRowId === item.frame_number) || (!currentRowId && index === 0)) ? 'selected' : ''} cursor-pointer`}
                    onClick={() => {
                      setCurrentRowId(item.frame_number)
                      DataPacketPageRef.current.setCurrentRowId(item.frame_number); // 滾動到頂部
                    }}
                  >
                    {/* 數據包內容（平行顯示） */}
                    <div style={{
                      display: 'flex',
                      width: '100%',
                      alignItems: 'center',
                      height: '30px' // 統一高度，確保對齊
                    }}>
                      {/* 左側區域（Frame 編號和時間差放在一起） */}
                      <div style={{ minWidth: '120px', display: 'flex', alignItems: 'center' }}>
                        {/* Frame 編號 */}
                        <div style={{ minWidth: '30px', textAlign: 'left', fontWeight: 'bold' }}>
                          {item.frame_number}
                        </div>

                        {/* 時間差 */}
                        <div style={{ minWidth: '80px', fontSize: 12, color: '#888', paddingLeft: '5px' }}>
                          {timestampDiff}
                        </div>
                      </div>

                      {/* 箭頭 */}
                      <div style={{ width: '60px', display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
                        <div className={`arrow ${isRight ? 'arrow-right' : 'arrow-left'}`} style={{
                          marginTop: 0,
                          display: 'flex',
                          alignItems: 'center',
                          height: '100%'
                        }}>
                          <span className="icon" style={{ marginTop: 0 }}></span>
                        </div>
                      </div>

                      {/* 右側 - 數據長度 (固定為一個整體，不拆分) */}
                      <div style={{
                        minWidth: '40px',
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'center',
                        whiteSpace: 'nowrap'
                      }}>
                        {item.len}
                      </div>

                      {/* 資訊 - 使用 Typography.Ellipsis 處理過長文字 */}
                      <div style={{ flex: 1, paddingLeft: '10px', overflow: 'hidden' }}>
                        <Typography.Ellipsis showTooltip style={{ width: '100%' }}>
                          {item.info}
                        </Typography.Ellipsis>
                      </div>
                    </div>
                  </div>
                );
              })}
            </Grid.Col>
          </Typography.Paragraph>
          <Grid.Col span={17} style={{ marginTop: -10, flexGrow: 1 }}>
            <DataPacketPage
              ref = {DataPacketPageRef}
              // 讓子組件可以透過match來獲取type和sessionId
              match={{
                params: {
                  type: 'detail',
                  sessionId: sessionId,
                },
              }}>
            </DataPacketPage>
          </Grid.Col>
        </Grid.Row>
      </div>
    </div>
  )
};

export default SessionTimeGraphTab;
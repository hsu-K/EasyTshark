import React, { useEffect, useState } from 'react';
import { Button, Card, Grid, Typography } from '@arco-design/web-react';
import LineChart from './LineChart.tsx';
import { apiGet, apiPost } from '../Api.ts';

// interface CaptureProps {
//   type? :string;
//   onsubmit?: (type: string) => void;
// }


const Capture = ({ type = '', onsubmit = function (_type?: string): void { } }) => {
  const [datas, setDatas] = useState([]);
  const [loading, setLoading] = useState(false);
  let intervalid: number | null = null;

  const startMonitorAdaptersFlowTrend = async () => {
    await apiGet('/api/startMonitorAdaptersFlowTrend');
    getAdaptersFlowTrendData();
  }

  const getAdaptersFlowTrendData = async () => {
    const values = await apiGet('/api/getAdaptersFlowTrendData');
    setDatas(values?.data || []);
  }

  const startCapture = async (key) => {
    setLoading(true)
    await apiPost('/api/startCapture', { adapterName: key })
    onsubmit(type);
    setLoading(false)
  }

  const stopMonitorAdaptersFlowTrend = async (item?: any, type?: string) => {
    await apiGet('/api/stopMonitorAdaptersFlowTrend');
    if (type) {
      startCapture(item);
    }
  };

  useEffect(() => {
    startMonitorAdaptersFlowTrend();
    intervalid = setInterval(getAdaptersFlowTrendData, 1000);
    return () => {
      if (intervalid !== null) {
        clearInterval(intervalid);
        stopMonitorAdaptersFlowTrend();
      }
    }
  }, []);

  return <Card style={{
    background: 'var(--color-fill-1)',
    maxHeight: 250,
    overflowY: 'auto',
    overflowX: 'hidden',
  }}
    className='mb-[20px]'>
    {Object.entries(datas).map(([key, value]) => (
      <div className={"mb-[10px]"} key={key}>
        <Grid.Row gutter={24} style={{ alignItems: 'end' }}>
          <Grid.Col span={5}><Typography.Ellipsis showTooltip>{key}</Typography.Ellipsis></Grid.Col>
          <Grid.Col span={16}><LineChart data={value} /></Grid.Col>
          <Grid.Col span={3} style={{ textAlign: 'right' }}><Button type='primary' size={type ? 'small' : 'default'} onClick={() => {stopMonitorAdaptersFlowTrend(key, 'cap');}}>抓包</Button></Grid.Col>
        </Grid.Row>
      </div>
    ))}
  </Card>;
};

export default Capture;
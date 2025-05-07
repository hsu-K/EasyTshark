import React, { useEffect, useState } from 'react';
import { Button, Card, Grid, Typography } from '@arco-design/web-react';
import styles from '../style/global.css';
import LineChart from './LineChart';
import { apiGet, apiPost } from '../Api.js';

const Capture = ( {type = '', onsubmit = null}) => {
  const [datas, setDatas] = useState([]);
  const [loading, setLoading] = useState(false);
  let intervalId = null;

  const startMonitorAdaptersFlowTrend = async() => {
    await apiGet('/api/startMonitorAdaptersFlowTrend');
    getAdaptersFlowTrendData();
  }

  const getAdaptersFlowTrendData = async() => {
    const values = await apiGet('/api/getAdaptersFlowTrendData');
    setDatas(values?.data || []);
  }

  const startCapture = async(key) => {
    setLoading(true);
    await apiPost('/api/startCapture', {adapterName: key});
    onsubmit(type);
    setLoading(false);
  }

  const stopMonitorAdaptersFlowTrend = async (item, type) => {
    await apiGet('/api/stopMonitorAdaptersFlowTrend');
    if(type){
      startCapture(item);
    }
  }

  useEffect(() => {
    startMonitorAdaptersFlowTrend();
    intervalId = setInterval(() => {
      getAdaptersFlowTrendData();
    }, 1000);
    return () => {
      if (intervalId !== null){
        clearInterval(intervalId);
        stopMonitorAdaptersFlowTrend();
      }
    };
  }, []);

  return <Card style={{
            background: 'var(--color-fill-1)',
            maxHeight: 250,
            overflowY: 'auto',
            overflowX: 'hidden',
          }}
          className= 'mb-[20px]'>
          {Object.entries(datas).map(([key, value]) => {
            // if(key === '乙太網路'){
            //   console.log('Data for', key, ':', value);
            // }
            // console.log('Data for', key, ':', value);
            return (
              <div className={`${styles['lines']} mb-[10px]`} key={key}>
                <Grid.Row gutter={24} style={{ alignItems: 'end'}}>
                  <Grid.Col span={5}><Typography.Ellipsis showTooltip>{key}</Typography.Ellipsis></Grid.Col>
                  <Grid.Col span={16}><LineChart data={value} /></Grid.Col>
                  <Grid.Col span={3} style={{ textAlign: 'right'}}><Button type='primary' size={type ? 'small' : 'default'} onClick={()=>stopMonitorAdaptersFlowTrend(key, 'cap')}>抓包</Button></Grid.Col>
                </Grid.Row>
              </div>
            );
          })}
          </Card>
}

export default Capture;
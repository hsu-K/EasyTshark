import React, { useEffect, useRef, useState } from 'react';
import { Button, Card, Grid, Message, Spin, Typography, Upload } from '@arco-design/web-react';
import { useHistory } from 'react-router-dom';
// import styles from '../style/home.css'
import { apiGet, apiPost } from '../Api.ts';
import { IconPlus } from '@arco-design/web-react/icon';
import Capture from '../components/Capture.tsx';


const HomePage = () => {
  const history = useHistory();
  const fileInputRef = useRef(null);
  const [loading, setLoading] = useState(false);
  const [networkLoading, setNetworkLoading] = useState(false);
  const [data, setData] = useState([]);
  const [cap, setCap] = useState(false);

  const stopMonitorAdaptersFlowTrend = async (item?: any, type?: any) => {
    await apiGet('/api/stopMonitorAdaptersFlowTrend');
  }

  const onsubmit = () => {
    setCap(true);
    setLoading(true);
    history.push('/data/dataPacket/all');
  }

  const handleSelectFile = async () => {
    Message.info("選擇封包文件進行分析");
    try {
      const selectedFilePath = await window.electronAPI.openFileDialog();
      if (selectedFilePath) {
        setLoading(true)
        try {
          await apiGet('/api/stopMonitorAdaptersFlowTrend')
          await apiPost('/api/analysisFile', { filePath: selectedFilePath })
          history.push('/data/dataPacket/all')
          setLoading(false)
        } catch {
          setLoading(false)
        }
      }
    } catch (error) {
      console.error('文件選擇或讀取失敗:', error);
    }
  }


  return (
    <div>
      <Spin loading={loading} style={{ width: '100%' }} tip={`${cap ? '實時抓包' : '文件'}分析中...`}>
        <div className='home' style={{ padding: '30px 20%' }}>
          <Typography.Title heading={6}>實石抓包分析</Typography.Title>
          <Capture onsubmit={onsubmit} />
          <Typography.Title heading={6}>離線封包文件</Typography.Title>
          <div className='input-file' onClick={() => handleSelectFile()} style={{ height: 80 }}>
            <p><IconPlus style={{ fontSize: 22 }} /></p>
            <p>上傳檔案</p>
          </div>
        </div>
      </Spin>
    </div>
  )
};


export default HomePage;
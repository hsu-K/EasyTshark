import React, { useEffect, useRef, useState } from 'react';
import { Button, Card, Grid, Message, Spin, Typography, Upload } from '@arco-design/web-react';
import { useNavigate } from 'react-router-dom';
import styles from '../style/global.css';
import { apiGet, apiPost } from '../Api.js';
import { IconPlus } from '@arco-design/web-react/icon';
import Capture from './Capture.jsx';

const HomePage = () => {
  const navigate = useNavigate();
  const fileInputRef = useRef(null);
  const [loading, setLoading] = useState(false);
  const [networkLoading, setNetworkLoading] = useState(false);
  const [data, setData] = useState([]);
  const [cap, setCap] = useState(false);

  const onsubmit = () => {
    setCap(true);
    setLoading(true);
    // 跳轉到數據包頁面
    navigate('/data/dataPacket/all');
  }

  const handleSelectFile = async () => {
    try {
      const selectedFile = await window.electronAPI.openFileDialog();
      if (selectedFile) {
        setLoading(true);
        try {
          await apiGet('/api/stopMonitorAdaptersFlowTrend')
          await apiPost('/api/analyzeFile', { filePath: selectedFile})
          navigate('/data/dataPacket/all');
          setLoading(false);
        }
        catch{
          setLoading(false);
          Message.error('文件分析失敗');
        }
      }
    }
    catch (error) {
      console.error('Error selecting file:', error);
    }
  }

  return (
    <div>
      <Spin loading={loading} style={{ width: '100%' }} tip={`${cap ? '實時抓包' : '文件'}分析中...`}>
        <div className='home' style={{ padding: '30px 20%' }}>
          <Typography.Title heading={6}>實時抓包分析</Typography.Title>
          <Capture onsubmit={onsubmit} />
          <Typography.Title heading={6}>離線分析文件</Typography.Title>
          <div className='input-file' onClick={() => handleSelectFile()} style={{ height: 80 }}>
            <p><IconPlus style={{ fontSize: 22 }} /></p>
            <p>點擊或拖曳文件到此處上傳</p>
          </div>
        </div>
      </Spin>
    </div>
  )
}

export default HomePage;

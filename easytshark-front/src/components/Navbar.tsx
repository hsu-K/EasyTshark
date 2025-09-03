import React, { useState, useEffect } from 'react';
import { useHistory } from 'react-router-dom';
import { Button, Message, Popover } from '@arco-design/web-react';
import "../style/global.css"
import { IconPlayCircle, IconHome, IconRecordStop, IconFile, IconSave } from '@arco-design/web-react/icon';
import Capture from './Capture.tsx';
import { apiGet, apiPost } from '../Api.ts';

const Navbar = ({onUpdateData = ():void => {}}) => {
  const STATUS_IDLE = 0
  const STATUS_ANALYSIS_FILE = 1
  const STATUS_CAPTURING = 2
  const STATUS_MONITORING = 3

  const [workStatus, setWorkStatus] = useState(STATUS_IDLE);
  const [stopLoading, setStopLoading] = useState(false);
  const [visible, setVisible] = useState(false);
  const [fileLoading, setFileLoading] = useState(false);

  const history = useHistory()

  const checkStatus = async () => {
    const _data = await apiGet('/api/getWorkStatus');
    console.log("Status: ", _data.data.workStatus);
    setWorkStatus(_data.data.workStatus);
  }

  useEffect(() => {
    checkStatus();
  }, [])

  const startCapture = () => {
    Message.info('開始抓包!');
  };

  const goHome = () => {
    history.push('/home');
  }

  const onCaptrueSubmit = () => {
    setVisible(false);
    if(onUpdateData != null){
      checkStatus();
      onUpdateData();
    }
  }

  const handleSelectFile = async () => {
    // Message.info("選擇封包文件進行分析");
    try {
      const selectedFilePath = await window.electronAPI.openFileDialog();
      if (selectedFilePath) {
        setFileLoading(true)
        try {
          await apiPost('/api/analysisFile', { filePath: selectedFilePath })
          setFileLoading(false)
          if (onUpdateData != null) { onUpdateData(); }
        } catch {
          setFileLoading(false)
        }
      }
    } catch (error) {
      console.error('文件選擇或讀取失敗:', error);
    }
  }



  const stopCapture = async () => {
    setStopLoading(true);
    Message.info('停止抓包!');
    const _data = await apiGet('/api/stopCapture');
    setStopLoading(false);
    checkStatus();
  };

  const analysisFile = () => {
    Message.info('分析文件!');
    handleSelectFile();
  };

  const saveFile = async () => {
    Message.info('保存文件!');
    try {
      const selectedFilePath = await window.electronAPI.showSavePath();
      if(selectedFilePath) {
        try{
          await apiPost('/api/savePacket', { savePath: selectedFilePath, filter: ''})
          Message.success('文件保存成功!');
        } catch (error) {
          console.error('文件保存失敗:', error);
        }
      }
    } catch (error) {
      console.error('文件保存失敗:', error);
    }
  };


  return (
    <div id="nav-bar" style={{ padding: 20 }}>
      <a href="/home"><Button type="primary" status='warning' icon={<IconHome />} disabled={workStatus != STATUS_IDLE}>首頁</Button></a>
      <Popover
        title='實時抓包分析'
        trigger="click"
        popupVisible={visible}
        // className='!max-w-[900px]'
        onVisibleChange={(value) => setVisible(value)}
        position="bottom"
        style={{ maxWidth: 'none' }} // 直接添加 style 屬性
        overlayStyle={{
          width: '650px',
          maxWidth: '60vw',
        }}
        content={
          <div style={{
            width: '650px',
            maxHeight: '70vh',
            overflow: 'auto',
            padding: '0 10px'
          }}>
            <Capture type="home" onsubmit={() => { setVisible(false) }} />
          </div>
        }
      >
        <Button
          type="primary"
          onClick={startCapture}
          icon={<IconPlayCircle />}
          disabled={[STATUS_ANALYSIS_FILE, STATUS_CAPTURING].includes(workStatus)}
        >
          開始抓包
        </Button>

      </Popover>

      <Button type="primary" status="danger" onClick={stopCapture} loading={stopLoading} icon={<IconRecordStop />} disabled={[STATUS_IDLE, STATUS_ANALYSIS_FILE, STATUS_MONITORING].includes(workStatus)}>停止抓包</Button>
      <Button type="primary" onClick={analysisFile} icon={<IconFile />} disabled={workStatus != STATUS_IDLE}>分析文件</Button>
      <Button type="primary" onClick={saveFile} icon={<IconSave />}>保存文件</Button>

    </div>
  );
}

export default Navbar;
import React, { useState, useEffect, useRef } from "react";
import { Button, Message, Popover, Space } from "@arco-design/web-react";
import "../style/global.css";
import Capture from "./Capture.jsx";
import { IconPlayCircle, IconHome, IconRecordStop, IconFile, IconSave } from '@arco-design/web-react/icon';
import { apiGet, apiPost } from "../Api.js";
import { useNavigate } from 'react-router-dom';

const STATUS_IDLE = 0
const STATUS_ANALYSIS = 1
const STATUS_CAPTURING = 2
const STATUS_MONITORING = 3

const Navbar = ({ onUpdateData = null }) => {

  const [stopLoading, setStopLoading] = useState(false);
  const [visible, setVisible] = useState(false);
  const [workStatus, setWorkStatus] = useState(STATUS_IDLE);
  const [poperVisible, setPoperVisible] = useState(false);
  const [fileLoading, setFileLoading] = useState(false);

  const navigate = useNavigate();

  const goHome = () => {
    navigate('/home');
  }

  const checkStatus = async () => {
    const _data = await apiGet('/api/getWorkStatus');
    setWorkStatus(_data.data.workStatus);
    // console.log('workStatus', _data.data.workStatus);
  }

  // const timerRef = useRef(null);
  // useEffect(() => {
  //   checkStatus();

  //   let id = setInterval(checkStatus, 1000);
  //   timerRef.current = id;

  //   return () => {
  //     if (timerRef.current !== 0) {
  //       clearInterval(timerRef.current);
  //       timerRef.current = 0;
  //     }
  //   };
  // }, []);

  const startCapture = () => {
    Message.info('開始抓包！');
  };

  const stopCapture = async () => {
    setStopLoading(true);
    await apiGet('/api/stopCapture');
    setStopLoading(false);
    checkStatus();
    Message.info('停止抓包！');
  };

  const onCaptureSubmit = async () => {
    setPoperVisible(false);
    if (onUpdateData != null) {
      checkStatus();
      onUpdateData();
    }
  }

  const handleSelectFile = async () => {
    try {
      const selectedFilePath = await window.electronAPI.openFileDialog();
      if (selectedFilePath) {
        setFileLoading(true);
        try {
          await apiGet('/api/stopMonitorAdaptersFlowTrend')
          await apiPost('/api/analyzeFile', { filePath: selectedFilePath })
          setFileLoading(false);
          if (onUpdateData != null) {
            onUpdateData();
          }
        }
        catch {
          setFileLoading(false);
          Message.error('文件分析失敗');
        }
      }
    }
    catch (error) {
      console.error('Error selecting file:', error);
    }
  }


  const saveFile = async () => {
    try {
      const selectedFilePath = await window.electronAPI.showSavePath();
      if (selectedFilePath) {
        try {
          await apiPost('/api/savePacket', { savePath: selectedFilePath, filter: '' })
          Message.info('保存成功！');
        }
        catch{

        }
      }
    }
    catch (error) {}
  };

  return (
    <div id="nav-bar" style={{ padding: 20 }}>
      <Button type="primary" onClick={goHome} status="waring" icon={<IconHome />} disabled={workStatus != STATUS_IDLE}>首頁</Button>

      <Popover
        title='實時抓包分析'
        trigger="click"
        popupVisible={poperVisible}
        // className='!max-w-[900px]'
        onVisibleChange={(value) => {
          setPoperVisible(value)
        }}

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
            <Capture type="home" onsubmit={onCaptureSubmit} />
          </div>
        }
      >
        <Button
          type="primary"
          icon={<IconPlayCircle />}
          disabled={[STATUS_ANALYSIS, STATUS_CAPTURING].includes(workStatus)}
        >
          開始抓包
        </Button>
      </Popover>

      <Button type="primary" status="danger" onClick={stopCapture} loading={stopLoading} icon={<IconRecordStop />} disabled={[STATUS_IDLE, STATUS_ANALYSIS, STATUS_MONITORING].includes(workStatus)}>停止抓包</Button>
      <Button type="primary" onClick={handleSelectFile} icon={<IconFile />} disabled={workStatus != STATUS_IDLE}>分析文件</Button>
      <Button type="primary" onClick={saveFile} icon={<IconSave />}>保存文件</Button>
    </div>
  )
};

export default Navbar;
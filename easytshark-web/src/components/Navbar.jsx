import React, { useState, useEffect, useRef } from "react";
import { Button, Message, Popover, Space } from "@arco-design/web-react";
import "../style/global.css";
import Capture from "./Capture.jsx";
import { IconPlayCircle, IconHome, IconRecordStop, IconFile, IconSave } from '@arco-design/web-react/icon';
import { apiGet } from "../Api.js";


const Navbar = () => {
  const STATUS_IDLE = 0
  const STATUS_ANALYSIS = 1
  const STATUS_CAPTURING = 2
  const STATUS_MONITORING = 3

  const [visible, setVisible] = useState(false);

  const [workStatus, setWorkStatus] = useState(STATUS_IDLE);
  const checkStatus = async () => {
    const _data = await apiGet('/api/getWorkStatus');
    setWorkStatus(_data.data.workStatus);
    // console.log('workStatus', _data.data.workStatus);
  }

  const timerRef = useRef(null);
  useEffect(() => {
    checkStatus();

    let id = setInterval(checkStatus, 1000);
    timerRef.current = id;

    return () => {
      if (timerRef.current !== 0) {
        clearInterval(timerRef.current);
        timerRef.current = 0;
      }
    };
  }, []);

  const startCapture = () => {
    Message.info('開始抓包！');
  };

  const stopCapture = async () => {

    Message.info('停止抓包！');
  };

  const analysisFile = () => {
    Message.info('分析文件！');
  };

  const saveFile = () => {
    Message.info('保存文件！');
  };

  return (
    <div id="nav-bar" style={{ padding: 20 }}>
      <a href="/home"><Button type="primary" status="waring" icon={<IconHome />} disabled={workStatus != STATUS_IDLE}>首頁</Button></a>

      <Popover
        title='實時抓包分析'
        trigger="click"
        popupVisible={visible}
        // className='!max-w-[900px]'
        onVisibleChange={(value) => {
          setVisible(value)
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
            <Capture type="home" onsubmit={() => { setVisible(false) }} />
          </div>
        }
      >
        <Button
          type="primary"
          icon={<IconPlayCircle />}
          onClick={startCapture}
          disabled={[STATUS_ANALYSIS, STATUS_CAPTURING].includes(workStatus)}
        >
          開始抓包
        </Button>
      </Popover>

      <Button type="primary" status="danger" onClick={stopCapture} loading={false} icon={<IconRecordStop />} disabled={[STATUS_IDLE, STATUS_ANALYSIS, STATUS_MONITORING].includes(workStatus)}>停止抓包</Button>
      <Button type="primary" onClick={analysisFile} icon={<IconFile />} disabled={workStatus != STATUS_IDLE}>分析文件</Button>
      <Button type="primary" onClick={saveFile} icon={<IconSave />}>保存文件</Button>
    </div>
  )
};

export default Navbar;
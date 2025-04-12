import React from "react";
import { Button, Message, Space } from "@arco-design/web-react";
import "../style/global.css";

const Navbar = () => {
  const startCapture = () => {
    Message.info('開始抓包！');
  };

  const stopCapture = () => {
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
      <Button type="primary" onClick={startCapture}>開始抓包</Button>
      <Button type="primary" status="danger" onClick={stopCapture}>停止抓包</Button>
      <Button type="primary" onClick={analysisFile}>分析文件</Button>
      <Button type="primary" onClick={saveFile}>保存文件</Button>
    </div>
  )
};

export default Navbar;
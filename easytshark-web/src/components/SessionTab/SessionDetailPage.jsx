import React, { useEffect} from "react";
import { Tabs, Radio, Typegraphy } from '@arco-design/web-react';
import "../../style/global.css";
import SessionDataStreamTab from "./SessionDataStreamTab";
import SessionTimeGraphTab from "./SessionTimeGraphTab";
import { useLocation } from "react-router-dom";

const TabPane = Tabs.TabPane;

const SessionDetailPage = () => {
  const location = useLocation();
  const params = new URLSearchParams(location.search);
  const windowParams = new URLSearchParams(window.location.search);
  const sessionId = params.get('sessionId') || windowParams.get('sessionId');

  return (
    <>
      <Tabs type='card'>
        <TabPane key='1' title = 'Session時序圖'>
          <SessionTimeGraphTab sessionId={sessionId}/>
        </TabPane>
        <TabPane key='2' title = 'Session數據流'>
          <SessionDataStreamTab sessionId={sessionId}/>
        </TabPane>
      </Tabs>
    </>
  );
};

export default SessionDetailPage;
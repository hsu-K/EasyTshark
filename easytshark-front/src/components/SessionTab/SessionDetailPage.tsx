import React, { useEffect } from 'react';
import { Tabs, Radio, Typography } from '@arco-design/web-react';
import "../../style/global.css"
import { useLocation } from 'react-router-dom';

import SessionTimeGraphTab from './SessionTimeGraphTab.tsx';
import SessionDataStreamTab from './SessionDataStreamTab.tsx';

const TabPane = Tabs.TabPane;

const SessionDetailPage = () => {

  const location = useLocation();
  const params = new URLSearchParams(location.search);
  const windowParams = new URLSearchParams(window.location.search);
  const sessionId = params.get('sessionId') || windowParams.get('sessionId');
  // console.log("Session ID:", sessionId);

  return (
    <Tabs type='card'>
      <TabPane key='1' title='Session 時序圖'>
        <SessionTimeGraphTab sessionId={sessionId} />
      </TabPane>
      <TabPane key='2' title='Session 數據流'>
        <SessionDataStreamTab sessionId={sessionId} />
      </TabPane>
    </Tabs>
  );
}

export default SessionDetailPage;
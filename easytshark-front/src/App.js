import React from 'react';
import { Button, Message } from '@arco-design/web-react';
import PageLayout from './PageLayout.tsx';
import SessionDetailPage from './components/SessionTab/SessionDetailPage.tsx';
import { BrowserRouter as Router, Route, Link, Switch, HashRouter } from 'react-router-dom';
import HomePage from './components/HomePage.tsx';
// import Capture from './components/Capture.tsx';

const App = () => {
  return (
    <HashRouter>
      <div class="App">
        <div id="page">
          <Route path="/data" component={PageLayout} />
          <Route path="/detail" component={SessionDetailPage} />
          <Route path="/home" component={HomePage} />
        </div>
      </div>
    </HashRouter>
  );
};

export default App;

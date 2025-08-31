import React from 'react';
import { Button, Message } from '@arco-design/web-react';
import PageLayout from './PageLayout.tsx';
import SessionDetailPage from './components/SessionTab/SessionDetailPage.tsx';
import { BrowserRouter as Router, Route, Link, Switch } from 'react-router-dom';

const App = () => {
  return (
    <Router>
      <div class="App">
        <div id="page">
          <Route path="/data" component={PageLayout} />
          <Route path="/detail" component={SessionDetailPage} />
        </div>
      </div>
    </Router>
  );
};

export default App;

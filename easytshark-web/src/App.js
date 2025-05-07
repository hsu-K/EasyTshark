import React from 'react';
import PageLayout from './PageLayout';
import SessionDetailPage from './components/SessionTab/SessionDetailPage';
import { BrowserRouter as Router, Route, Link, Switch, Routes } from 'react-router-dom';
import HomePage from './components/HomePage';

function App() {

  return (
    <Router>
      <div class="App">
        <div id="page">
          <Routes>
            <Route path="/home" element={<HomePage />} />
            <Route path="/data/*" element={<PageLayout />} />
            <Route path="/detail" element={<SessionDetailPage />} />
          </Routes>
        </div>
      </div>
    </Router>
  );
}

export default App;

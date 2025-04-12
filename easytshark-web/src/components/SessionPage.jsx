import React, { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';

import { Table, TableColumnProps, Pagination } from '@arco-design/web-react';
import { Typography, Tag, Link } from '@arco-design/web-react';
import dayjs from 'dayjs';
import { apiPost } from '../Api.js';

const { Text, Ellipsis } = Typography;

const columns = [
  {
    title: 'Session ID',
    dataIndex: 'session_id',
    width: 80
  },
  {
    title: 'IP1',
    dataIndex: 'ip1',
    width: 150
  },
  {
    title: 'IP1 Port',
    dataIndex: 'ip1_port',
    width: 100
  },
  {
    title: 'IP2',
    dataIndex: 'ip2',
    width: 150
  },
  {
    title: 'IP2 Port',
    dataIndex: 'ip2_port',
    width: 100
  },
  {
    title: 'Session 開始時間',
    dataIndex: 'start_time',
    width: 230,
    render: (value) => {
      const split = value.toString().split('.')
      return (`${dayjs(value * 1000).format('YYYY-MM-DD HH:mm:ss')}.${split[1]}`)
    }
  },
  {
    title: 'Session 結束時間',
    dataIndex: 'end_time',
    width: 230,
    render: (value) => {
      const split = value.toString().split('.')
      return (`${dayjs(value * 1000).format('YYYY-MM-DD HH:mm:ss')}.${split[1]}`)
    }
  },
  {
    title: '應用層協議',
    dataIndex: 'app_proto',
    width: 120
  },
  {
    title: '數據包總大小',
    dataIndex: 'total_bytes',
    width: 120
  },
  {
    title: '數據包總數量',
    dataIndex: 'packet_count',
    width: 120
  },
];


const SessionPage = () => {
  const { type } = useParams();

  const [loading, setLoading] = useState(false);
  const [currentPage, setCurrentPage] = useState(1);
  const [total, setTotel] = useState(0);
  const [pageSize, setPageSize] = useState(30);
  const [dataList, setDataList] = useState([]);

  const loadData = async () => {
    setLoading(true);

    let proto = '';
    if (type === 'tcp') {
        proto = "TCP"
      } else if (type === 'udp') {
        proto = "UDP"
      } else if (type === 'dns') {
        proto = "DNS"
      } else if (type === 'http') {
        proto = "HTTP"
      } else if (type === 'tls') {
        proto = "TLS"
      } else if (type === 'ssh') {
        proto = "SSH"
      }

    // 發送Post，後面是請求的參數
    const _data = await apiPost('/api/getSessionList', {
      "pageSize": pageSize,
      "pageNum": currentPage,
      "proto": proto
    })
    console.log(_data.data);
    setDataList(_data.data);
    setTotel(_data.total);
    setLoading(false);
  }

  useEffect(() => {
    loadData();
  }, [currentPage, pageSize, type]);

  return (
    <div>
      <Table 
        columns={columns}
        data={dataList}
        loading={loading}
        pagination={false}
        scroll={{y: 600}}
        />
      <Pagination 
        current={currentPage}
        showTotal
        sizeCanChange
        total={total}
        pageSize={pageSize}
        onChange={(page) => setCurrentPage(page)}
        onPageSizeChange={(size) => setPageSize(size)}
        style={{ marginTop: 16, textAlign: 'right'}}
      />
    </div>
  )
};

export default SessionPage;
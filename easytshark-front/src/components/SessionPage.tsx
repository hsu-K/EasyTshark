import React from 'react';
import { useState, useEffect } from 'react';
import { Table, TableColumnProps, Pagination } from '@arco-design/web-react';
import { Typography, Tag, Link } from '@arco-design/web-react';
import { Button } from '@arco-design/web-react';
import dayjs from 'dayjs';
import { apiPost } from '../Api.ts';

const { Text, Ellipsis } = Typography;

const columns: TableColumnProps[] = [
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
    title: 'IP1端口',
    dataIndex: 'ip1_port',
    width: 100
  },
  {
    title: 'IP2',
    dataIndex: 'ip2',
    width: 150
  },
  {
    title: 'IP2端口',
    dataIndex: 'ip2_port',
    width: 100
  },
  {
    title: 'Session開始時間',
    dataIndex: 'start_time',
    width: 230,
    render: (value) => {
      const split = value.toString().split('.')
      return (`${dayjs(value * 1000).format('YYYY-MM-DD HH:mm:ss')}.${split[1]}`)
    }
  },
  {
    title: 'Session結束時間',
    dataIndex: 'end_time',
    width: 230,
    render: (value) => {
      const split = value.toString().split('.')
      return (`${dayjs(value * 1000).format('YYYY-MM-DD HH:mm:ss')}.${split[1]}`)
    }
  },
  {
    title: '應用協議',
    dataIndex: 'app_proto',
    width: 120
  },
  {
    title: '封包總大小',
    dataIndex: 'total_bytes',
    width: 120
  },
  {
    title: '封包總數量',
    dataIndex: 'packet_count',
    width: 120
  },
  {
    className: 'custom-arco-table-row-action',
    fixed: 'right',
    title: '操作',
    align: 'center' as const,
    width: 120,
    dataIndex: 'operations',
    render: (_, record) => (
      <div
        onClick={(e) => {
          const params = new URLSearchParams({ ...record }).toString();
          e.stopPropagation();
          localStorage.setItem(`row${record.session_id}`, JSON.stringify(record))
          window.open(`/detail?sessionId=${record.session_id}`)
        }}
      >
        <Button type='outline' size='mini'>Session詳情</Button>
      </div>
    ),
  },
];


const SessionPage = (props) => {
  const [loading, setLoading] = useState(false);
  const [currentPage, setCurrentPage] = useState(1);
  const [total, setTotal] = useState(0);
  const [pageSize, setPageSize] = useState(30);
  const [dataList, setDataList] = useState([])

  const loadData = async () => {
    setLoading(true);

    let proto = '';
    if (props.match.params.type === 'tcp') {
      proto = "TCP"
    } else if (props.match.params.type === 'udp') {
      proto = "UDP"
    } else if (props.match.params.type === 'dns') {
      proto = "DNS"
    } else if (props.match.params.type === 'http') {
      proto = "HTTP"
    } else if (props.match.params.type === 'tls') {
      proto = "TLS"
    } else if (props.match.params.type === 'ssh') {
      proto = "SSH"
    }

    const _data = await apiPost('/api/getSessionList', {
      "pageSize": pageSize,
      "pageNum": currentPage,
      "proto": proto
    })
    console.log(_data.data)
    setDataList(_data.data)
    setTotal(_data.total);
    setLoading(false);
  }

  useEffect(() => {
    loadData();
  }, [currentPage, pageSize, props.match.params.type])

  return (
    <div>
      <Table
        columns={columns}
        data={dataList}
        loading={loading}
        pagination={false}
        scroll={{ y: 600 }}
      />
      <Pagination
        current={currentPage}
        showTotal={(total: Number) => `共 ${total} 筆`}
        total={total}
        pageSize={pageSize}
        onChange={(page) => setCurrentPage(page)}
        style={{ marginBottom: 20 }}
        pageItemStyle={{ background: 'var(--color-bg-2)', marginRight: 2 }}
        activePageItemStyle={{ background: 'var(--color-fill-2)' }}
      />
    </div>
  )

};

export default SessionPage;
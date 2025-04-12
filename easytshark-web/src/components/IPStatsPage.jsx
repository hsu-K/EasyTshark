import React, { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';

import { Table, TableColumnProps, Pagination } from '@arco-design/web-react';
import { Typography, Tag, Link } from '@arco-design/web-react';
import dayjs from 'dayjs';
import { apiPost } from '../Api.js';

const { Text, Ellipsis } = Typography;

const columns = [
    {
        title: 'IP',
        dataIndex: 'ip',
        width: 150
    },
    {
        title: '協議',
        dataIndex: 'proto',
        width: 200,
        render: (proto) => {
            if (typeof proto === 'string') {
                
                const protocols = proto.split(',').filter(p => p.trim() !== '');
                //console.log(protocols); // Log the protocols array
                // Skip empty protocol entries
                return (
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '4px' }}>
                        {protocols.map((protocol, index) => (
                            <Tag key={index} color="green">{protocol.trim()}</Tag>
                        ))}
                    </div>
                );
            }
            return proto;
        }
    },
    {
        title: 'Ports',
        dataIndex: 'ports',
        width: 200,
        render: (ports) => {
            if (Array.isArray(ports)) {
                return (
                    <div style={{ display: 'flex', flexWrap: 'wrap', gap: '4px' }}>
                        {ports.map((port, index) => (
                            <Tag key={index} color="blue">{port}</Tag>
                        ))}
                    </div>
                );
            }
            return ports;
        }
    },
    {
        title: '最早開始時間',
        dataIndex: 'earliest_time',
        width: 230,
        render: (value) => {
            const split = value.toString().split('.')
            return (`${dayjs(value * 1000).format('YYYY-MM-DD HH:mm:ss')}.${split[1]}`)
        }
    },
    {
        title: '最新的時間',
        dataIndex: 'latest_time',
        width: 230,
        render: (value) => {
            const split = value.toString().split('.')
            return (`${dayjs(value * 1000).format('YYYY-MM-DD HH:mm:ss')}.${split[1]}`)
        }
    },
    {
        title: '發送數據包總數',
        dataIndex: 'total_sent_packets',
        width: 120
    },
    {
        title: '發送數據包總大小',
        dataIndex: 'total_sent_bytes',
        width: 120
    },
    {
        title: '接收數據包總數',
        dataIndex: 'total_recv_packets',
        width: 120
    },
    {
        title: 'TCP session 總數',
        dataIndex: 'tcp_session_count',
        width: 120
    },
    {
        title: 'UDP session 總數',
        dataIndex: 'udp_session_count',
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

    // 發送Post，後面是請求的參數
    const _data = await apiPost('/api/getIPStatsList', {
      "pageSize": pageSize,
      "pageNum": currentPage,
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
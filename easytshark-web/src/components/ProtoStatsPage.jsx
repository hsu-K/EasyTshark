import React, { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';

import { Table, TableColumnProps, Pagination } from '@arco-design/web-react';
import { Typography, Tag, Link } from '@arco-design/web-react';
import dayjs from 'dayjs';
import { apiPost } from '../Api.js';

const { Text, Ellipsis } = Typography;

const columns = [
    {
        title: '協議',
        dataIndex: 'proto',
        width: 150,
        render: (proto) => {
            
            return (
                <div style={{ display: 'flex', flexWrap: 'wrap', gap: '4px' }}>
                    <Tag color="green">{proto.trim()}</Tag>
                </div>
            );
            // return proto;
        }
    },
    {
        title: '總封包數',
        dataIndex: 'total_packet',
        width: 150,
    },
    {
        title: '總封包大小',
        dataIndex: 'total_bytes',
        width: 150,
    },
    {
        title: '總Session數',
        dataIndex: 'session_count',
        width: 150,
    },
    {
        title: '協議描述',
        dataIndex: 'proto_description',
        width: 600,
    }
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
    const _data = await apiPost('/api/getProtoStatsList', {
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
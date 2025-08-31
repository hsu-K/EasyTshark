import React from 'react';
import { useState, useEffect } from 'react';
import { Table, TableColumnProps, Pagination } from '@arco-design/web-react';
import { Typography, Tag, Link } from '@arco-design/web-react';
import dayjs from 'dayjs';
import { apiPost } from '../Api.ts';


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

const ProtoStatsPage = (props) => {
  const [loading, setLoading] = useState(false);
  const [currentPage, setCurrentPage] = useState(1);
  const [total, setTotal] = useState(0);
  const [pageSize, setPageSize] = useState(30);
  const [dataList, setDataList] = useState([])

  const loadData = async () => {
    setLoading(true);

    const _data = await apiPost('/api/getProtoStatsList', {
      "pageSize": pageSize,
      "pageNum": currentPage,
    })
    console.log(_data.data)
    setDataList(_data.data)
    setTotal(_data.total);
    setLoading(false);
  }

  useEffect(() => {
    loadData();
  }, [currentPage, pageSize])

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

export default ProtoStatsPage;
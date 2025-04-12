import React, { useState, useEffect, forwardRef } from 'react';
import { useParams } from 'react-router-dom';

import { Table, TableColumnProps, Pagination } from '@arco-design/web-react';
import { Typography, Tag, Link, Tree, Switch } from '@arco-design/web-react';
import dayjs from 'dayjs';
import { Resizable } from 'react-resizable';
import { apiPost } from '../Api.js';

const { Text, Ellipsis } = Typography;

const initalColumns = [
  {
    title: '序號',
    dataIndex: 'frame_number',
    width: 80
  },
  {
    title: '時間',
    dataIndex: 'timestamp',
    width: 250,
    render: (value) => {
      const split = value.toString().split('.');
      const s = `${dayjs(value * 1000).format('YYYY-MM-DD HH:mm:ss')}.${split[1]}`
      return <Ellipsis showTooltip>{s}</Ellipsis>
    }
  },
  {
    title: '來源IP',
    dataIndex: 'src_ip',
    width: 180
  },
  {
    title: '來源Port',
    dataIndex: 'src_port',
    width: 150
  },
  {
    title: '目的IP',
    dataIndex: 'dst_ip',
    width: 180
  },
  {
    title: '目的Port',
    dataIndex: 'dst_port',
    width: 150
  },
  {
    title: '數據包大小',
    dataIndex: 'len',
    width: 150
  },
  {
    title: '協議',
    dataIndex: 'protocol',
    width: 120
  },
  {
    title: '訊息',
    dataIndex: 'info',
    width: 350
  },
];


const CustomResizeHandle = forwardRef((props, ref) => {
  const { handleAxis, ...restProps } = props;
  return (
    <span
      ref={ref}
      className={`react-resizable-handle react-resizable-handle-${handleAxis}`}
      {...restProps}
      onClick={(e) => {
        e.stopPropagation();
      }}
    />
  );
});

const ResizableTitle = (props) => {
  const { onResize, width, ...restProps } = props;

  if (!width) {
    return <th {...restProps} />;
  }

  return (
    <Resizable
      width={width}
      height={0}
      handle={<CustomResizeHandle />}
      onResize={onResize}
      draggableOpts={{
        enableUserSelectHack: false,
      }}
    >
      <th {...restProps} />
    </Resizable>
  );
};

const DataPacketPage = () => {
  const { type } = useParams();

  const [loading, setLoading] = useState(false);
  const [currentPage, setCurrentPage] = useState(1);
  const [total, setTotel] = useState(0);
  const [pageSize, setPageSize] = useState(30);
  const [dataList, setDataList] = useState([]);

  function handleResize(index) {
    return (e, { size }) => {
      setColumns((prevColumns) => {
        const nextColumns = [...prevColumns];
        nextColumns[index] = { ...nextColumns[index], width: size.width };
        return nextColumns;
      });
    };
  }

  const [columns, setColumns] = useState(
    initalColumns.map((column, index) => {
      if (column.width) {
        return {
          ...column,
          onHeaderCell: (col) => ({
            width: col.width,
            onResize: handleResize(index),
          }),
        };
      }
      return column;
    })
  );

  const loadData = async () => {
    setLoading(true);

    let proto = '';
    if(type === 'arp'){
      proto = 'arp';
    }
    else if(type === 'icmp'){
      proto = 'icmp';
    }
    else if(type === 'icmpv6'){
      proto = 'icmpv6';
    }
    else{
      proto = '';
    }

    // 發送Post，後面是請求的參數
    const _data = await apiPost('/api/getPacketList', {
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


  const [currentRowId, setCurrentRowId] = useState(1);

  const [treeData, setTreeData] = useState([]);
  
  // 定義一個生成唯一ID的閉包
  const makeIdGenerator = () => {
    let idCounter = 0;
    return () => ++idCounter;
  }

  // 創建一個ID生成器的實例
  const generateId = makeIdGenerator();
  const addUniqueID = (node) => {
    const updateNode = { ...node, id: generateId().toString()};
    if(Array.isArray(updateNode.field) && updateNode.field.length > 0){
      updateNode.field = updateNode.field.map(addUniqueID);
    }
    return updateNode;
  }

  const renderTitle = (node) => {
    const title = node.dataRef.showname || node.dataRef.name || node.dataRef.show;
    if (node._level === 0) {
      return <span style={{ color: 'rgb(var(--primary-5))' }}>{title}</span>;
    }
    return title;
  };


  const loadPacketDetail = async () => {
    const _data = await apiPost('/api/getPacketDetail', {
      "frameNumber": currentRowId
    });
    const tree = _data?.data.proto.map(addUniqueID);
    setTreeData(tree);
  }

  useEffect(() => {
    loadPacketDetail();
  }, [currentRowId])

  return (
    <div>
      <Table 
        className='table-demo-resizable-column'

        components={{
          header:{
            cell: ResizableTitle,
          }
        }}

        columns={columns}
        data={dataList}
        loading={loading}
        pagination={false}
        rowkey="frame_number"
        // rowClassName={currentRowId}

        // 通過scroll來設置表格的高度和寬度
        scroll={{y: '50vh', x: '100%'}}
        onRow={(record) => ({
          onClick: () => {
            setCurrentRowId(record.frame_number);
          }
        })}

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
      <Tree
        // selectedKeys={selectedKeys}
        treeData={treeData}
        blockNode
        showLine
        autoExpandParent={false}
        fieldNames={{
          key: 'id',
          title: 'showname',
          children: 'field',
        }}
        renderTitle={renderTitle}
      />

    </div>
  )
};

export default DataPacketPage;
import React, { useState, useEffect, forwardRef, useImperativeHandle, useRef } from 'react';
import { useParams } from 'react-router-dom';

import { Table, TableColumnProps, Pagination } from '@arco-design/web-react';
import { Typography, Tag, Link, Tree, Switch } from '@arco-design/web-react';
import dayjs from 'dayjs';
import { Resizable } from 'react-resizable';
import { apiPost, apiGet } from '../Api.js';
import '../style/global.css';

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

const DataPacketPage = forwardRef((props, ref) => {
  // 因為有使用match直接傳構建的參數，因此使用props.match.params來獲取參數，而不是useParams
  // const { type } = useParams();
  const params = useParams();
  const type = props.match?.params?.type || params?.type;

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
    setDataList([]);
    setTotel(0);

    let proto = '';
    let sessionId = 0;
    if (type === 'arp') {
      proto = 'arp';
    }
    else if (type === 'icmp') {
      proto = 'icmp';
    }
    else if (type === 'icmpv6') {
      proto = 'icmpv6';
    }
    else if (type === 'detail') {
      console.log("this is detail")
      sessionId = parseInt(props.match.params.sessionId);
    }
    else {
      proto = '';
    }
    console.log(sessionId);
    // 發送Post，後面是請求的參數
    const _data = await apiPost('/api/getPacketList', {
      "pageSize": pageSize,
      "pageNum": currentPage,
      "proto": proto,
      "session_id": sessionId
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

  // 定義一個生成唯一ID的閉包，每次呼叫這個函數時idCounter會加1
  const makeIdGenerator = () => {
    let idCounter = 0;
    return () => ++idCounter;
  }

  // 創建一個ID生成器的實例
  const generateId = makeIdGenerator();
  const addUniqueID = (node) => {
    // 為其加上唯一的ID
    const updateNode = { ...node, id: generateId().toString() };
    if (Array.isArray(updateNode.field) && updateNode.field.length > 0) {
      // 如果node有子節點，則為其也加上ID，使用map函數來遍歷
      updateNode.field = updateNode.field.map(addUniqueID);
    }
    return updateNode;
  }

  const renderTitle = (node) => {
    const title = node.dataRef.showname || node.dataRef.name || node.dataRef.show;
    // 如果該節點層級為0，即根節點，就渲染成有特定樣式的Span元素
    if (node._level === 0) {
      return <span style={{ color: 'rgb(var(--primary-5))' }}>{title}</span>;
    }
    return title;
  };


  const loadPacketDetail = async () => {
    const _data = await apiPost('/api/getPacketDetail', {
      "frameNumber": currentRowId
    });
    // 遍歷proto，並未其加上唯一的ID
    // const tree = _data?.data.proto.map(addUniqueID);
    // setTreeData(tree);
    // transformHexData(_data?.data?.hexdata)
    // 增加對 proto 的檢查
    if (_data?.data?.proto) {
      // 遍歷proto，並為其加上唯一的ID
      const tree = _data.data.proto.map(addUniqueID);
      setTreeData(tree);
    } else {
      console.warn('收到的封包詳情中 proto 為空');
      setTreeData([]); // 設置為空陣列避免渲染錯誤
    }

    // 增加對 hexdata 的檢查
    if (_data?.data?.hexdata) {
      transformHexData(_data.data.hexdata);
    } else {
      // 處理沒有 hexdata 的情況
      setOffsetData([]);
      setHexData([]);
      setAscData([]);
    }
  }

  useEffect(() => {
    loadPacketDetail();
  }, [currentRowId])

  // 十六進制數據
  const [offsetData, setOffsetData] = useState([]);
  const [hexData, setHexData] = useState([]);
  const [ascData, setAscData] = useState([]);

  const hexCharCodeToStr = (hex) => {
    if (!hex) return '';

    // 移除可能的空格
    const trimmedHex = hex.replace(/\s+/g, '');

    // 確保字串長度是偶數，否則無法正確解析
    if (trimmedHex.length % 2 !== 0) {
      throw new Error('Invalid hex string');
    }

    let str = '';
    for (let i = 0; i < trimmedHex.length; i += 2) {
      // 每兩個字符解析為一個字元
      const charCode = parseInt(trimmedHex.substr(i, 2), 16);
      str += String.fromCharCode(charCode);
    }

    return str;
  };

  const strTwoSplit = (hex) => {
    if (!hex) return '';

    // 移除可能的空格
    const trimmedHex = hex.replace(/\s+/g, '');

    // 確保字串長度是偶數，否則無法正確分割
    if (trimmedHex.length % 2 !== 0) {
      throw new Error('Invalid hex string');
    }

    // 按每兩個字符分割
    const result = [];
    for (let i = 0; i < trimmedHex.length; i += 2) {
      result.push(trimmedHex.substr(i, 2));
    }

    return result.join(','); // 返回以逗號分隔的字串
  };

  const padNumber = (num, length, padChar = '0') => {
    const str = num.toString();
    return str.length >= length ? str : padChar.repeat(length - str.length) + str;
  };

  const transformHexData = (hex) => {
    const offsetData = [];
    const hexData = [];
    const ascData = [];

    // 先把16進制的數據轉換成字符串，其值存在label中，並給予key
    const params = hexCharCodeToStr(hex)
    for (let i = 0; i < params.length; i++) {
      const asc = { label: params[i], key: i, show: false }

      // Check if character is printable ASCII (codes 32-126)
      // Replace non-printable characters with '.'
      if (params.charCodeAt(i) <= 32 || params.charCodeAt(i) >= 126) {
        asc.label = '.'
        asc.show = true
      }
      ascData.push(asc)
    }
    // 分割16進制數據，並給予key
    const arr = strTwoSplit(hex).split(',')
    arr.map((item, index) => {
      hexData.push({ label: item, key: index })
    })
    // 判斷總偏移量是否為整數
    let offset = ascData.length / 16
    if (!Number.isInteger(offset)) {
      offset = parseInt(offset) + 1
    }
    // 計算偏移量，不夠4位數，就在左側補0
    for (let i = 0; i < offset; i++) {
      const hex = padNumber((i * 16).toString(16), 4, '0')
      offsetData.push({ label: hex, key: i + 1 })
    }
    setOffsetData(offsetData)
    setHexData(hexData)
    setAscData(ascData)

  }

  const [selectedLeftHex, setSelectedLeftHex] = useState([]);
  const [selectedRightHex, setSelectedRightHex] = useState([]);
  const [selectedKeys, setSelectedKeys] = useState();

  const handleNodeClick = (node, data) => {
    // 設置選中的節點
    setSelectedKeys(node);

    // 計算選中的範圍
    const pos = parseInt(data.node?.props.pos);
    const size = parseInt(data.node?.props.size) + pos;
    const leftArr = [];
    const rightArr = [];
    for (let i = pos; i < size; i++) {
      leftArr.push(i);
    }
    setSelectedRightHex([...leftArr]);

    // 計算8位偏移量是否選中
    if (data.node && parseInt(data.node?.props.size) !== 0) {
      const posLeft = !Number.isInteger((pos + 1) / 16) ? ((pos + 1) / 16 >> 0) + 1 : (pos + 1) / 16
      const sizeLeft = !Number.isInteger(size / 16) ? (size / 16 >> 0) + 1 : size / 16
      for (let i = posLeft; i <= sizeLeft; i++) {
        rightArr.push(i)
      }
      setSelectedLeftHex([...rightArr])
    } else {
      setSelectedLeftHex([])
    }
  }

  useImperativeHandle(ref, () => ({
    setCurrentRowId: setCurrentRowId,
  }));


  const timerRef = useRef(0);
  useEffect(() => {
    const checkStatus = async () => {
      const _data = await apiGet('/api/getWorkStatus');
      if(_data.data.workStatus === 2){
        loadData();
      }
      else{
        console.log("clearInterval: ", timerRef.current);
        clearInterval(timerRef.current);
        timerRef.current = 0;
      }
    };

    // console.log("type: ", type);
    if(type === 'detail'){
      let id = setInterval(checkStatus, 2000);
      timerRef.current = id;
    }


    return () => {
      if (timerRef.current !== 0) {
        clearInterval(timerRef.current);
        timerRef.current = 0;
      }
    };
  }, [])

  return (
    <div>
      <Table
        className='table-demo-resizable-column'
        components={{
          header: {
            cell: ResizableTitle,
          }
        }}

        columns={columns}
        data={dataList}
        loading={loading}
        pagination={false}
        rowkey="frame_number"
        rowClassName={(record) => record.frame_number === currentRowId ? 'selected-row' : ''}

        // 通過scroll來設置表格的高度和寬度
        scroll={{ y: '50vh', x: '100%' }}
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
        style={{ marginTop: 16, textAlign: 'right' }}
      />
      <div className='detail-pannel'>
        <div className='proto-tree'>
          <Tree
            selectedKeys={selectedKeys}
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
            onSelect={handleNodeClick}
          />
        </div>
        <div className="hex-data">
          <div className="offset">
            <div className="left pdt8 pdb8">
              {offsetData.map(item => {
                return <span key={item.key} className={selectedLeftHex.indexOf(item.key) !== -1 ? 'selected' : ''}>{item.label}</span>
              })}
            </div>
            <div className="center">
              {hexData.map(item => {
                return <span id={item.key} key={item.key} className={selectedRightHex.indexOf(item.key) !== -1 ? 'selected' : ''}>{item.label}</span>
              })}
            </div>
            <div className="right">
              {ascData.map(item => {
                return <span key={item.key} className={selectedRightHex.indexOf(item.key) !== -1 ? 'selected' : ''} style={item.show ? { color: '#bbb' } : {}}>{item.label}</span>
              })}
            </div>
          </div>
        </div>

      </div>

    </div>
  )
});

export default DataPacketPage;
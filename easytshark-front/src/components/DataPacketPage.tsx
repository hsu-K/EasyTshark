import React from 'react';
import { useState, useEffect, forwardRef, useImperativeHandle, useRef } from 'react';
import { Table, TableColumnProps, Pagination, Divider } from '@arco-design/web-react';
import { Typography, Tag, Link, Switch } from '@arco-design/web-react';
import dayjs from 'dayjs';
import { apiPost, apiGet } from '../Api.ts';
import { Tree } from '@arco-design/web-react';

const { Text, Ellipsis } = Typography;

const columns: TableColumnProps[] = [
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
      const split = value.toString().split('.')
      return (`${dayjs(value * 1000).format('YYYY-MM-DD HH:mm:ss')}.${split[1]}`)
    }
  },
  {
    title: '源IP',
    dataIndex: 'src_ip',
    width: 180
  },
  {
    title: '來源端口',
    dataIndex: 'src_port',
    width: 150
  },
  {
    title: '目的IP',
    dataIndex: 'dst_ip',
    width: 180
  },
  {
    title: '目的端口',
    dataIndex: 'dst_port',
    width: 150
  },
  {
    title: '封包大小',
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

interface DataPacketPageProps {
  match: {
    params: {
      type: string;
      sessionId: string;
    }
  };
}

interface DataPacketPageHandle {
  setCurrentRowId: (id: number) => void;
}


const DataPacketPage = forwardRef((props: DataPacketPageProps, ref: DataPacketPageHandle) => {
  const [currentPage, setCurrentPage] = useState(1);
  const [loading, setLoading] = useState(false);
  const [total, setTotal] = useState(0);
  const [pageSize, setPageSize] = useState(30);
  const [dataList, setDataList] = useState([])


  const loadData = async (Cur_page: number) => {
    setLoading(true);
    let sessionId = 0;
    let proto = '';
    if (props.match.params.type === 'arp') {
      proto = "ARP"
    } else if (props.match.params.type === 'icmp') {
      proto = "ICMP"
    } else if (props.match.params.type === 'icmpv6') {
      proto = "ICMPv6"
    } else if (props.match.params.type === 'detail') {
      sessionId = parseInt(props.match.params.sessionId)
    } else {
      proto = ""
    }
    // console.log("Proto: ", proto, " Session ID: ", sessionId);

    const _data = await apiPost('/api/getPacketList', {
      "pageSize": pageSize,
      "pageNum": Cur_page,
      "proto": proto,
      "session_id": sessionId
    })
    setDataList(_data.data)
    setTotal(_data.total);
    setLoading(false);
  }

  useEffect(() => {
    loadData(currentPage);
  }, [currentPage, pageSize, props.match.params.type])

  // ID 生成器
  const makeIdGenerator = () => {
    let idCounter = 0;
    return () => ++idCounter;
  };

  const generateId = makeIdGenerator();
  const addUniqueID = (node) => {
    const updatedNode = { ...node, id: generateId().toString() };
    if (Array.isArray(updatedNode.field) && updatedNode.field.length > 0) {
      updatedNode.field = updatedNode.field.map(addUniqueID);
    }
    return updatedNode;
  };

  // Tree
  const [treeData, setTreeData] = useState([]);
  const [currentRowId, setCurrentRowId] = useState(1);

  useImperativeHandle(ref, () => ({
    setCurrentRowId: setCurrentRowId,
  }), []);
  // const [selectedKeys, setSelectedKeys] = useState([]);
  const loadPacketDetail = async () => {
    const _data = await apiPost('/api/getPacketDetail', {
      "frameNumber": currentRowId
    });
    const tree = _data?.data?.proto.map(addUniqueID);
    setTreeData(tree);
    if (_data?.data?.hexdata) {
      transformHexData(_data.data.hexdata);
    }
  }

  useEffect(() => {
    loadPacketDetail();
  }, [currentRowId])

  const renderTitle = (node) => {
    const title = node.dataRef.showname || node.dataRef.name || node.dataRef.show;
    if (node._level === 0) {
      return <span style={{ color: 'rgb(var(--primary-5))' }}>{title}</span>;
    }
    return title;
  };

  const rowClassName = (record) => {
    return record.frame_number === currentRowId ? 'table-row-selected' : '';
  };


  // Hex Data View
  const [offsetData, setOffsetData] = useState([])
  const [hexData, setHexData] = useState([])
  const [ascData, setAscData] = useState([])

  // 把hex轉成字串
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

  // 每兩個字符分割字串，並以逗號分隔
  const strTwoSplit = (hex) => {
    if (!hex) return '';

    // 移除可能的空格
    const trimmedHex = hex.replace(/\s+/g, '');

    // 確保字串長度是偶數，否則無法正確分割
    if (trimmedHex.length % 2 !== 0) {
      throw new Error('Invalid hex string');
    }

    // 按每兩個字符分割
    const result: string[] = [];
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
    const _offsetData: { label: string; key: number; }[] = []
    const _hexData: { label: string; key: number; }[] = []
    const _ascData: { label: string; key: number; show: boolean }[] = []
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
      _ascData.push(asc)
    }
    // 分割16進制數據，並給予key
    const arr = strTwoSplit(hex).split(',')
    arr.map((item, index) => {
      _hexData.push({ label: item, key: index })
    })
    // 判斷總偏移量是否為整數
    let offset = _ascData.length / 16
    if (!Number.isInteger(offset)) {
      offset = Math.ceil(offset)
    }
    // 計算偏移量，不夠4位數，就在左側補0
    for (let i = 0; i < offset; i++) {
      const hex = padNumber((i * 16).toString(16), 4, '0')
      _offsetData.push({ label: hex, key: i + 1 })
    }
    setOffsetData(_offsetData)
    setHexData(_hexData)
    setAscData(_ascData)
    // console.log(_offsetData, _hexData, _ascData)
  }



  const [selectedLeftHex, setSelectedLeftHex] = useState([]);
  const [selectedRightHex, setSelectedRightHex] = useState([]);
  const [selectedKeys, setSelectedKeys] = useState();

  const handleNodeClick = (node, data) => {
    console.log('Selected node:', node, data);
    // 設置選中的節點
    setSelectedKeys(node);

    // 計算選中的範圍
    const pos = parseInt(data.node?.props.pos);
    const size = parseInt(data.node?.props.size) + pos;
    const leftArr: number[] = [];
    const rightArr: number[] = [];
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

  const currentPageRef = useRef(1);

  // 當 currentPage 改變時更新 ref
  useEffect(() => {
    currentPageRef.current = currentPage;
  }, [currentPage]);


  const timerRef = useRef(0);
  useEffect(() => {
    const checkStatus = async () => {

      const _data = await apiGet('/api/getWorkStatus');
      // 2 為抓包中
      // console.log('Status: ', _data.data.workStatus);
      if (_data.data.workStatus === 2) {
        const currentPageNum = currentPageRef.current;
        // console.log("使用頁碼:", currentPageNum);
        loadData(currentPageNum);
      }
      else {
        console.log("clearInterval: ", timerRef.current);
        clearInterval(timerRef.current);
        timerRef.current = 0;
      }
    };

    if (props.match.params.type !== 'detail') {
      let id = setInterval(checkStatus, 2000);
      timerRef.current = id;
    }

    return () => {
      if (timerRef.current !== 0) {
        clearInterval(timerRef.current);
        timerRef.current = 0;
      }
    }
  }, []);



  return (
    <div>
      <Table
        columns={columns}
        data={dataList}
        loading={loading}
        pagination={false}
        scroll={{ y: 600 }}
        rowKey="frame_number"
        rowClassName={rowClassName}
        onRow={(record) => ({
          onClick: () => {
            setCurrentRowId(record.frame_number);
            // setSelectedKeys([record.frame_number]);
            // loadPacketDetail();
            // console.log('Row clicked:', record);
          },
        })}
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
      <div className="detail-pannel">
        <div className="proto-tree">
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
                // return <span key={item.key} >{item.label}</span>
                return <span key={item.key} className={selectedLeftHex.indexOf(item.key) !== -1 ? 'selected' : ''}>{item.label}</span>
              })}
            </div>
            <div className="center">
              {hexData.map(item => {
                // return <span id={item.key} key={item.key}>{item.label}</span>
                return <span id={item.key} key={item.key} className={selectedRightHex.indexOf(item.key) !== -1 ? 'selected' : ''}>{item.label}</span>
              })}
            </div>
            <div className="right">
              {ascData.map(item => {
                // return <span key={item.key} style={item.show ? { color: '#bbb' } : {}}>{item.label}</span>
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


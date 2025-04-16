import React, { useState, useEffect } from 'react';
import { Radio, Spin, Empty } from '@arco-design/web-react'
import { apiPost } from '../../Api';
import { formatFileSize } from '../../util.js';

const RadioGroup = Radio.Group;

const SessionDataStreamTab = ({ sessionId }) => {
  const [data, setData] = useState([]);
  const [record, setRecord] = useState(JSON.parse(localStorage.getItem(`row${sessionId}`)));

  const getSessionDataStream = async (value) => {
    setLoading(true);
    const values = await apiPost('/api/getSessionDataStream', {
      session_id: parseInt(sessionId)
    })
    setCountObj(values?.count);
    const datas = value === 'b' ? values?.data.filter(v => v.srcNode === values?.count?.node0) :
      value === 'c' ? values?.data.filter(v => v.srcNode === values?.count?.node1) : values?.data;

    datas.map(item => {
      if (item.hexData !== '') {
        const arr = getAscTransformation(item.hexData);
        item.offsetData = arr.offsetData;
        item.ascData = arr.ascData;
        item.hexAscData = arr.hexAscData;
        item.hexData = arr.hexData;
        item.formattedHexData = arr.formattedHexData;
      }
    })

    setData(datas);
    // if (pageNum === 1){
    //   setData(datas);
    // }
    // else{
    //   setData([...data, ...datas]);
    // }
    setLoading(false);
  }

  useEffect(() => {
    getSessionDataStream();
  }, [])

  const hexCharCodeToAscStr = (hex) => {
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
      if (charCode <= 32 || charCode >= 126) {
        str += '.';
      }
      else {
        str += String.fromCharCode(charCode);
      }
    }

    return str;
  };
  const strSplit = (hex, num, spc = ',') => {
    if (!hex) return '';

    // 移除可能的空格
    const trimmedHex = hex.replace(/\s+/g, '');

    // 按每num個字符分割
    const result = [];
    let endIndex = num;
    for (let i = 0; i < trimmedHex.length; i += num) {
      endIndex = Math.min(endIndex, trimmedHex.length)
      result.push(trimmedHex.substring(i, endIndex));
      endIndex += num;
    }

    return result.join(spc); // 返回以逗號分隔的字串
  };

  const padNumber = (num, length, padChar = '0') => {
    const str = num.toString();
    return str.length >= length ? str : padChar.repeat(length - str.length) + str;
  };

  const getAscTransformation = (hex) => {
    let hexData = []
    const ascData = []
    let hexAscData = []
    const offsetData = []
    const formattedHexData = [];
    // 按16位分割ascii
    const ascParams = hexCharCodeToAscStr(hex)
    ascData.push(ascParams)
    // 因為原始數據可能有','，這裡用'換'替代
    hexAscData = strSplit(ascParams, 16, '換').split('換')
    // // 按16位分割hex
    hexData = strSplit(hex, 2).split(',')

    for (let i = 0; i < hexData.length; i += 16) {
      formattedHexData.push(hexData.slice(i, i + 16));
    }
    

    for (let i = 0; i < hexAscData.length; i++) {
      const offset = padNumber((i * 16).toString(16), 8, '0')
      offsetData.push(offset)
    }
    return { hexData, ascData, offsetData, hexAscData, formattedHexData }
  }


  const [loading, setLoading] = useState(false);
  const [countObj, setCountObj] = useState(null);
  const [type, setType] = useState('ascii');

  return (
    <div>
      <RadioGroup defaultValue='a' style={{ marginBottom: 10 }} onChange={(value) => getSessionDataStream(value)}>
        <Radio value='a'>全部（{formatFileSize(countObj?.node0BytesCount + countObj?.node1BytesCount)}，{countObj?.totalPacketCount}個數據包）</Radio>
        <Radio value='b'><span style={{ color: '#C04545' }}>{countObj?.node0}</span> -&gt; <span style={{ color: '#319EFF' }}>{countObj?.node1}</span> ({formatFileSize(countObj?.node0BytesCount)}，{countObj?.node0PacketCount}個數據包)</Radio>
        <Radio value='c'><span style={{ color: '#319EFF' }}>{countObj?.node1}</span> -&gt; <span style={{ color: '#C04545' }}>{countObj?.node0}</span>（{formatFileSize(countObj?.node1BytesCount)}，{countObj?.node1PacketCount}個數據包）</Radio>
      </RadioGroup>
      <RadioGroup value={type} style={{ marginBottom: 10 }} type='button' onChange={(value => { setType(value); })}>
        <Radio value='ascii'>ASCII</Radio>
        <Radio value='rawData'>原始數據</Radio>
        <Radio value='hex'>HEX形式</Radio>
      </RadioGroup>
      <Spin loading={loading} block>
        {data.length === 0 ? <Empty className='mt-5' /> :
          type === 'ascii' &&
          // <div className='stream-main' ref={leftRef}>
          <div className='stream-main'>
            {data.map((item, i) => <div key={i} className={item.srcNode == countObj?.node0 ? 'send' : 'accept'}>
              {item.ascData.map((info, j) =>
                <pre key={j} style={{ marginBottom: 0, whiteSpace: 'break-spaces', fontFamily: 'Consolas, monospace' }}>{info}</pre>
              )}
            </div>)}
          </div>}
        {type === 'rawData' && <div className='stream-main'>
          {data.map((item, index) => <div key={index} className={item.srcNode == countObj?.node0 ? 'send' : 'accept'}>
            <pre style={{
              marginBottom: 0,
              whiteSpace: 'break-spaces',
              fontFamily: 'Consolas, monospace',
              lineHeight: '1.5',
              overflowWrap: 'break-word'
            }}>
              {/* 檢查 hexData 是數組還是原始字符串 */}
              {Array.isArray(item.hexData)
                ? item.hexData.join(' ') // 如果是數組，則用空格連接
                : item.hexData  // 如果已經是字符串，直接顯示
              }
            </pre>
          </div>)}
        </div>}
        {type === 'hex' && <div className='stream-main'>
          {data.map((item, index) => <div className={`${item.srcNode == countObj?.node0 ? 'send' : 'accept'} stream-offset mgb20`} key={index}>
            <div className="left">
              {item.offsetData.map((v) => <div key={v}>{v}</div>)}
            </div>
            <div className="center">
              {item.formattedHexData.map((row, i) => (
                <div key={i} style={{ whiteSpace: 'pre' }}>
                  {row.join(' ')}
                </div>
              ))}
            </div>
            <div className="right">
              {item.hexAscData.map((v) => 
              <div 
                key={v}
                style={{
                  letterSpacing: '3px',
                }}
              >{v}
              </div>
            )}

            </div>
          </div>)}

        </div>}
      </Spin>
    </div>
  )
};

export default SessionDataStreamTab;
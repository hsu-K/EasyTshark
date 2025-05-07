import React from 'react';
import EChartsReact from 'echarts-for-react';

const LineChart = ({ data }) => {
  const xData = [];
  const arr = [];
  data.forEach((item) => {
    xData.push(item.time);
    arr.push(item.bytes);
  });
  const option = {
    grid: {
      left: 0,
      right: 0,
      top: 5,
      bottom:3
    },
    xAxis: {
      type: 'category',
      boundaryGap: false,
      axisLine: { show: false }, // 不顯示x軸線
      axisTick: { show: false },  // 不顯示x軸刻度
      splitLine: {show: false}, // 不顯示x軸分割線
      data: xData,
    },
    yAxis: {
      type: 'value',
      min: 0,
      max: function(value) {
        return value.max === 0 ? 0.1 : value.max; // 如果最大值是0，則顯示微小值作為最大值
      },
      axisLine: { show: false }, // 不顯示y軸線
      axisTick: { show: false },  // 不顯示y軸刻度
      splitLine: {show: false}, // 不顯示y軸分割線
    },
    series: [{
      name: '',
      type: 'line',
      data: arr,
      symbol: 'none', // 不顯示數據點
      smooth: true, // 平滑曲線
      lineStyle: {
        width: 1,
        // color: '#FF0000', // 線條顏色
      },
      // areaStyle: {
      //   opacity: 0.1,  // 設置透明度
      //   color: {
      //     type: 'linear',
      //     x: 0, y: 0, x2: 0, y2: 1,
      //     colorStops: [
      //       { offset: 0, color: '#5470c6' },
      //       { offset: 1, color: 'rgba(84,112,198,0)' }
      //     ]
      //   }
      // }
    }]
  };
  return <EChartsReact option={option} style={{ height: 35}} />;
}

export default LineChart;
import React, { useState, useEffect } from 'react';
import axios from 'axios';

const baseUrl = "http://127.0.0.1:8080"

export const apiPost = async (url, data = {}, config = {}) => {
  try{

    // 設定分頁
    let _url = url;
    if(data["pageNum"] != undefined && data["pageSize"] != undefined){
      _url = url + "?pageSize=" + data["pageSize"] + "&pageNum=" + data["pageNum"];
    }

    // 發送POST請求
    const response = await axios.post(baseUrl + _url, data, config);
    
    // 返回響應的數據
    return response.data;
  
  } catch (error){
    console.error("API POST Error: ", error);
    throw error;
  }
}
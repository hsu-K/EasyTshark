import React, { useEffect, useState } from 'react';
import axios from 'axios';

const baseUrl = "http://127.0.0.1:8080"

export const apiPost = async (url: string , data = {}, config = {}) => {
  try {
    
    let _url = url
    if (data["pageNum"] != undefined && data["pageSize"] != undefined) {
      _url = url + '?pageSize=' + data["pageSize"] + "&pageNum=" + data["pageNum"]
    }

    
    const response = await axios.post(baseUrl + _url, data, config);
    
    
    return response.data;
  } catch (error) {
    
    console.error('Error in postData:', error);
    throw error;
  }
};

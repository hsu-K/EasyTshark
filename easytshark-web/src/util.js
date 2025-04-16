
/**
 * 計算兩個時間參數之間的時間差
 * @param {string|number} startTime - 開始時間，時間戳或時間字符串
 * @param {string|number} endTime - 結束時間，時間戳或時間字符串
 * @returns {string} - 格式化後的時間差
 */
export const calculateDuration = (startTime, endTime) => {
  // 處理輸入參數，確保可以轉換為時間戳（秒）
  // 如果是字符串，嘗試轉換為Date再獲取時間戳（毫秒），然後除以1000轉為秒
  const start = typeof startTime === 'string' ? new Date(startTime).getTime() / 1000 : Number(startTime);
  const end = typeof endTime === 'string' ? new Date(endTime).getTime() / 1000 : Number(endTime);

  // 計算時間差（秒）
  const duration = end - start;

  // 如果時間差為負或無效，返回 '0秒'
  if (isNaN(duration) || duration < 0) {
    return '0秒';
  }

  // 計算時分秒
  const seconds = Math.floor(duration) % 60;
  const minutes = Math.floor(duration / 60) % 60;
  const hours = Math.floor(duration / (60 * 60)) % 24;
  const days = Math.floor(duration / (60 * 60 * 24));

  // 構建時間差字符串
  const parts = [];
  if (days > 0) parts.push(`${days}天`);
  if (hours > 0) parts.push(`${hours}小時`);
  if (minutes > 0) parts.push(`${minutes}分鐘`);
  if (seconds > 0 || parts.length === 0) parts.push(`${seconds}秒`);

  return parts.join(' ');
};

/**
 * 將字節數格式化為易讀的形式（B、KB、MB、GB、TB）
 * @param {number} bytes - 要格式化的字節數
 * @param {number} decimals - 小數點後的位數 (默認為 2)
 * @returns {string} 格式化後的字串，例如 "1.5 MB"
 */
export const formatFileSize = (bytes, decimals = 2) => {
  if (bytes === undefined || bytes === null) {
    return "0 Bytes";
  }
  
  if (bytes === 0) {
    return "0 Bytes";
  }
  
  // 單位定義
  const k = 1024;
  const sizes = ["Bytes", "KB", "MB", "GB", "TB", "PB"];
  
  // 計算適當的單位
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  
  // 轉換為該單位的值並四捨五入到指定的小數位
  return parseFloat((bytes / Math.pow(k, i)).toFixed(decimals)) + " " + sizes[i];
};
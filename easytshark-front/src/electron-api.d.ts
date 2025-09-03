interface ElectronAPI {
  openFileDialog(): Promise<string | undefined>;
  showSavePath(): Promise<string | undefined>;
  
}

declare global {
  interface Window {
    electronAPI: ElectronAPI;
  }
}

export {};
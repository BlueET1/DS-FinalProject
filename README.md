# 資料結構期末專題 — TCP Socket 伺服器與 Huffman 壓縮

> **學習記錄用途**：此 repo 為大學資料結構課程期末專題成果，記錄學習歷程與實驗結果，供個人求職與學習參考使用。

## 專案簡介

以 C 語言實作 TCP Client-Server 通訊系統，結合 **Huffman 編碼**進行文字壓縮，並以 **cJSON** 處理 JSON 格式的資料交換。

- **課程**：資料結構（Data Structures）期末專題
- **學校**：國立中央大學 資訊電機學院學士班

---

## 功能架構

### v1（基礎版）— `sever.c` / `client.c`

- TCP socket 連線（port 8080）
- 客戶端傳送指令，伺服器回應
- 支援指令：`ls`（列出檔案）、`exit`（斷線）

### v2（進階版）— `severver2.c` / `clientver2.c`

- 保留 v1 全部功能
- 新增 **Huffman 編碼/解碼**：
  - 計算字元頻率 → 建立 Huffman 樹（min-heap + qsort）→ 產生編碼表
  - 傳輸前壓縮，接收後解壓縮
- 以 **cJSON** 序列化 Huffman 編碼表，透過 socket 傳送
- 使用 `select()` 實作 I/O 多路複用，支援 10 分鐘逾時自動斷線

---

## 檔案結構

```
.
├── sever.c          # 伺服器 v1（基礎 TCP）
├── severver2.c      # 伺服器 v2（Huffman + cJSON）
├── client.c         # 客戶端 v1
├── clientver2.c     # 客戶端 v2
├── cJSON.c          # cJSON 函式庫（Dave Gamble, MIT License）
├── cJSON.h
├── test.c           # 單元測試
├── test.json        # 測試用 JSON
└── 資料結構期末專題報告.pdf  # 完整專題報告
```

---

## 編譯與執行

**環境**：Linux / macOS（需 GCC，Windows 可用 WSL）

```bash
# 編譯 v2
gcc -o sever severver2.c -lm
gcc -o client clientver2.c -lm

# 執行（需開兩個終端機）
./sever          # 終端機 1：啟動伺服器
./client         # 終端機 2：啟動客戶端
```

---

## 核心資料結構

| 結構 | 用途 |
|------|------|
| `Node`（二元樹節點） | Huffman 樹節點，含字元、頻率、左右子樹 |
| `heap[]`（指標陣列） | 模擬 min-heap，使用 `qsort` 維護順序 |
| `cJSON` 物件 | 序列化編碼表，透過 TCP socket 傳輸 |

---

## 參考資源

1. Huffman, "A Method for the Construction of Minimum-Redundancy Codes," 1952
2. [cJSON](https://github.com/DaveGamble/cJSON) — Dave Gamble, MIT License

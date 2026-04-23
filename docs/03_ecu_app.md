# ECUアプリ 仕様

## 目的

vcan0 を受信し、正常/異常を判別してログ出力する ECU 相当アプリ。

## 配置ルール

```
targets/<name>/
├── main.cpp     # C++ 実装（ビルド成果物: app）
└── scenario.yml
```

`run_poc.sh` は `app` バイナリを起動する。

---

## C++ 実装（ecu_cpp）

- API: Linux SocketCAN（`linux/can.h`、`linux/can/raw.h`）
- ビルド: `g++ -O2 -o targets/ecu_cpp/app targets/ecu_cpp/main.cpp`
- 実行: `targets/ecu_cpp/app`

---

## 動作仕様

| 受信データ | 判定 | ログ出力例 |
|-----------|------|-----------|
| Data の全バイトが `0xFF`（DLC≥1） | 異常 | `[ABNORMAL] ID=0x123 Data=ffffffffffffffff` |
| Data が空（DLC=0） | 正常 | `[NORMAL] ID=0x123 Data=` |
| それ以外 | 正常 | `[NORMAL] ID=0x123 Data=0102030405060708` |

- ID フィルタ: `TARGET_ID=0x123` のフレームのみ処理し、他の ID は無視する
- ログは標準出力へ出力し、`run_poc.sh` が `logs/ecu.log` にリダイレクトする

## 終了

- SIGTERM または SIGINT を受け取って正常終了
- `run_poc.sh` の trap EXIT によって自動終了される

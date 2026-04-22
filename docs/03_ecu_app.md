# ECUアプリ 仕様

## 目的

vcan0 を受信し、正常/異常を判別してログ出力する最小 ECU 相当アプリ。

## 実装

- 言語: Python 3
- ライブラリ: `python-can`
- 実行方式: プロセス（Docker 不使用）

## 動作仕様

| 受信データ | 判定 | ログ出力例 |
|-----------|------|-----------|
| ID=0x123, Data=01 02 03 04 05 06 07 08 | 正常 | `[NORMAL] ID=0x123 Data=0102030405060708` |
| ID=0x123, Data=FF FF FF FF FF FF FF FF | 異常 | `[ABNORMAL] ID=0x123 Data=ffffffffffffffff` |
| その他 | 不明 | `[UNKNOWN] ID=0x??? Data=...` |

## 正常/異常の判別ロジック（PoC版）

- Data のすべてのバイトが `0xFF` → 異常
- それ以外 → 正常

## 起動・終了

- 起動: `python3 ecu_app/ecu.py`
- 終了: Ctrl+C または `run_poc.sh` からの SIGTERM
- vcan0 が存在しない場合はエラーメッセージを出力して終了コード 1

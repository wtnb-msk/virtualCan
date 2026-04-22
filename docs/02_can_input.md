# CAN入力生成 仕様

## 目的

テスト治具として vcan0 に CAN フレームを送信する。ECU として常駐させる必要はない。

## テンプレート形式

テキストファイル（`.can`）に1行1フレームの形式で記述する。

```
# 書式： <CAN_ID>#<DATA_HEX>  （can-utils cansend 形式）
123#0102030405060708
```

## 正常テンプレート（`templates/normal.can`）

- CAN ID: `0x123`
- Data: `01 02 03 04 05 06 07 08`
- 送信周期: 100ms
- 送信回数: 10回（PoC では固定）

## 異常テンプレート（`templates/abnormal.can`）

- CAN ID: `0x123`
- Data: `FF FF FF FF FF FF FF FF`
- 送信方式: 単発 or 低頻度（500ms間隔で3回）

## スクリプト仕様

### send_normal.sh

- テンプレートファイルを読み込み、100ms 間隔で繰り返し送信
- 引数なしで実行可能
- 終了後にプロセス終了

### send_abnormal.sh

- 異常テンプレートを 500ms 間隔で3回送信して終了

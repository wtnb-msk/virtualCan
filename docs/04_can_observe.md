# CAN観測・判定 仕様

## 目的

vcan0 上を流れる CAN フレームを観測し、PoC の動作を確認する。

## PoC で確認すべき項目

1. vcan0 上に CAN フレームが流れていること
2. 正常テンプレートと異常テンプレートが区別できること

## 観測手段

### candump（簡易確認用）

```bash
candump vcan0
```

ターミナルにリアルタイムでフレームを表示する。PoC の目視確認に使用。

### ECUアプリのログ（自動判定）

`ecu_app/ecu.py` が `[NORMAL]` / `[ABNORMAL]` を標準出力するため、
`run_poc.sh` 内でログファイルに保存して判定結果を確認できる。

## 出力例

```
[NORMAL]   ID=0x123 Data=0102030405060708
[ABNORMAL] ID=0x123 Data=ffffffffffffffff
[NORMAL]   ID=0x123 Data=0102030405060708
```

## 将来の拡張（CI 化時）

- ログファイルに `[ABNORMAL]` が含まれることをアサートするスクリプトを追加
- `[NORMAL]` のみのシーケンスでエラーが出ないことを確認

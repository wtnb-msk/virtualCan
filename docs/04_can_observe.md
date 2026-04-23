# CAN観測・判定 仕様

## 目的

vcan0 上を流れる CAN フレームを観測し、PoC の動作を確認する。

## 観測手段

### candump（生ログ）

```bash
candump vcan0
```

ターミナルにリアルタイムでフレームを表示する。`run_poc.sh` はバックグラウンドで起動し `logs/candump.log` に保存する。

### ECUアプリのログ（判定結果）

ECUアプリ（Python / C++）が標準出力へ `[NORMAL]` / `[ABNORMAL]` を出力し、`logs/ecu.log` に保存される。

## 出力例

```
[NORMAL]   ID=0x123 Data=0102030405060708
[NORMAL]   ID=0x123 Data=0102030405060708
[ABNORMAL] ID=0x123 Data=ffffffffffffffff
```

## CI での自動判定（check_result.sh）

```bash
bash scripts/check_result.sh logs/ecu.log
```

`logs/ecu.log` に `[NORMAL]` と `[ABNORMAL]` が両方含まれることをアサートする。
どちらか一方でも欠けていれば exit 1 で CI が失敗する。

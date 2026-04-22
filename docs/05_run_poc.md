# 起動スクリプト 仕様

## 目的

PoC 全体をワンコマンドで起動・実行・終了する。

## 使い方

```bash
./run_poc.sh
```

## 実行シーケンス

```
1. vcan0 セットアップ        (scripts/setup_vcan.sh)
2. ECUアプリ起動             (python3 ecu_app/ecu.py &)
3. candump 起動              (candump vcan0 > logs/candump.log &)
4. 正常テンプレート送信      (scripts/send_normal.sh)
5. 異常テンプレート送信      (scripts/send_abnormal.sh)
6. 待機（全送信完了まで）
7. ECUアプリ・candump 終了  (kill)
8. ログ出力・結果表示
```

## 出力ファイル

| ファイル | 内容 |
|---------|------|
| `logs/candump.log` | candump の生ログ |
| `logs/ecu.log` | ECUアプリの判定ログ |

## 終了コード

| コード | 意味 |
|--------|------|
| 0 | 正常完了（全フレーム送受信確認済み） |
| 1 | vcan0 セットアップ失敗 |
| 2 | ECUアプリ起動失敗 |

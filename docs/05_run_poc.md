# 起動スクリプト 仕様

## 目的

PoC 全体をワンコマンドで起動・実行・終了する。

## 使い方

```bash
# デフォルトターゲット（targets/ecu_basic）
sudo bash run_poc.sh

# ターゲットを指定
sudo bash run_poc.sh targets/ecu_cpp
```

## 実行シーケンス

```
1. vcan0 セットアップ          (scripts/setup_vcan.sh)
2. ECUアプリ起動（バックグラウンド）
     app.py が存在 → Python で起動
     app バイナリが存在 → バイナリを直接起動
3. candump 起動（バックグラウンド） → logs/candump.log
4. シナリオ実行                (scripts/run_scenario.sh <target>/scenario.yml)
5. trap EXIT → ECUアプリ・candump を kill
6. ログ内容を標準出力へ表示
```

## 出力ファイル

| ファイル | 内容 |
|---------|------|
| `logs/candump.log` | candump の生ログ |
| `logs/ecu.log` | ECUアプリの判定ログ |

## 終了コード

| コード | 意味 |
|--------|------|
| 0 | 正常完了 |
| 1 | vcan0 セットアップ失敗 / アプリが見つからない |
| 2 | ECUアプリ起動失敗（.venv 未作成 等） |

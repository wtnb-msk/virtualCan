# チケット一覧

> **ルール**: 各チケットの作業が完了したら `- [ ]` を `- [x]` に更新すること。

## 進捗サマリー

- [x] #1 WSL2 カスタムカーネルビルド（vcan 有効化）
- [x] #2 vcan0 セットアップスクリプト作成（setup_vcan.sh）
- [x] #3 CAN テンプレートファイル作成（normal.can / abnormal.can）
- [x] #4 CAN 送信スクリプト作成（send_normal.sh / send_abnormal.sh）
- [x] #5 ECU アプリ作成（ecu_app/ecu.py）
- [x] #6 PoC 起動スクリプト作成（run_poc.sh）
- [x] #7 README.md 作成

---

## #1 WSL2 カスタムカーネルビルド（vcan 有効化）

**ステータス**: [ ] pending / [ ] in_progress / [x] completed  
**優先度**: 最高（他タスクのブロッカー）

WSL2 のデフォルトカーネルには vcan モジュールが含まれていないため、カスタムカーネルをビルドして vcan を有効化する。

### タスク
- [x] WSL2 カーネルソース取得
- [x] `CONFIG_CAN=y`, `CONFIG_CAN_VCAN=y` を有効化してビルド
- [x] `.wslconfig` でカスタムカーネルを指定
- [x] WSL2 再起動後に `modprobe vcan` が成功することを確認

---

## #2 vcan0 セットアップスクリプト作成（setup_vcan.sh）

**ステータス**: [ ] pending / [ ] in_progress / [x] completed  
**依存**: #1

`scripts/setup_vcan.sh` を作成する。

### タスク
- [x] `modprobe vcan` の実装
- [x] `ip link add vcan0 type vcan` の実装
- [x] `ip link set vcan0 up` の実装
- [x] vcan0 が既に存在する場合はスキップ（冪等性）
- [x] 失敗時は終了コード 1

---

## #3 CAN テンプレートファイル作成（normal.can / abnormal.can）

**ステータス**: [ ] pending / [ ] in_progress / [x] completed

`templates/` ディレクトリに正常・異常テンプレートを作成する。

### タスク
- [x] `normal.can` 作成（ID=0x123, Data=0102030405060708）
- [x] `abnormal.can` 作成（ID=0x123, Data=ffffffffffffffff）

---

## #4 CAN 送信スクリプト作成（send_normal.sh / send_abnormal.sh）

**ステータス**: [ ] pending / [ ] in_progress / [x] completed  
**依存**: #3

`templates/` のファイルを読み込んで `cansend` で送信するスクリプトを作成する。

### タスク
- [x] `send_normal.sh` 作成（100ms 間隔で 10 回送信）
- [x] `send_abnormal.sh` 作成（500ms 間隔で 3 回送信）

---

## #5 ECU アプリ作成（ecu_app/ecu.py）

**ステータス**: [ ] pending / [ ] in_progress / [x] completed

`python-can` を使って vcan0 を受信し、正常/異常を判別してログ出力する Python スクリプトを作成する。

### タスク
- [x] vcan0 受信ループの実装
- [x] 正常/異常判別ロジック（全バイト `0xFF` → ABNORMAL）
- [x] ログ出力フォーマット実装
- [x] vcan0 未存在時のエラー処理
- [x] SIGTERM による正常終了

---

## #6 PoC 起動スクリプト作成（run_poc.sh）

**ステータス**: [ ] pending / [ ] in_progress / [x] completed  
**依存**: #2, #3, #4, #5

全体をワンコマンドで起動する `run_poc.sh` を作成する。

### タスク
- [x] `setup_vcan.sh` 呼び出し
- [x] ECU アプリのバックグラウンド起動（ログ: `logs/ecu.log`）
- [x] `candump` のバックグラウンド起動（ログ: `logs/candump.log`）
- [x] `send_normal.sh` 実行
- [x] `send_abnormal.sh` 実行
- [x] ECU アプリ・candump の終了処理
- [x] ログ内容の表示

---

## #7 README.md 作成

**ステータス**: [ ] pending / [ ] in_progress / [x] completed  
**依存**: #1〜#6

プロジェクトの `README.md` を作成する。

### タスク
- [x] プロジェクト概要の記載
- [x] 前提条件（WSL2 カスタムカーネル or GitHub Actions）の記載
- [x] セットアップ手順の記載
- [x] 実行方法（`./run_poc.sh`）の記載
- [x] ログ確認方法の記載

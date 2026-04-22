# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## プロジェクト概要

仮想 CAN 通信を用いた検証 PoC。Raspberry Pi や実機を使わず、Linux 仮想環境のみで完結させる。
将来的に GitHub Actions (Ubuntu runner) で CI 化することを前提とする。

## 全体アーキテクチャ

```
┌─────────────────────────────────────┐
│           Host PC (Linux)           │
│                                     │
│  [CAN入力生成]  →  vcan0  →  [ECUアプリ]  │
│  (cansend)       (SocketCAN)   (受信・判定) │
│                    ↑                │
│              [CAN観測]              │
│              (candump/Python)       │
└─────────────────────────────────────┘
```

### 構成要素

| モジュール | 役割 | 実装 |
|-----------|------|------|
| vcan セットアップ | 仮想CANバス作成 | シェルスクリプト |
| シナリオ実行 | テスト治具。scenario.yml を読んで cansend で送信 | run_scenario.sh + Python |
| 被検証App | 受信・ログ出力・正常/異常判別 | Python (python-can) |
| CAN観測 | candump または Python でバス監視・判定 | candump / Python |
| 起動スクリプト | 全体をワンコマンドで起動 | `./run_poc.sh [target]` |

## 環境制約

- **現状**: WSL2 環境。vcan モジュール未搭載のためカスタムカーネルビルドが必要
- **将来 CI**: GitHub Actions の `ubuntu-latest` ランナーは vcan 標準搭載

## 成果物一覧

```
virtualCan/
├── CLAUDE.md
├── README.md
├── run_poc.sh              # ワンコマンド起動（引数でターゲット指定）
├── requirements.txt        # Python 依存パッケージ
├── docs/                   # 各モジュール仕様
├── scripts/
│   ├── setup_vcan.sh       # vcan0 セットアップ
│   └── run_scenario.sh     # シナリオYAMLを読んで cansend 実行
└── targets/                # 被検証App + シナリオのルート
    └── ecu_basic/          # ターゲット例
        ├── app.py          # 被検証App本体
        └── scenario.yml    # テストシナリオ定義
```

## ターゲットの追加方法

新しい被検証Appをテストする場合は `targets/<name>/` ディレクトリを作成し、
`app.py`（被検証App）と `scenario.yml`（テストシナリオ）を配置する。

```bash
mkdir targets/my_ecu
cp targets/ecu_basic/app.py targets/my_ecu/app.py      # ベースをコピーして改変
cp targets/ecu_basic/scenario.yml targets/my_ecu/scenario.yml
```

## 主要コマンド

```bash
# vcan0 セットアップ（要 sudo + カスタムカーネル）
./scripts/setup_vcan.sh

# PoC 全体起動（デフォルト: targets/ecu_basic）
sudo bash run_poc.sh

# ターゲットを指定して起動
sudo bash run_poc.sh targets/my_ecu

# 手動での CAN 送受信確認
candump vcan0 &
cansend vcan0 123#0102030405060708
```

## 実装方針

- 実機・CI 性能・セキュリティは考慮しない
- 手動操作に依存しない構成（将来の CI 化のため）
- 被検証App はプロセス実行（Docker 不要）
- シナリオと被検証App は同じ `targets/<name>/` に格納する

## チケット管理ルール

チケットは `docs/tickets.md` で管理する。

**作業時に必ず守ること:**
1. 作業を開始したチケットのステータスを `in_progress` にチェックする
2. サブタスク（`- [ ]`）が完了したら即座に `- [x]` に更新する
3. チケット全体が完了したら冒頭の「進捗サマリー」のチェックボックスも `- [x]` にする
4. チケットのステータス行は該当するものだけ `[x]` にし、他は `[ ]` のままにする

例（作業中）:
```
**ステータス**: [ ] pending / [x] in_progress / [ ] completed
```

例（完了後）:
```
**ステータス**: [ ] pending / [ ] in_progress / [x] completed
```

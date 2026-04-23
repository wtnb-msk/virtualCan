# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## プロジェクト概要

仮想 CAN 通信を用いた検証 PoC。Raspberry Pi や実機を使わず、Linux 仮想環境のみで完結させる。
GitHub Actions (ubuntu-latest) による CI 対応済み。

## 全体アーキテクチャ

```
┌─────────────────────────────────────┐
│           Host PC (Linux)           │
│                                     │
│  [CAN入力生成]  →  vcan0  →  [ECUアプリ]  │
│  (cansend)       (SocketCAN)   (受信・判定) │
│                    ↑                │
│              [CAN観測]              │
│              (candump)              │
└─────────────────────────────────────┘
```

### 構成要素

| モジュール | 役割 | 実装 |
|-----------|------|------|
| vcan セットアップ | 仮想CANバス作成 | `scripts/setup_vcan.sh` |
| シナリオ実行 | テスト治具。`scenario.yml` を読んで cansend で送信 | `scripts/run_scenario.sh` |
| 被検証App | 受信・ログ出力・正常/異常判別 | Python (`app.py`) または C++ (`main.cpp` → `app`) |
| CAN観測 | candump でバス監視 | candump |
| 起動スクリプト | 全体をワンコマンドで起動 | `run_poc.sh` |
| CI | 全ターゲットをビルド・テスト | `.github/workflows/ci.yml` |

## 環境制約

- **現状**: WSL2 環境。vcan モジュール有効化のためカスタムカーネルビルド済み
- **CI**: GitHub Actions の `ubuntu-latest` ランナーは `linux-modules-extra` をインストールで対応

## ターゲット構成

```
targets/<name>/
├── main.cpp     # C++ ECU（ビルドして app バイナリを生成）
└── scenario.yml # テストシナリオ定義（assertions セクション含む）
```

## 成果物一覧

```
virtualCan/
├── CLAUDE.md
├── README.md
├── run_poc.sh
├── requirements.txt
├── .github/
│   └── workflows/ci.yml
├── docs/
│   ├── 01_vcan_setup.md
│   ├── 02_can_input.md
│   ├── 03_ecu_app.md
│   ├── 04_can_observe.md
│   ├── 05_run_poc.md
│   ├── 06_dependencies.md
│   ├── 07_ci.md
│   └── tickets.md
├── scripts/
│   ├── setup_vcan.sh
│   ├── run_scenario.sh
│   └── check_result.sh
└── targets/
    └── ecu_cpp/         # C++ ECU（参照実装）
        ├── main.cpp
        └── scenario.yml
```

## 主要コマンド

```bash
# vcan0 セットアップ（要 sudo）
sudo bash scripts/setup_vcan.sh

# C++ ECU をビルドして起動
g++ -O2 -o targets/ecu_cpp/app targets/ecu_cpp/main.cpp
sudo bash run_poc.sh targets/ecu_cpp

# CI 判定スクリプト単体実行
bash scripts/check_result.sh logs/ecu.log targets/ecu_cpp/scenario.yml
```

## 実装方針

- 実機・CI 性能・セキュリティは考慮しない
- 手動操作に依存しない構成（CI 化のため）
- 被検証App はプロセス実行（Docker 不要）
- ECU アプリは C++ で実装する

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

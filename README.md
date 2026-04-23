# virtualCan PoC

仮想 CAN 通信を用いた検証 PoC。Raspberry Pi や実機を使わず、Linux 仮想環境のみで完結する。
GitHub Actions (ubuntu-latest) による CI も対応済み。

## 前提条件

### WSL2 環境

WSL2 のデフォルトカーネルには `vcan` モジュールが含まれていないため、カスタムカーネルのビルドが必要。
詳細は [`docs/01_vcan_setup.md`](docs/01_vcan_setup.md) を参照。

### GitHub Actions 環境

`ubuntu-latest` ランナーは `linux-modules-extra` をインストールするだけで vcan が利用できる。カーネルビルド不要。

## セットアップ

```bash
# 1. システムパッケージのインストール
sudo apt install -y can-utils python3-venv python3-dev build-essential

# 2. Python 仮想環境の作成（run_scenario.sh が pyyaml を使用）
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

# 3. C++ ターゲットのビルド
g++ -O2 -o targets/ecu_cpp/app targets/ecu_cpp/main.cpp
```

## 実行方法

```bash
sudo bash run_poc.sh targets/ecu_cpp
```

実行シーケンス:

1. `vcan0` のセットアップ
2. ECUアプリ（C++ バイナリ）をバックグラウンド起動
3. `candump` をバックグラウンド起動
4. `scenario.yml` の全ステップを順番に cansend で送信
5. 全プロセスを終了してログを表示

## ログ確認

```
=== candump log ===
vcan0  123   [8]  01 02 03 04 05 06 07 08
vcan0  123   [8]  FF FF FF FF FF FF FF FF

=== ECU log ===
[NORMAL]   ID=0x123 Data=0102030405060708
[ABNORMAL] ID=0x123 Data=ffffffffffffffff
```

| ファイル | 内容 |
|---------|------|
| `logs/candump.log` | candump の生ログ |
| `logs/ecu.log` | ECU アプリの判定ログ |

## ターゲットの追加方法

```bash
mkdir targets/my_ecu
```

`main.cpp` と `scenario.yml` を配置し、`g++` でビルドして `app` バイナリを生成する。

`scenario.yml` の形式:

```yaml
name: my_ecu_test

assertions:
  - pattern: "期待するログ文字列"
    description: "テスト内容の説明"

steps:
  - label: "正常フレーム送信"
    template: "123#0102030405060708"
    count: 10
    interval: 0.1
  - label: "異常フレーム送信"
    template: "123#FFFFFFFFFFFFFFFF"
    count: 3
    interval: 0.5
```

## ディレクトリ構成

```
virtualCan/
├── run_poc.sh                  # ワンコマンド起動（app バイナリを実行）
├── requirements.txt            # Python 依存パッケージ（pyyaml 等）
├── .github/
│   └── workflows/
│       └── ci.yml              # GitHub Actions ワークフロー
├── scripts/
│   ├── setup_vcan.sh           # vcan0 セットアップ
│   ├── run_scenario.sh         # scenario.yml を読んで cansend 実行
│   └── check_result.sh         # CI 用ログアサート（assertions を参照）
├── targets/                    # 被検証App + シナリオのルート
│   └── ecu_cpp/                # C++ ECU（参照実装）
│       ├── main.cpp
│       └── scenario.yml
├── docs/                       # 各モジュール仕様書
└── logs/                       # 実行ログ出力先（.gitignore 対象）
```

## ドキュメント

| ファイル | 内容 |
|---------|------|
| [`docs/01_vcan_setup.md`](docs/01_vcan_setup.md) | WSL2 カスタムカーネルビルド手順 |
| [`docs/02_can_input.md`](docs/02_can_input.md) | シナリオ定義・cansend 仕様 |
| [`docs/03_ecu_app.md`](docs/03_ecu_app.md) | ECU アプリ仕様（C++） |
| [`docs/04_can_observe.md`](docs/04_can_observe.md) | CAN 観測・ログ仕様 |
| [`docs/05_run_poc.md`](docs/05_run_poc.md) | run_poc.sh 仕様 |
| [`docs/06_dependencies.md`](docs/06_dependencies.md) | 依存パッケージ一覧 |
| [`docs/07_ci.md`](docs/07_ci.md) | GitHub Actions CI 手順書 |

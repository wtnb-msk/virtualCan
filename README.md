# virtualCan PoC

仮想 CAN 通信を用いた検証 PoC。Raspberry Pi や実機を使わず、Linux 仮想環境のみで完結する。

## 前提条件

### WSL2 環境（現状）

WSL2 のデフォルトカーネルには `vcan` モジュールが含まれていないため、カスタムカーネルのビルドが必要。
手順の詳細は [`docs/01_vcan_setup.md`](docs/01_vcan_setup.md) を参照。

### GitHub Actions 環境（将来 CI）

`ubuntu-latest` ランナーは `vcan` が標準搭載されているため、カーネルビルド不要。

## セットアップ

```bash
# 1. システムパッケージのインストール
sudo apt install -y can-utils python3-venv python3-dev

# 2. Python 仮想環境の作成
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 実行方法

```bash
# デフォルトターゲット（targets/ecu_basic）で実行
sudo bash run_poc.sh

# ターゲットを指定して実行
sudo bash run_poc.sh targets/ecu_basic
```

スクリプトが以下を自動で実行する：

1. `vcan0` のセットアップ
2. 被検証App（`targets/<name>/app.py`）をバックグラウンド起動
3. `candump` をバックグラウンド起動
4. シナリオ（`targets/<name>/scenario.yml`）の全ステップを順番に送信
5. 全プロセスを終了してログを表示

## ログ確認

実行後に以下のログが標準出力に表示される。

```
=== candump log ===
vcan0  123   [8]  01 02 03 04 05 06 07 08
vcan0  123   [8]  FF FF FF FF FF FF FF FF

=== ECU log ===
[NORMAL]   ID=0x123 Data=0102030405060708
[ABNORMAL] ID=0x123 Data=ffffffffffffffff
```

ログファイルは `logs/` ディレクトリに保存される。

| ファイル | 内容 |
|---------|------|
| `logs/candump.log` | candump の生ログ |
| `logs/ecu.log` | ECU アプリの判定ログ |

## ターゲットの追加方法

新しい被検証App をテストする場合は `targets/<name>/` を作成し、2ファイルを配置する。

```bash
mkdir targets/my_ecu
# app.py  : 被検証App本体（vcan0 を受信して判定するPythonスクリプト）
# scenario.yml : テストシナリオ定義（正常系・異常系のステップを記述）
```

`scenario.yml` の形式：

```yaml
name: my_ecu_test
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
├── run_poc.sh              # ワンコマンド起動（引数でターゲット指定）
├── requirements.txt        # Python 依存パッケージ
├── scripts/
│   ├── setup_vcan.sh       # vcan0 セットアップ
│   └── run_scenario.sh     # シナリオYAMLを読んで cansend 実行
├── targets/                # 被検証App + シナリオのルート
│   └── ecu_basic/          # ターゲット例
│       ├── app.py          # 被検証App本体
│       └── scenario.yml    # テストシナリオ定義
├── logs/                   # 実行ログ出力先
└── docs/                   # 各モジュール仕様書
```

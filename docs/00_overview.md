# virtualCan 環境 概要説明

このドキュメントは、virtualCan の仕組み全体を初めて読む人向けに説明したものです。

---

## そもそも何をしたいのか

実際の車のECU（電子制御ユニット）に乗るアプリケーション（C++で書かれたプログラム）を、  
**実車もハードウェアも使わずに、PCの中だけでテストする**仕組みを作りたい。

---

## 実車でのCAN通信とは

実際の車の中では、複数の ECU がお互いに **CAN（Controller Area Network）** というバスを通じてメッセージをやりとりしている。

```
[エンジンECU] ──┐
[ブレーキECU] ──┤── CANバス ──── [各ECUがメッセージを受信・判定]
[センサーECU] ──┘
```

CANメッセージの構造はシンプルで、**ID（誰宛か）** と **Data（内容）** の2つからなる。

```
例:  ID=0x123  Data=01 02 03 04 05 06 07 08
```

---

## この環境でやっていること

実車の CANバスの代わりに、Linux が提供する **vcan（仮想CANバス）** を使う。

```
【実車の世界】                    【この環境】

  CANバス（物理ケーブル）   →     vcan0（Linuxの仮想バス）
  他のECUが送るメッセージ   →     テストシナリオ（scenario.yml）が cansend で送信
  被検証ECUアプリ           →     targets/ecu_cpp/app（C++バイナリ）
```

**Linuxの中だけで完結するので、実機なしでテストが回る。**

---

## 全体の構成図

```
┌─────────────────────────────────────────────────────┐
│                   Linux 環境                         │
│                                                     │
│  ┌─────────────┐   CANフレーム   ┌───────────────┐  │
│  │ run_scenario│ ─────────────→ │    vcan0      │  │
│  │    .sh      │  (cansend)     │ (仮想CANバス) │  │
│  │             │                └───────┬───────┘  │
│  │scenario.yml │                        │受信        │
│  │を読んで送信  │                        ↓           │
│  └─────────────┘               ┌───────────────┐  │
│                                 │  ECUアプリ    │  │
│  ┌─────────────┐   同じバスを   │  (main.cpp)   │  │
│  │  candump    │ ←──観測────── │               │  │
│  │（バス監視） │                │ 正常/異常判定 │  │
│  └─────────────┘               └───────┬───────┘  │
│         ↓ログ保存                        ↓ログ保存   │
│  logs/candump.log               logs/ecu.log        │
└─────────────────────────────────────────────────────┘
```

| 登場人物 | 役割 | 実体 |
|---------|------|------|
| `vcan0` | 仮想 CAN バス。実車のケーブルに相当 | Linux カーネルの仮想デバイス |
| `run_scenario.sh` | テスト治具。シナリオに従って CAN メッセージを送る | シェルスクリプト + `cansend` コマンド |
| `scenario.yml` | 何をどの順番で送るか・何をテスト合格条件にするかを定義 | YAML ファイル |
| ECUアプリ（`main.cpp`）| 被検証対象。vcan0 を受信して正常/異常を判定しログを出す | C++ プログラム |
| `candump` | vcan0 上の全フレームをそのまま記録する観測ツール | `can-utils` のコマンド |
| `check_result.sh` | テスト合否を自動判定する | シェルスクリプト + Python |

---

## シナリオファイル（scenario.yml）の役割

```yaml
name: ecu_cpp_test

# ─── テスト合否条件 ──────────────────────────────
assertions:
  - pattern: "[NORMAL]"
    description: "正常フレームが NORMAL と判定されること"
  - pattern: "[ABNORMAL]"
    description: "異常フレームが ABNORMAL と判定されること"

# ─── 送信するメッセージの手順 ────────────────────
steps:
  - label: "正常フレーム送信"
    template: "123#0102030405060708"   # ID=0x123, Data=01 02 03 04 05 06 07 08
    count: 5
    interval: 0.1                      # 0.1秒間隔で5回送信
```

`scenario.yml` は **「何を送るか（steps）」** と **「何が出力されればテスト成功か（assertions）」** の2つを1ファイルに持つ。

### assertions の仕組み

```
ECUアプリが logs/ecu.log に出力する文字列
         ↓
assertions の pattern と照合
         ↓
全部一致 → PASS（CI 成功）
1つでも欠ける → FAIL（CI 失敗）
```

**実車 App に差し替えるときは pattern を実車 App が実際に出力する文字列に書き換えるだけでよい。**

---

## CI（GitHub Actions）の役割

ローカルで手動実行するのではなく、**コードを push するたびに GitHub のサーバー上で自動的に同じテストを実行する**仕組み。

```
開発者が git push
      ↓
GitHub Actions が自動起動
      ↓
Ubuntu の仮想マシンが起動（毎回まっさら）
      ↓
① 依存パッケージのインストール（can-utils, linux-modules-extra 等）
② C++ ビルド（g++ で main.cpp → app バイナリ）
③ vcan0 セットアップ
④ ECUアプリ起動 + シナリオ送信
⑤ assertions でログを照合 → PASS/FAIL
⑥ ログを Artifact として保存
      ↓
Actions タブで結果を確認（✅ or ❌）
```

**「push したら自動でテストが走り、結果が GitHub に表示される」** という状態になっている。

---

## 実行の流れ（ローカル手動実行の場合）

```bash
# 1. C++ ビルド
g++ -O2 -o targets/ecu_cpp/app targets/ecu_cpp/main.cpp

# 2. ワンコマンド実行
sudo bash run_poc.sh targets/ecu_cpp
```

`run_poc.sh` の内部で以下が順番に実行される：

```
1. vcan0 をセットアップ（modprobe vcan → ip link add vcan0）
2. ECUアプリ（app）をバックグラウンドで起動
3. candump をバックグラウンドで起動
4. scenario.yml の steps を順番に cansend で送信
5. プロセスを終了してログを表示
```

実行後に `logs/ecu.log` を確認すれば ECU がどう判定したかがわかる：

```
[NORMAL]   ID=0x123 Data=0102030405060708
[NORMAL]   ID=0x123 Data=0102030405060708
[ABNORMAL] ID=0x123 Data=ffffffffffffffff
[NORMAL]   ID=0x123 Data=ff00000000000000
```

---

## 実車 App に差し替えるときの変更箇所

実際の車載 App（C++）を投入する場合、変更は最小限で済む設計になっている。

| 変更箇所 | 何を変えるか |
|---------|------------|
| `targets/ecu_cpp/main.cpp` | 実車 App のコードに置き換える |
| `scenario.yml` の `steps` | 実車 App が期待する CAN メッセージに合わせる |
| `scenario.yml` の `assertions` の `pattern` | 実車 App が実際に出力するログ文字列に合わせる |

`check_result.sh` / `run_poc.sh` / `ci.yml` は**変更不要**。

---

## ディレクトリ構成（早見表）

```
virtualCan/
│
├── run_poc.sh              ← ワンコマンド起動スクリプト
├── requirements.txt        ← Python 依存（pyyaml 等）
│
├── .github/workflows/
│   └── ci.yml              ← GitHub Actions の定義
│
├── scripts/
│   ├── setup_vcan.sh       ← vcan0 を作る
│   ├── run_scenario.sh     ← scenario.yml を読んで cansend を実行
│   └── check_result.sh     ← assertions と照合して合否判定
│
└── targets/ecu_cpp/
    ├── main.cpp            ← 被検証ECUアプリ（ここを実車Appに差し替える）
    └── scenario.yml        ← テスト手順 + 合否条件（ここの pattern を合わせる）
```

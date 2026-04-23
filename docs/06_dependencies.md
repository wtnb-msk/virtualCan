# 依存パッケージ仕様

## WSL2 環境

```bash
sudo apt update
sudo apt install -y \
  can-utils \
  python3-venv \
  python3-dev \
  build-essential
```

| パッケージ | 用途 |
|-----------|------|
| `can-utils` | cansend / candump |
| `python3-venv` | Python 仮想環境 |
| `python3-dev` | pip wheel ビルド用ヘッダ |
| `build-essential` | g++（C++ ターゲットのビルド） |

---

## GitHub Actions 環境

```yaml
sudo apt-get install -y \
  can-utils python3-venv python3-dev \
  linux-modules-extra-$(uname -r)
```

`linux-modules-extra-$(uname -r)` は ubuntu-latest ランナーで vcan モジュールを有効にするために必要。

---

## Python 環境（venv）

Ubuntu 24.04 では `externally-managed-environment` 制約があるため venv 必須。

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## requirements.txt

```
python-can==4.6.1
pyyaml==6.0.3
```

| パッケージ | 用途 |
|-----------|------|
| `python-can` | Python ECU アプリの SocketCAN バインディング |
| `pyyaml` | `run_scenario.sh` 内での scenario.yml パース |

---

## C++ ターゲットのビルド

追加ライブラリ不要。Linux カーネルヘッダ（`linux/can.h`）のみ使用する。

```bash
g++ -O2 -o targets/ecu_cpp/app targets/ecu_cpp/main.cpp
```

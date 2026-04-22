# 依存パッケージ仕様

## システムパッケージ

```bash
sudo apt update
sudo apt install -y \
  can-utils \
  python3-venv \
  python3-dev \
  python3-pip \
  build-essential
```

| パッケージ | 用途 | バージョン（Ubuntu 24.04） |
|-----------|------|--------------------------|
| `can-utils` | cansend / candump | 2023.03-1 |
| `python3-venv` | Python 仮想環境 | - |
| `python3-dev` | pip wheel ビルド用ヘッダ | - |

---

## Python 環境（venv）

Ubuntu 24.04 では `externally-managed-environment` 制約があるため、**venv 必須**。

```bash
cd /home/scsk-watanabe/practice/virtualCan

python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

---

## requirements.txt

```
python-can==4.6.1
```

> `python-can` の socketcan バックエンドは追加パッケージ不要。
> vcan0（SocketCAN）へのアクセスに `interface="socketcan"` を使用する。

---

## python-can の最小使用例

```python
import can

with can.Bus(channel="vcan0", interface="socketcan") as bus:
    for msg in bus:
        print(f"ID=0x{msg.arbitration_id:03X} Data={msg.data.hex()}")
```

---

## バージョン確認コマンド

```bash
# can-utils
dpkg -l can-utils

# python-can
source .venv/bin/activate
python3 -c "import can; print(can.__version__)"
```

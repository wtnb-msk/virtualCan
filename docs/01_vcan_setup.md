# vcan セットアップ仕様

## 目的

SocketCAN の仮想バス `vcan0` をホスト PC 上に作成する。

---

## WSL2 カスタムカーネルビルド手順

WSL2 のデフォルトカーネルは `CONFIG_CAN_VCAN` が無効なため自前ビルドが必要。
CAN 基盤（`CONFIG_CAN=m` 等）は既に有効なので、**追加は `CONFIG_CAN_VCAN=m` の1行のみ**。

### ビルド依存パッケージのインストール

```bash
sudo apt update
sudo apt install -y \
  build-essential flex bison \
  libssl-dev libelf-dev libncurses-dev \
  git bc dwarves cpio python3-dev
```

### カーネルソース取得

```bash
mkdir -p ~/wsl-kernel && cd ~/wsl-kernel
git clone --depth=1 \
  --branch linux-msft-wsl-6.6.87.2 \
  https://github.com/microsoft/WSL2-Linux-Kernel.git \
  WSL2-Linux-Kernel-6.6.87.2
cd WSL2-Linux-Kernel-6.6.87.2
```

### カーネル設定変更

```bash
cp Microsoft/config-wsl .config
scripts/config --module CONFIG_CAN_VCAN
make olddefconfig

# 確認（期待値: CONFIG_CAN_VCAN=m）
grep "CONFIG_CAN_VCAN" .config
```

### ビルド

```bash
# LOCALVERSION="" を指定しないと modprobe が失敗するので必須
make -j$(nproc) LOCALVERSION=""
```

> 目安: 12コア環境で 10〜30 分。ディスク消費約 8〜10GB。

### カーネルモジュールのインストール（**必須**）

```bash
sudo make modules_install INSTALL_MOD_PATH=/
```

> **これを省略すると `modprobe vcan` が失敗する。** 必ず実行すること。

### Windows 側への配置と .wslconfig 設定

```bash
# カーネルを Windows ユーザーホームへコピー
cp arch/x86/boot/bzImage /mnt/c/Users/scsk-watanabe/wsl_kernel

# .wslconfig を作成（パス区切りは \\ 必須、スペース不可）
cat > /mnt/c/Users/scsk-watanabe/.wslconfig << 'EOF'
[wsl2]
kernel=C:\\Users\\scsk-watanabe\\wsl_kernel

[boot]
systemd=true
EOF
```

### WSL2 再起動・動作確認

PowerShell で実行:
```powershell
wsl --shutdown
```

再起動後:
```bash
uname -r          # カーネルバージョンが変わっていることを確認
sudo modprobe vcan  # エラーが出なければ成功
```

---

## setup_vcan.sh の仕様

| 項目 | 内容 |
|------|------|
| 実行権限 | sudo が必要 |
| 操作 | `modprobe vcan` → `ip link add vcan0 type vcan` → `ip link set vcan0 up` |
| 冪等性 | vcan0 が既に存在する場合はスキップ |
| 終了コード | 成功=0、失敗=1 |

---

## 注意点

| ポイント | 説明 |
|---------|------|
| `make olddefconfig` を必ず実行 | 依存オプション未設定によるビルドエラーを防ぐ |
| `LOCALVERSION=""` を指定 | 省略すると `-dirty` 付きになり `modprobe` が失敗する |
| `.wslconfig` のパス区切り | `\\` （二重バックスラッシュ）を使用。スペース含むパスは不可 |
| `wsl --shutdown` の実行 | ターミナルを閉じるだけではカーネルが切り替わらない |
| vcan0 は再起動のたびに消える | `setup_vcan.sh` を毎回実行する必要がある |

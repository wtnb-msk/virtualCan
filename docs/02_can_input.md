# CAN入力生成 仕様

## 目的

テスト治具として vcan0 に CAN フレームを送信する。

## シナリオ定義ファイル（scenario.yml）

各ターゲットの `targets/<name>/scenario.yml` にテスト手順を定義する。

```yaml
name: ecu_basic_test
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

### フィールド仕様

| フィールド | 型 | 説明 |
|-----------|-----|------|
| `name` | string | シナリオ名（ログ識別用） |
| `steps[].label` | string | ステップ名（ログ出力用） |
| `steps[].template` | string | `cansend` 形式（`<ID>#<DATA_HEX>`） |
| `steps[].count` | int | 送信回数 |
| `steps[].interval` | float | 送信間隔（秒） |

### template の書式

```
<CAN_ID>#<DATA_HEX>

例:
  123#0102030405060708    # 標準フレーム、8バイト
  123#FF                  # 標準フレーム、1バイト
  123#                    # データなし（DLC=0）
  1FFFFFFF#DEADBEEF       # 拡張フレームID（29bit）
```

## 実行スクリプト（scripts/run_scenario.sh）

`scenario.yml` を読み込み、各ステップを順番に `cansend` で実行する。

```bash
bash scripts/run_scenario.sh targets/ecu_basic/scenario.yml
```

内部では Python（pyyaml）で YAML をパースし、`subprocess.run(["cansend", ...])` を呼び出す。

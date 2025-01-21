# M5player

## ビルド環境

動作検証環境：Arduino IDE 2.3.4 (Windows/Linux)

### セットアップ

1. File > Preferences > Additional Boards Manager URL に以下を追加する．
    - `https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json`
2. Boards Manager にて以下をインストールする．
    -  "M5Stack" (v2.1.3)
3. Library Manager にて以下をインストールする．
    - "M5AtomS3" (v1.0.0) とその依存先
    - "MIDI Library" (v5.0.2)
4. Library Manager にて，互換性のため "FastLED" を v3.9.9 以前にダウングレードする．

### ビルド

1. Tools > Board > M5AtomS3 を選択する．
2. Tools > Port でデバイスに対応するポートを選択する．
3. コンパイルする．

## Acknowledgements

以下のリポジトリを参考にした．

- https://github.com/necobit/GROVE-MIDI-UNIT

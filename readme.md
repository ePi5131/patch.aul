# patch.aul
## これはなに
AviUtlや拡張編集のバグを直したり機能を追加するプラグイン
黒魔術を一手に引き受けるのが目的

詳細はpatch.aul.txtを読むこと

## ビルドについて

- [CUDA TOOLKIT](https://developer.nvidia.com/cuda-toolkit)を用意してください
 これのOpenCLライブラリを使います 環境変数`CUDA_PATH`を使います

-  `test`フォルダを用意してください
 これは`aviutl.exe`などを置くフォルダで、デバッグ実行するとこのディレクトリの`aviutl.exe`が実行されます

- ビルドすると、`pack`フォルダにリリース用のファイルが集まります

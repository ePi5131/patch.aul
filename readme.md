# patch.aul
## これはなに
AviUtlや拡張編集のバグを直したり機能を追加するプラグイン
黒魔術を一手に引き受けるのが目的

詳細はpatch.aul.txtを読むこと

## ビルドについて

- 以下の環境変数を設定してください


| 名前                  | 詳細                                         |
| --------------------- | -------------------------------------------- |
| BOOST_1_78_0_DIR      | https://www.boost.org                        |
| AVIUTL_EXEDIT_SDK_DIR | https://github.com/ePi5131/aviutl_exedit_sdk |
| WINWRAP_DIR           | https://github.com/ePi5131/winwrap           |

- `pack`フォルダを作成してください
 これはリリース用のファイルが集まる場所になります

-  `test`フォルダを作成してください
 これは`aviutl.exe`などを置くフォルダで、デバッグ実行するとこのディレクトリの`aviutl.exe`が実行されます


後はソリューションファイルを開いて適当にやるだけ

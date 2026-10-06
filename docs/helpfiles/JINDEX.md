# ヘルプ索引

このコマンドは横64文字以上表示できる画面モードで使用して下さい.

以下の項目に対する説明が用意されています.
特定のコマンドに関する詳しい説明を表示させるには

```
HELP  command name
```

と入力して下さい.

| コマンド | 説明 |
|---|---|
| [`ALIAS`](ALIAS.md) | エイリアスの表示/設定 |
| [`ASSIGN`](ASSIGN.md) | ドライブの割当 |
| [`ATDIR`](ATDIR.md) | ディレクトリの属性の変更 |
| [`ATTRIB`](ATTRIB.md) | ファイルの属性の変更 |
| [`BASIC`](BASIC.md) | MSX disk BASICの起動 |
| [`BEEP`](BEEP.md) | ビープ音の発生 |
| [`BOOT`](BOOT.md) | Nextor起動ドライブの設定 |
| [`BUFFERS`](BUFFERS.md) | ディスクバッファ数の表示/設定 |
| [`CD`](CD.md) | [`CHDIR`](CHDIR.md)の省略形 |
| [`CDD`](CDD.md) | ディフォルトディレクトリとドライブの表示/設定 |
| [`CDPATH`](CDPATH.md) | ディレクトリサーチパスの表示/設定 |
| [`CHDIR`](CHDIR.md) | ディフォルトディレクトリの表示/設定 |
| [`CHKDSK`](CHKDSK.md) | ディスク内容のチェック |
| [`CLS`](CLS.md) | 画面表示の消去 |
| [`COLOR`](COLOR.md) | 画面の色の変更 |
| [`CONCAT`](CONCAT.md) | ファイルの結合 |
| [`COPY`](COPY.md) | ファイルのコピー |
| [`CPU`](CPU.md) | CPUモードの表示/設定 |
| [`DATE`](DATE.md) | 日付の表示/設定 |
| [`DEL`](DEL.md) | [`ERASE`](ERASE.md)と同じ |
| [`DEVINFO`](DEVINFO.md) | ディスクドライバが扱うデバイスの表示 |
| [`DIR`](DIR.md) | ディスク上のファイル名の表示 |
| [`DIRB`](DIRB.md) | `DIR`と同じ、ただしサイズは常にバイト単位で表示 |
| [`DISKCOPY`](DISKCOPY.md) | ディスクの全データのコピー |
| [`DRIVERS`](DRIVERS.md) | システム内のディスクドライバの表示 |
| [`DRVINFO`](DRVINFO.md) | 各ドライブの割当先の表示 |
| [`DSKCHK`](DSKCHK.md) | ディスクチェック状態の表示/設定 |
| [`ECHO`](ECHO.md) | 文字列の表示 |
| [`ECHOS`](ECHOS.md) | 文字列の表示 (改行なし) |
| [`ELSE`](ELSE.md) | コマンドモードの反転 |
| [`END`](END.md) | バッチファイル実行の終了 [.BTM] |
| [`ENDIFF`](ENDIFF.md) | コマンドモードの復元 |
| [`ERA`](ERA.md) | [`ERASE`](ERASE.md)の省略形 |
| [`ERASE`](ERASE.md) | ファイルの消去 |
| [`EXIT`](EXIT.md) | `COMMAND3.COM`の終了 |
| [`FIXDISK`](FIXDISK.md) | MSX-DOS1のディスクをMSX-DOS2用に変更 |
| [`FORMAT`](FORMAT.md) | ディスクの初期化 |
| [`FREE`](FREE.md) | ディスク容量 (全体/使用中/空き) の表示 |
| [`GOSUB`](GOSUB.md) | バッチファイルのサブルーチンの呼び出し [.BTM] |
| [`GOTO`](GOTO.md) | バッチファイル内のラベルへのジャンプ [.BTM] |
| [`HELP`](HELP.md) | コマンドの説明 |
| [`HERTZ`](HERTZ.md) | VDPの画面リフレッシュ速度の設定 |
| [`HISTORY`](HISTORY.md) | コマンドヒストリの表示/バッファのサイズ変更 |
| [`IF`](IF.md) | 条件が真の場合にコマンドを実行 |
| [`IFF`](IFF.md) | 条件が真の場合にコマンドモードをONに設定 |
| [`INKEY`](INKEY.md) | 1文字を環境変数に読み込み |
| [`INPUT`](INPUT.md) | 文字列を環境変数に読み込み |
| [`KMODE`](KMODE.md) | 漢字モードの変更 |
| [`LOCK`](LOCK.md) | ドライブのロック/ロック解除 |
| [`MAPDRV`](MAPDRV.md) | ドライブをデバイスに割当,またはファイルをマウント |
| [`MD`](MD.md) | [`MKDIR`](MKDIR.md)の省略形 |
| [`MEM`](MEM.md) | メモリマッパの簡易表示 |
| [`MEMORY`](MEMORY.md) | システムRAMの容量と状態の表示 |
| [`MKDIR`](MKDIR.md) | サブディレクトリの作成 |
| [`MODE`](MODE.md) | スクリーンモードの変更 |
| [`MOVE`](MOVE.md) | ファイルの移動 |
| [`MVDIR`](MVDIR.md) | サブディレクトリの移動 |
| [`PATH`](PATH.md) | コマンドサーチパスの表示/設定 |
| [`PAUSE`](PAUSE.md) | 一時停止 |
| [`POPD`](POPD.md) | 保存したドライブとディレクトリの復元 |
| [`PUSHD`](PUSHD.md) | 現在のドライブとディレクトリの保存 |
| [`RALLOC`](RALLOC.md) | 縮小アロケーション情報モードの表示/設定 |
| [`RAMDISK`](RAMDISK.md) | RAMディスクサイズの表示/設定 |
| [`RD`](RD.md) | [`RMDIR`](RMDIR.md)の省略形 |
| [`REM`](REM.md) | 注釈 |
| [`REN`](REN.md) | [`RENAME`](RENAME.md)の省略形 |
| [`RENAME`](RENAME.md) | ファイル名の変更 |
| [`RESET`](RESET.md) | システムのリセット |
| [`RETURN`](RETURN.md) | バッチファイルのサブルーチンからの復帰 [.BTM] |
| [`RMDIR`](RMDIR.md) | サブディレクトリの消去 |
| [`RNDIR`](RNDIR.md) | サブディレクトリ名の変更 |
| [`SET`](SET.md) | 環境変数の表示/設定 |
| [`SHELLRAM`](SHELLRAM.md) | シェル機能用RAMセグメント使用の表示/設定 |
| [`SHIFT`](SHIFT.md) | バッチファイル引数の左シフト |
| [`THEN`](THEN.md) | 直後のコマンドの実行 |
| [`TIME`](TIME.md) | 時刻の表示/設定 |
| [`TO`](TO.md) | 別のディレクトリへの移動 |
| [`TREE`](TREE.md) | ディレクトリのツリー形式表示 |
| [`TYPE`](TYPE.md) | ファイル内容の表示 |
| [`UNDEL`](UNDEL.md) | 消去したファイルの復活 |
| [`VER`](VER.md) | Nextorのバージョン番号の表示 |
| [`VERIFY`](VERIFY.md) | ベリファイフラグの表示/設定 |
| [`VOL`](VOL.md) | ボリューム名の表示/設定 |
| [`XCOPY`](XCOPY.md) | `COPY`の拡張版 |
| [`XDIR`](XDIR.md) | `DIR`の拡張版 |
| [`YENSLASH`](YENSLASH.md) | 円記号のバックスラッシュへの置き換えの表示/設定 |
| [`Z80MODE`](Z80MODE.md) | レガシードライバのZ80アクセスモードの表示/設定 |

Nextorの全般的機能に関する説明は以下の通りです.

| 項目 | 説明 |
|---|---|
| [`ALI`](ALI.md) | エイリアスと実行可能拡張子 |
| [`BATCH`](BATCH.md) | バッチ処理 |
| [`EDITING`](EDITING.md) | コマンド行の編集 |
| [`ENV`](ENV.md) | 環境変数 |
| [`ERRORS`](ERRORS.md) | エラーメッセージ |
| [`IO`](IO.md) | リダイレクト,パイプ,標準入出力 |
| [`SYNTAX`](SYNTAX.md) | コマンドの説明に使われる記法 |

---

[English version](INDEX.md) · [About these files](README.md)


    This is a Virtual Floppy Drive (VFD) for Windows NT platform.
    Copyright (C) 2003-2008 Ken Kato (chitchat.vdk@gmail.com)
    http://chitchat.at.infoseek.co.jp/vmware/vfdj.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    The GNU General Public License is also available from:
    http://www.gnu.org/copyleft/gpl.html


プログラムファイル

    vfd.sys
	VFD カーネルモードドライバ.

    vfd.exe
	VFD コントロールプログラム (コンソール版).

    vfd.dll
        VFD デバイス制御およびシェルエクステンションライブラリ

    vfdwin.exe
	VFD コントロールプログラム (GUI 版).


インストール

    以前のバージョンが既にインストールされている場合、次の「アップデート」
    セクションを参照してください。

    上記のプログラムファイルをローカルディレクトリにコピーします。

    VFD.EXE または VFDWIN.EXE を使用してインストールを行います:

    - ドライバをインストールする
        (VFD.EXE)     VFD.EXE INSTALL
        (VFDWIN.EXE)  [ドライバ] -> [インストール]

    - システム起動時にドライバが自動で起動するようにしたい場合
        (VFD.EXE)     VFD.EXE INSTALL /AUTO
        (VFDWIN.EXE)  [ドライバ] -> 開始種別 [自動] を選択 -> [インストール]

    - シェルエクステンションをインストールしたい場合
        (VFD.EXE)     VFD.EXE SHELL /ON
        (VFDWIN.EXE)  [シェル] -> [シェルエクステンション] にチェックをつける
                      -> [適用]

    - ファイルの関連付けをしたい場合
        (VFDWIN.EXE)  [関連付け] -> 関連付けを設定 -> [適用]

    !!! 注意 !!!
    デバイスドライバはネットワークドライブからは実行できません。
    ドライバファイル (vfd.sys) は必ずローカルドライブに置いてください。


アップデート

    旧バージョンのドライバが動作中でないこと、シェルエクステンションが無効に
    なっていることを確認してください。あとはプログラムファイルを全て新しいも
    のと置き換えるだけです。

    !!! 注意 !!!
    Windows 2000/XP で バージョン 2.0 RC が使用していたドライバ自動起動設定は
    現バージョンのドライバでは不具合があります。もしも 2.0 RC を自動起動で使用
    していた場合は、現バージョンの VFD.EXE または VFDWIN.EXE で、いったん手動
    に設定し、もう一度自動に設定しなおしてください。

    !!! 注意 !!!
    シェルエクステンションを無効にしても、Windows エクスプローラがすぐに DLL
    を開放せず、DLL ファイルの更新ができない場合があります。その場合、エクス
    プローラを再起動してください（例：一度ログオフして再ログオンする）。


アンインストール

    VFD.EXE または VFDWIN.EXE を使用します。

    - ドライバをシステムからアンインストールする
        (VFD.EXE)     VFD.EXE REMOVE
        (VFDWIN.EXE)  [ドライバ] -> [アンインストール]

    - シェルエクステンションを使用していた場合
        (VFD.EXE)     VFD.EXE SHELL /OFF
        (VFDWIN.EXE)  [シェル] -> [シェルエクステンション] のチェックをはずす
                      -> [適用]

    - ファイル関連付けを使用していた場合
        (VFDWIN.EXE)  [関連付け] -> [すべてクリア] -> [適用]

    その後、VFD ファイルをドライブから削除します。


履歴

    2008/02/06  バージョン 2.1.2008.0206 リリース
                -- zlib バージョンアップ、ユーザインタフェース微修正
    2005/04/04  バージョン 2.1 リリース
    2003/04/27  vfdwin.exe 1.01 -- ユーザインタフェースバグ修正
    2003/04/16  初版リリース


コピーライト

    このプログラムは Bo Branten 氏による ntifs.h を使用しています
    (http://www.acc.umu.se/~bosse/)。

      a free ntifs.h project
      Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004 Bo Branten.
      The GNU General Public License 

    このプログラムは Jean-loup Gailly 氏と Mark Adlerzlib 氏による zlib
    圧縮ライブラリを使用しています (http://www.gzip.org/zlib/).

      'zlib' general purpose compression library
      Copyright (C) 1995-2004 Jean-loup Gailly and Mark Adler

      This software is provided 'as-is', without any express or implied
      warranty.  In no event will the authors be held liable for any damages
      arising from the use of this software.

      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute it
      freely, subject to the following restrictions:

      1. The origin of this software must not be misrepresented; you must not
         claim that you wrote the original software. If you use this software
         in a product, an acknowledgment in the product documentation would be
         appreciated but is not required.
      2. Altered source versions must be plainly marked as such, and must not be
         misrepresented as being the original software.
      3. This notice may not be removed or altered from any source distribution.

    このプログラムは Gilles Vollant 氏による zlib コンパイル済みライブラリを
    使用しています (http://www.winimage.com/zLibDll/)。

      zlibstat.lib : The 32 bits statis library of zLib for Visual C++
      Copyright (C) Gilles Vollant


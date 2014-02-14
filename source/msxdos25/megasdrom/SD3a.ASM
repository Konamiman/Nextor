;MegaSDドライバ

;2007/10/15
;RDMBR, DSKCHG書き換え


;2007/09/04
;MMC14h7.ASMを進化
;SDアクセス中INITを実行する条件の中に、
;
;ドライブ種別 00  ドライブ未接続
;             01  ID0 SLOT 1  自動設定（ブロック数が00 xx xxは自動設定無し）
;             02  ID1 SLOT 2    〃
;             FF  似非ROMディスク

;ドライブ固有ワーク
; +0  セマフォ
; +1～+3  ldir / ret
; +4～+17 テーブル用バッファ

;DOSドライブ登録テーブルはバンク1Fh
;仮想FDドライブ登録テーブルはバンク1Eh
;仮想FDフラグ 00は「DOSモード」、FFhは「現在はDOSモードだが次回起動で仮想FD」
;　　　　　　 FEhは「このMegaSDでは仮想FDを使わない」
;　　　　　　 それ以外は仮想FDモード

;SD/MMCアクセスの「ldir」は固有ワークに置く
;固有ワークのアドレスはBANK 1Fh 5FAEhに保存


	org	07417h

KERNEL	equ	4969h		;bank3は5897h
DOSBK	equ	00h		;bank3は06h
SLTWRK	equ	4dd8h		;bank3は5fc2h
MFUNC	equ	07FCDh

BANK_4	equ	06000h
SECBUF	equ	0F34Dh
TRANSF	equ	0F36Eh
DATBNK	equ	07F9Ah

;**********************************************	
;名称:  CK_SPC
;機能:  パーティションテーブルを得る
;入力:  A=ローカルなドライブ番号
;出力:  IX=パーティションテーブルのアドレス
;       似非ＲＡＭディスク	Cy=0:Zf=1
;	SD/MMC			Cy=1:Zf=0
;	ＦＤＤ			Cy=1:Zf=1
;備考:　元のAFはAF'として保存される
;***********************************************

CK_SPC:
	push	af
	push	hl
	ld	hl,5f88h
	call	TERTBL
	pop	hl
	or	a		;Zf=1 DOS /Zf=0 EFD
	jr	z,CK_SP0	;DOSモード
	inc	a
	jr	z,CK_SP0	;5f88h=ff
	inc	a		;5f88h=fe?
	jr	nz,CK_SP1
CK_SP0:	pop	af
	jr	CK_SUB
	
CK_SP1:	pop	af
	call	CK_EFD		;仮想FDモード

CK_SUB:	exx			;
	ld	l,a
	ex	af,af'		;裏にする
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	bit	2,h
	ld	h,5fh
	jr	z,CK_DON
	dec	h
CK_DON:	call	TERTBL		;テーブルからhlの値を読む
	push	hl
	pop	ix
	exx
	sub	-1
	ret	nc		;似非RAM/ROM	Cy=0 Zf=1
	dec	a		;SD/MMC		Cy=1 Zf=0
	ret			;FDD		Cy=1 Zf=1

	


;***********************************************
;名称:  CK_EFD	
;機能:  仮想FD使用時、パーティションテーブルを得る
;入力:　A=ローカルなドライブ番号
;出力:　IX=パーティションテーブルのアドレス
;***********************************************


CK_EFD:	exx
	ld	c,a		;ドライブ
	ex	af,af'		;裏にする
	di			;
	ld	a,02h
	out	(099h),a
	ld	a,08fh
	out	(99h),a
CK_VDP:	in	a,(99h)
	and	81h
	dec	a
	jr	z,CK_VDP
	xor	a
	out	(99h),a
	ld	a,8fh
	out	(99h),a
	ld	a,c		;c=drive
	add	a,8ch
	ld	c,a		;drive A:8ch  drive B:8dh

CEFD_1:	call	TE_TBL		;bank 1fh テーブルを出す
	ret	c
	ld	hl,(05FAAh)	;媒体交換キー
	call	KEYCHK		;キー入力チェック
	jr	z,CEFD_6	;ディスク交換
	ex	af,af'		;表
	ld	b,5fh
	ld	a,(bc)
	set	6,a		;仮想FDのディスク番号 40h,41h....
	call	TE_DOS		;バンク戻し
	exx
	ei
	ret
	
CEFD_6:	in	a,(0aah)
	ld	l,a
CEFD_7:	call	CEFD_2
	jr	z,CEFD_7
CEFD_8:	call	CEFD_2
	jr	nz,CEFD_8
	ld	a,l
	out	(0aah),a
	jr	CEFD_1	

	
CEFD_2:	push	hl
	ld	h,10h		;テンキーとフルキーを使用
	ld	de,KTBL		;キーテーブルのポインタ
CEFD_3:	djnz	CEFD_3		;ランプ点滅速度調整ループ

CEFD_4:	push	hl
	ld	a,(de)		;テンキー
	ld	l,a
	call	KEY_IN
	inc	de
	jr	z,CEFD_C
	ld	a,(de)		;フルキー
	ld	l,a
	call	KEY_IN
	jr	nz,CEFD_5
	
CEFD_C:	ld	a,050h
	sub	h
	ld	b,5fh
	push	de		;キーテーブル
	push	bc		;拡張ドライブ5f8x
	push	af		;A=ディスク番号
	ld	hl,5f00h
	call	FL_TSF		;テーブルをバッファに転送
	pop	af
	pop	bc
	push	bc
	push	af
	ld	b,0ffh
	ex	de,hl
	add	hl,bc
	ld	(hl),a		;拡張ドライブに仮想FDのディスク番号
	call	FL_BUF		;flashルーチン転送
	exx
	push	hl		;転送アドレス保存
	push	de		;セクタ番号保存
	push	bc		;セクタ数保存
	ld	hl,5f00h
	call	FL_ERS		;flash消去＆書き換え
	
	pop	bc		;セクタ数
	pop	de		;セクタ番号
	pop	hl		;転送アドレス
	exx
	ld	a,1eh		;拡張テーブル
	ld	(6000h),a

	pop	af		;A=40h-4fh
	add	a,a
	add	a,a
	add	a,a
	add	a,a
	ld	l,a
	ld	h,5eh
	inc	hl
	push	hl
	call	FL_BF2		;res  7,(hl)ルーチンの転送
	ld	hl,FLASH6
	ld	bc,FLASH7-FLASH6
	ldir
	pop	de		;テーブルアドレス
	call	FL_BYT		;res  7,(hl)実行
	ld	a,1fh
	ld	(6000h),a
	pop	bc
	pop	de
CEFD_5:	pop	hl
	inc	de
	dec	h
	jr	nz,CEFD_4
	
	ld	l,074h		;R800-ROM mode
	call	CEFD_A
	ld	l,075h		;Z80 mode
	call	CEFD_A	
	
	pop	hl
	inc	h
	ld	a,h
	and	40h
	xor	l
	out	(0aah),a
	bit	6,b
	ret	

CEFD_A:	call	KEY_IN
	ret	nz
	ex	af,af'
	push	af
	push	bc
	ld	a,l
	cpl
	and	81h
	exx
	rst	30h			;CALLF
	db	80h
	dw	180h
	di
	exx
	pop	bc
	pop	af
	ex	af,af'
	ld	b,5Fh			;キーが押されたぞフラグ兼用
	ret
	

;***************************************
;SEMAPH
;入力: Cy=0:開放 Cy=1:確保
;出力: Cy=1:エラー,A=エラーコード
;****************************************

SEMAPH:	push	hl
	push	ix
	push	bc
	push	af
	call	SLTWRK
	pop	af
	pop	bc
	pop	ix
	jr	c,SMF_R1
	ld	(hl),01h	;開放 01
	pop	hl
	ret
	
SMF_R1:	sra	(hl)		;確保中は 00
	pop	hl
	ccf
	ret	nc
	ld	a,06h		;エラーコード'drive busy'
	ret
	
;***************************************
;KEYCHK
;指定した２つのキーが押されているかを調べる
;****************************************

KEYCHK:	call	KEY_R1
	ret	nz
	ld	l,h
KEY_R1:	push	hl
	ld	a,l
	and	15
	ld	h,a
	in	a,(0aah)
	and	0f0h
	or	h
	out	(0aah),a
	in	a,(0a9h)
	ld	h,a
	ld	a,l
	rlca
KEY_L1:	rlc	h
	add	a,20h
	jr	nc,KEY_L1
	xor	h
	rra
	sbc	a,a
	pop	hl
	ret

KEY_IN:	call	KEY_R1
	ret	nz
KEY_R2:	call	KEY_R1
	jr	z,KEY_R2
	xor	a
	ret

	
;***************
;SELECT
;DISCON
;	SCSI専用ファンクション
SCSI:	ret		

;***************************************************************************
;名称:	TO_SPC
;機能:	ＳＰＣを4000-5FFFhに出す
;入力:	無し
;出力:	衝突の恐れがあれば、Cy=1,A=エラーコード
;	IY=ＳＰＣレジスタのベースアドレス
;破壊:	AF
;***************************************************************************

TO_SPC:	scf				;SCSIをアクセスする宣言
	call	SEMAPH
	ret	c

;***************************************************************************
;名称:	TE_SPC
;機能:	TE_DOSで引っ込めたＳＰＣをまた表に出す
;入力:	無し
;出力:	IY=ＳＰＣレジスタのベースアドレス
;破壊:	特に無し
;***************************************************************************

TE_SPC:	push	af
	ld	a,40h
	ld	(6000h),a		;ＳＰＣのバンクに切り替える
	pop	af
	ret

	
;***************************************
;TO_TBL
;パーティションテーブルを4000-5FFFhに出す
;***************************************
TO_TBL:	scf
	call	SEMAPH	
	ret	c	

;***************************************
;TE_TBL
;	一時的にパーティションテーブルを4000-5FFFhに出す
;***************************************
TE_TBL:	push	af
	ld	a,01fh		;パーティションテーブル
	ld	(06000h),a
	pop	af
	ret
	
;****************************************
;SECSIZ
SECSIZ:	ld	a,b		;セクタ数
	exx
	ld	e,a		;DE'=セクタ数
	ld	d,0
	exx
	ret

;****************************************
;名称:	SECALC
;機能:	セクタ番号関係の処理をする
;入力:	B,CDEにセクタ数やセクタ番号を入れておく事
;       DE'は D'=00  E'=セクタ数 
;	IX=パーティションテーブル
;       ここに来る前にテーブルのバンクを4000h～にマッピングする
;出力:	Cy=1,A=E_OTHRならば、パーティション外へのアクセス
;	DEに論理ブロック番号,E'に論理ブロック数
;*****************************************

SECALC:	ld	a,c		;セクタ番号(上位）
	push	de		;セクタ番号(下位）
	exx			;裏にする

	pop	hl		;セクタ番号(下位）
	add	hl,de
	adc	a,d		;セクタ番号(上位）
	ld	d,a		;DHL=最終セクタ番号

	ld	a,(ix+7)	;総セクタ数下位
	sub	l
	ld	a,(ix+6)	;総セクタ数中位
	sbc	a,h
	ld	a,(ix+5)	;総セクタ数上位
	sbc	a,d
	
	exx
	ld	a,0Ch
	ret	c		;ブロック総数を越えてしまう

	ld	a,e		;開始ブロック＋セクタ番号
	add	a,(ix+4)
	ld	e,a
	ld	a,d
	adc	a,(ix+3)
	ld	d,a
	ld	a,c
	adc	a,(ix+2)
	ld	c,a
	
	xor	a
	ret



;*****************************************
BUSRES:
INISPC:
MANTFR:
HRDTFR:
	ret
;*****************************************
RRD_L1:
RD_L1:	bit	7,d
	jr	nz,RRD_L2
	ex	af,af'			;バンク番号を裏へ
	dec	de			;データがページ０、１の場合
	ld	a,d
	inc	de
	cp	3eh
	push	bc
	ld	b,1
	jr	nc,RRD_R2
	ex	af,af'			;バンク番号を表へ
	call	RRD_L2			;データがページ０の場合
RRD_R1:	pop	bc
	djnz	RRD_L1
	ret

RRD_R2:					;データがページ１の場合
	ex	af,af'			;バンク番号を表へ
	push	de
	ld	de,(SECBUF)
	call	RRD_L2
	pop	de
	push	hl
	ld	b,2
	ld	hl,(SECBUF)
	call	TE_DOS
	call	TRANSF
	ld	(BANK_4),a
	pop	hl
	jr	RRD_R1

RRD_L2:	ex	af,af'			;データがページ２、３の場合
	ld	a,b

	ld	b,2
	ldir

	ld	b,a
	ex	af,af'
	bit	5,h
	jr	z,RRD_R3		;まだ６０００ｈまで来ていない
	ld	h,40h			;４０００ｈに戻す
	inc	a			;バンク番号を１つ増やす
	ld	(BANK_4),a
RRD_R3:	djnz	RRD_L2
	ret


;*****************************************
;名称:	DISKIO
;機能:	論理セクタの読み書き
;入力:	A=ローカルなドライブ番号(0-7)
;	Cy=0:読み込み,Cy=1:書き込み
;	B=読み込むセクタ数
;	DE=読み込むセクタ番号
;	HL=読み込むアドレス
;出力:	Cy=0:正常終了,Cy=1:エラー
;	A=エラーコード
;破壊:	C,DE,AF',BC',DE',HL',IX,IY
;*****************************************

DISKIO:	ei
	call	CK_SPC		;ix=テーブルのアドレス
DSKIO:	jr	nz,S_DKIO	;SD/MMC
	jr	nc,R_DKIO
	ex	af,af'		;表
	ld	ix,4010h
	
CALDSK:	ex	af,af'		;裏
	push	af
	push	hl
	ld	hl,5f88h
	call	TERTBL		;a=仮想FDフラグの値
	pop	hl
	dec	a
	cp	08fh
	jr	c,CAL_R1
	pop	af
	ex	af,af'		;表
	ld	a,02h
	scf
	ret
	
CAL_R1:	inc	a
	push	af
	pop	iy		;FDDのスロット
	ex	af,af'		;表
	ld	a,0
	ld	c,0f9h		;media ID
	call	001Ch
	ex	af,af'		;裏
	pop	af
	ex	af,af'		;表
	ret
	
R_DKIO:				;似非RAM
	scf
	call	SEMAPH
	ret	c
	push	hl			;転送アドレスを待避
	ex	de,hl
	add	hl,hl			;セクタ番号を２倍する
	ld	a,l
	or	40h			;４０００ｈから
	and	5fh			;５ｆｆｆｈまで
	ld	e,0
	ld	d,a			;バンク内アドレス
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	a,(DATBNK)
	or	80h
	add	a,h			;A=バンク番号
	ld	c,e			;C=E=0
	ld	(BANK_4),a		;転送開始バンクを4000-5FFFhに出す
	pop	hl			;転送アドレスを復帰
	ex	af,af'
	jr	nc,R_RD
R_WR:	ex	af,af'
	jr	R_DONE
R_RD:	ex	af,af'
	ex	de,hl
	call	RRD_L1
	ex	de,hl
R_DONE:	xor	a
	jp	TE_DOS

;***************************
;SDカードアクセス

S_DKIO:	
	
SDACCS:	bit	7,c
	jr	z,SRW_R0	;C=セクタ番号上位
	ld	c,0
SRW_R0:	call	TE_BNK		;ドライブ登録テーブルを出す SEMAPH確保
	ret	c
	call	SECSIZ
	call	SECALC		;セクタ番号＋開始ブロックの計算
	jp	c,TE_DOS	;エラー
SRW_R1:	ld	a,(ix+0)	;ID
	dec	a		;a=slot
	push	ix		;ix=テーブルのアドレス
	call	TE_TBL		;bank 1fh
	ld	ix,(5faeh)
	inc	ix		;ixはED,B0,C9のアドレス
	call	TE_SPC		;bank 40h
	ld	(5fffh),a	;slot change
	ex	af,af'		;
	jr	nc,SRW_R3	;Read
	call	WR_R1		;書き込み
	pop	ix
	jp	TE_DOS		;開放
	
SRW_R3:	push	af		;a=drive
	call	RD_R1		;読みだし
	jr	nc,SRW_R4	;正常終了
	cp	0ch
	jr	z,SDBOOT	;カードが交換された
	
SRW_R4:	inc	sp
	inc	sp
	pop	ix
	jp	TE_DOS		;開放

;---------------
SDBOOT:	

SDBOT1:	di
	call	TE_DOS
	pop	af		;a=drive
	push	af
	push	hl		;DTA

	inc	ix		;固有ワーク＋１
	inc	ix
	inc	ix
	push	ix
	pop	hl

	ld	c,a		;c=drive
	push	hl
	push	bc
	ld	a,84h
	call	MFUNC		;固有ワークにテーブルリード
	
	call	RDMBR		;MBRを読む
	call	GETPTN		;パーティション設定
	
SDBOT2:	db	0ddh,0cbh,01h,0beh	;res  7,(ix+1)フラグリセット
	pop	bc
	pop	hl
	ld	a,85h
	call	MFUNC	
SDBOT3:	
	pop	hl		;DTA
	pop	af		;drive
	or	a
	ex	af,af'
	pop	ix		;ドライブ登録テーブルのポインタ
	ld	de,0
	ld	bc,0100h
	jr	SRW_R0


;**************
;ドライブ登録テーブルの値を読みだす
;入力:　HL=読み出すアドレス
;出力:　A=読み出した値


TORTBL:	scf			;確保		[SD_2]
	call	SEMAPH
	ret	c
	ld	a,1fh
	and	h
	ld	(6000h),a
	ld	a,(hl)
	
;DOSバンクに戻す
	
TO_DOS:	push	af
	ld	a,DOSBK		;dos1
	ld	(6000h),a
	xor	a
	call	SEMAPH		;開放
	pop	af
	ret
	
	
;*************
TERTBL:	ld	a,1fh
	and	h
	ld	(6000h),a
	ld	a,(hl)

;TE_DOS

TE_DOS:	push	af
	ld	a,DOSBK
	ld	(6000h),a
	pop	af
	ret
	

;***************************
TO_BNK:	scf			;確保		[SD_2]
	call	SEMAPH
	ret	c

TE_BNK:	push	af		;テーブルに対応するバンクに切り換える
	push	ix
	pop	af
	bit	7,a
	jr	nz,TE_BK1	;ページ3のバッファにテーブルを読み込んだとき
	and	1fh
	ld	(6000h),a
TE_BK1:	pop	af
	ret



;******************************************
;DSKCHG
;機能:	ディスクが交換されたかを調べる
;入力:	A=ローカルなドライブ番号(0-7)
;	HL=読み込むアドレス
;出力:	正常終了:Cy=0	B= 1:交換されていない
;			B= 0:交換されたか不明
;			B=-1:交換された
;	異常終了:Cy=1	A=エラーコード
;			B= 0:交換されたか不明
;破壊:	C,AF',BC',DE',HL',IX,IY
;******************************************

DSKCHG:	ei			;		[SD_3]
	call	CK_SPC
		
DCG_R1:	push	hl
	call	c,DCG_S1	;ディスク交換を調べる
				;（セクタバッファにブートセクタをリード）
	pop	de		;読み込むアドレス
	ld	b,0
	ret	c		;Cy=1 B=0 (不明）
	inc	b
	call	TE_BNK		;ixのアドレスに対応したバンクに切り替え
	db	0ddh,0cbh,01h,7eh	;bit  7,(ix+1)
	jp	nz,TE_DOS	;Cy=0 B=1 交換されていない
	call	TE_DOS
	ex	af,af'		;local drive
	ex	de,hl
	or	a
	
	push	hl		;読み込むアドレス
	push	af		;a=drive Cy=0 read
	call	TE_TBL		;bank 1fh
	ld	hl,(5faeh)	;固有ワークのアドレスを得る
	call	TE_DOS
	inc	hl
	inc	hl
	inc	hl
	inc	hl
	push	hl		;MFNC#84hのバッファ
	pop	ix		;テーブルのポインタ
	ld	c,a		;c=drive
	push	hl
	push	bc
	ld	a,84h
	call	MFUNC		;テーブルを読み込む
	call	RDMBR		;MBRの読み出し
	call	GETPTN		;パーティション設定
	
	db	0ddh,0cbh,01h,0feh	;set  7,(ix+1)フラグを立てる
	pop	bc
	pop	hl
	ld	a,85h
	call	MFUNC
	pop	af		;a=drive Cy=0
	pop	hl		;hl=読み込むアドレス
	call	CK_SUB
	call	GTDPB_		;DPBテーブル初期化
	ld	b,-1		;B=-1 抜き差しされた
	ret	nc		;正常終了
	cp	0ah
	ret	z
	scf
	inc	b
	ret			;B=0 エラー
	
;-------------------------
DCG_S1:	jr	nz,DCG_S2	;SD/MMC

	ld	ix,04013h	;FDD
	jp	CALDSK
	
DCG_S2:	push	ix		;テーブルのアドレス
	ex	af,af'		;
	push	af		;drive
	call	RDMBR_		;開始ブロック書き換え無しでセクタ０を読み込む
	ex	af,af'		;
	pop	af		;drive
	pop	ix
	ex	af,af'		;セクターリードの戻り値
	or	a
	ret	z		;正常に読み出しできた　交換されてない
	cp	02h
	scf
	ret	z		;ディスクが抜かれている？
	

	call	TE_BNK		;テーブルに対応するバンクに切り替え
	ld	c,(ix+0)	;ＩＤ番号を得る	
	call	TE_DOS		;
	push	ix
	push	bc
	call	FL_BF2		;1バイト書き換えルーチン転送
	ld	hl,FLASH6	;res  7,(hl)
	ld	bc,FLASH7-FLASH6
	ldir

	pop	bc
	ld	b,24		;テーブルの数が２４個・・・多いなぁ
	ld	de,15		;１テーブルあたり１６バイト
	ld	hl,5e00h
DCG_S5:	ld	a,1fh		;テーブルの値を読む
	and	h
	ld	(6000h),a
	ld	a,(hl)
	cp	c		;同じＩＤのドライブを全て「交換された」
	inc	hl		;にする
	jr	nz,DCG_S6
	push	hl
	push	de
	push	bc
	ex	de,hl
	call	FL_BYT		;res  7,(hl)
	pop	bc
	pop	de
	pop	hl
DCG_S6:	add	hl,de
	djnz	DCG_S5
	pop	ix
	xor	a
	jp	TE_DOS		;dosバンクに戻す SEMAPH 開放
	

;**************************************
;GETDPB

GETDPB:	ei
	or	a		;似非RAM    Cy=0 Z=1
	call	CK_SPC		;MMC        Cy=1 Z=0
	                        ;FDD        Cy=1 Z=1
	
GTDPB_:	push	hl
	ld	hl,(0f34dh)
	ld	de,0
	ld	bc,01ffh
	call	DSKIO
	pop	hl
	ret	c
	
DPB_R1:	ld	iy,(0f34dh)
	inc	hl
	push	hl
	ld	bc,18
	ex	de,hl
	ld	hl,07405h	;DPBデータ
	ldir
	pop	hl

	ld	a,(iy+15h)
	cp	0f9h			;２ＤＤかな？
	ret	z
	ld	(hl),a			;メディアＩＤ
	inc	hl

	ld	a,(iy+0bh)
	ld	(hl),a			;バイト／セクター（下位）
	inc	hl
	ld	a,(iy+0ch)
	ld	(hl),a			;バイト／セクター（上位）
	inc	hl

	ld	(hl),0fh		;ディレクトリ・マスク
	inc	hl
	ld	(hl),4			;ディレクトリ・シフト
	inc	hl

	ld	a,(iy+0dh)
	dec	a
	ld	(hl),a			;クラスタ・マスク(0,1,3,7,15)
	inc	hl
	add	a,1			;結果が零の時、Cy=1になる
	ld	b,0			;（デタラメなブートセクターの場合に
DPB_L1:	inc	b			;　備え、無限ループしない様にする為）
	rra				;A!=0か、Cy=1なら無限ループを回避
	jr	nc,DPB_L1
	ld	(hl),b			;クラスタ・シフト(1,2,3,4,5)
	inc	hl

	push	bc			;クラスタ・シフト

	ld	a,(iy+0eh)
	ld	(hl),a			;ＦＡＴ先頭セクタ番号（下位）
	inc	hl
	ld	d,(iy+0fh)
	ld	(hl),d			;ＦＡＴ先頭セクタ番号（上位）
	inc	hl
	ld	b,(iy+10h)
	ld	(hl),b			;ＦＡＴの数
	inc	hl
DPB_L2:	add	a,(iy+16h)		;ＦＡＴ領域のセクタ数
	jr	nc,DPB_R2
	inc	d
DPB_R2:	djnz	DPB_L2
	ld	c,a
	ld	b,d			;ディレクトリ先頭セクタ番号

	ld	e,(iy+11h)
	ld	d,(iy+12h)
	ld	a,d
	or	a
	ld	a,254
	jr	nz,DPB_R3
	ld	a,e
DPB_R3:	ld	(hl),a			;ディレクトリ数
	inc	hl
	dec	de

	ld	a,4
DPB_L3:	srl	d
	rr	e
	dec	a
	jr	nz,DPB_L3
	inc	de
	ex	de,hl
	add	hl,bc
	ex	de,hl
	ld	(hl),e			;データ領域先頭セクタ番号（下位）
	inc	hl
	ld	(hl),d			;データ領域先頭セクタ番号（下位）
	inc	hl

	ld	a,(iy+13h)		;総セクタ数（下位）
	sub	e
	ld	e,a
	ld	a,(iy+14h)		;総セクタ数（上位）
	sbc	a,d
	ld	d,a			;データ領域のセクタ数

	pop	af			;クラスタ・シフト

DPB_L4:	dec	a
	jr	z,DPB_R4
	srl	d
	rr	e
	jr	DPB_L4
DPB_R4:	inc	de
	ld	(hl),e			;「クラスタ数＋１」（下位）
	inc	hl
	ld	(hl),d			;「クラスタ数＋１」（上位）
	inc	hl

	ld	a,(iy+16h)
	ld	(hl),a
	inc	hl

	ld	(hl),c			;ディレクトリ先頭セクタ番号（下位）
	inc	hl
	ld	(hl),b			;ディレクトリ先頭セクタ番号（上位）

	xor	a
	ret

;***************************************************************************
;名称:	CHOICE
;機能:	フォーマットメッセージを返す
;入力:	A=ローカルなドライブ番号
;出力:	HL=0:メッセージが無い／似非ＲＡＭディスク
;	(HL)=0:フォーマット不可能なドライブ
;破壊:	IX
;備考:	似非ＳＣＳＩでは0,'SCSI5FF0',0へのポインタを返す
;***************************************************************************

CHOICE:	call	CK_SPC			;似非ＲＡＭディスク	Cy=0:Zf=1
					;ＳＣＳＩ		Cy=1:Zf=0
					;ＦＤＤ			Cy=1:Zf=1

	ld	hl,0			;ＲＡＭディスク
	ret	nc
	ld	hl,CHO_D1		;ＳＣＳＩ、ＦＤＤ


MTROFF:
	xor	a
	ret

CHO_D1:	db	0,'SCSI5FF0',0
	

;****************************************
;DSKFMT
DSKFMT:
	ld	a,010h
	scf
	ret
	
;****************************************
;INIHRD
;セクタバッファは使えない
;バンク0 47D6hから呼ばれる
;ドライバ切り離し以外はバンク1Eh、1Fhを書き換える
;****************************************
	
INIHRD:	call	TE_TBL
	ld	hl,(05fa2h)	;[GRAPH]+[SPACE]
	call	KEYCHK
	jr	nz,IHRD_1
	call	TE_DOS
	pop	af		;ドライブ切り離し
	ret
	

IHRD_1:
	ld	hl,(05fa0h)	;初期化 GRAPH+ESC
	call	KEYCHK
	ex	af,af'		;裏にフラグ保存
	ld	a,1eh
	ld	(6000h),a
	ld	hl,5e00h	;仮想FDテーブル転送
	ld	de,0d000h
	ld	bc,0100h
	ldir
	ex	af,af'	
	jr	nz,IHRD_2	;初期化しない
	
	ex	af,af'		;裏にフラグ保存
	ld	hl,0d000h	;仮想FDテーブルの初期化
	ld	de,0d001h
	ld	bc,00ffh
	ld	(hl),0
	ldir
	jr	IHRD_3		;bank 1eh書き換え
	
IHRD_2:	ex	af,af'		;裏にフラグ保存
	ld	hl,0d000h
	ld	de,000fh
	ld	b,10h
IHRD_4:	ld	a,(hl)
	inc	hl
	or	a		;仮想FDテーブルの
	jr	z,IHRD_5	;使われているドライブの
	set	7,(hl)		;交換フラグをセット
IHRD_5:	add	hl,de
	djnz	IHRD_4
	
;仮想FDテーブル書き込み
IHRD_3:	ld	hl,0d000h
	call	FL_BUF+3	;flashルーチン転送
	ld	hl,5e00h
	ld	de,0d000h
	call	FL_ERS+4	;フラッシュ書き換え
	
;DOSテーブル
	ld	a,1fh
	ld	(6000h),a

	ex	af,af'		;初期化起動キーの状態
	jr	nz,IHRD_6	;初期化しない

	xor	a
	ld	de,0d000h
IHRD6a:	inc	a
	ld	(de),a
	inc	de
	ld	hl,7f71h	;00,00,00,00,ff,ff,ff,02,00
	ld	bc,000fh
	ldir
	cp	02h
	jr	c,IHRD6a
	ld	l,e
	ld	h,d
	inc	de
	ld	bc,005fh
	ld	(hl),0
	ldir
	ld	hl,7f80h	;初期化は7f80hからテーブル転送
	ld	bc,0080h
	ldir
	jr	IHRD_9
	
IHRD_6:	ld	hl,5f00h	;初期化しないdosテーブル転送
	ld	de,0d000h
	ld	bc,0100h
	ldir
	ld	b,08h
	ld	de,000fh
	ld	hl,0d000h
IHRD_7:	ld	a,(hl)
	inc	hl
	or	a
	jr	z,IHRD_8	;使われていないドライブ
	set	7,(hl)		;ディスク交換フラグをセット（交換されていない）
IHRD_8:	add	hl,de
	djnz	IHRD_7
	ld	hl,(05fa8h)	;仮想FDモード解除キー
	call	KEYCHK
	jr	nz,IHRD_9
	ld	hl,0d088h	;仮想FDモード解除
	xor	a
	ld	(hl),a
	
IHRD_9:	ld	a,(5f88h)
	cp	0ffh
	jr	nz,IHRD_A
	res	4,a
	ld	(0d088h),a	;仮想FDフラグをEFh(仮想FDモード)にする
	
IHRD_A:	ld	de,0d000h
	ld	hl,5f00h
	call	FL_ERS+4	;bank 1fh書き換え
	ld	a,(5f88h)
	cp	0feh
	jr	nz,IHRD_B
	pop	af
IHRD_B:	jp	TE_DOS		;DOSバンクに戻す

;*******************
;DRIVES     INITの途中(48C6h)から呼ばれる
;パーティション設定（いきなりFAT16のため）

	push	af
	push	bc
	push	de
	call	SLTWRK		;DOS1: CALL 5FC2h  HL=SLOT WORK
	push	hl
	ld	a,01h
	ld	(hl),a		;セマフォ semaph
	inc	hl
	ld	a,0edh
	ld	(hl),a
	inc	hl
	ld	a,0b0h
	ld	(hl),a
	inc	hl
	ld	a,0c9h
	ld	(hl),a
	inc	hl
	ld	e,l		;ファンクション用バッファの初期化
	ld	d,h
	inc	de
	ld	(hl),0
	ld	bc,15
	ldir
	pop	hl
	call	IWRK_S		;拡張テーブルの初期化、パーティション設定
	ld	hl,5f99h	;システム領域のバンク番号
	call	TERTBL		;テーブルから1バイト読む
	ld	(7000h),a
	inc	a
	ld	(7800h),a
	ld	hl,5f88h	;仮想fdフラグ
	call	TERTBL
	or	a
	jr	nz,DRV_R4	;仮想fdで起動
	
	ld	a,1fh
	ld	(6000h),a
	ld	hl,(05fa6h)	;ctrl起動
	call	KEYCHK
	and	02h
	ld	(0f33fh),a
	call	TE_DOS
	jr	DRV_R5	
	
DRV_R4:	ld	hl,0fd99h
	inc	(hl)
	pop	de
DRV_R5:	ld	hl,05f80h	;確保するドライブ数
	call	TERTBL
	ld	l,a		;L=ドライブ数
	pop	de
	pop	bc
	pop	af
	ret	
	
;***************************************************************************
;名称:	INIWRK
;機能:	拡張ドライブテーブルを初期化する
;	DOSテーブル、パーティション設定

IWRK_S:	di
	ld	de,0d000h
	ld	(0f34dh),de	;セクタバッファを仮設定
	ld	(ix+4),l	;固有ワークのアドレス
	ld	(ix+5),h
	inc	hl
	inc	hl
	inc	hl
	inc	hl
	call	SLT_WK		;bank 1fh 5FAEhに保存
	ld	hl,5f88h	;仮想FDフラグ
	call	TERTBL		;テーブルから1バイト読み出し
	or	a		;この時点で仮想FDフラグは00h(DOS)かEFh(仮想FD)
	jr	nz,SPRT_5	;仮想FDモード

	ld	a,1eh
	ld	(6000h),a
	ld	hl,0d000h	;仮想FDテーブルのクリア
	ld	de,0d001h
	ld	bc,0100h
	ld	(hl),l
	ldir
	call	FL_BUF
	ld	hl,5e00h
	call	FL_ERS		;flash消去&書き込み
	call	TE_DOS		;bank 0
;partition
	call	SLTWRK		;固有ワークのポインタ
	inc	hl
	inc	hl
	inc	hl
	inc	hl		;M-functionのバッファ
	push	hl
	push	hl
	ld	hl,5f80h	;ドライブ数
	call	TERTBL
	ld	b,a		;ドライブ数
	ld	c,0
	pop	hl		;バッファのポインタ
	pop	ix
SPRT_2:	push	bc
	push	hl
	ld	a,84h		;DOSテーブル読み出し
	call	MFUNC
	ld	a,(ix+0)
	inc	a
	jr	z,SPRT_4	;似非ROMディスク
	ld	a,(ix+5)
	or	a
	jr	z,SPRT_4	;仮想FDのドライブ割り当て
	call	RDMBR		;MBRの読み込み
	or	a
	jr	nz,SPRT_3	;read error
	call	GETPTN		;super floppy以外はパーティション設定
	
SPRT_3:	db	0ddh,0cbh,01h,0beh	;res 7,(ix+1)
	pop	hl
	pop	bc
	push	bc
	push	hl
	ld	a,85h
	call	MFUNC
	
SPRT_4:	pop	hl
	pop	bc
	inc	c
	djnz	SPRT_2
SPRT_5:	ld	hl,0		;セクタバッファの仮設定解除
	ld	(0f34dh),hl
	jp	TE_DOS




;**********************************************
;E_RUNC

E_RUNC:	ld	hl,5f88h
	call	TERTBL
	or	a
	jr	z,RUNC_2	;dosモードで起動
				;この時点でフラグは00hかEFh
	ld	a,1fh
	ld	(6000h),a
	ld	hl,0d000h
	call	FL_BF2+3	;1バイト書き換えルーチン転送
	ld	hl,FLASH7
	ld	bc,FLASH8-FLASH7
	ldir
	
	ld	de,5f88h	;FDドライブのスロット
	call	FL_BYT		;ld  a,(0fb22h)/ ld  (5f88h),a

	call	TE_DOS
	call	0402dh		;MegaSDのスロットを得る
	ld	(0fb22h),a
	ld	(0fb2ch),a
	ld	hl,07fdah
	ld	(0fb2dh),hl
	JR	RUNC_4
	
RUNC_2:	ld	hl,5f81h	;起動ドライブ
	call	TERTBL
	or	a
	jr	z,RUNC_4
	ld	(0f2fdh),a
RUNC_4:	jp	KERNEL


;****************************************
;M_SFNC

	push	hl
	push	af
	ld	hl,FNCTBL
SFC_L1:	sub	(hl)
	inc	hl
	jr	C,SFC_R1
	inc	hl	
	inc	hl	
	jr	SFC_L1
SFC_R1:	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	pop	af
	ex	(sp),hl	
	ld	iy,5ff0h
	ret
	
FNCTBL:	DB	8
	DW	SFC_00			;func 00h (DISKIO)
	DB	8
	DW	SFC_ER			;そんなファンクションは無いぞ
	DB	8
	DW	SFC_10			;func 10h (DSKCHG)
	DB	40h-18h
	DW	SFC_ER			;そんなファンクションは無いぞ
	DB	16
	DW	SFC_40			;func 40h (ex-DISKIO)
	DB	16
	DW	SFC_50			;func 50h (ex-DSKCHG)
	DB	80h-60h
	DW	SFC_ER			;そんなファンクションは無いぞ
	DB	4
	DW	SFC_80			;func 80h (DOStable get/free)
	DB	4			;
	DW	SFC_84			;func 84h (DOStable read/write)

	DB	0a0h-88h
	DW	SFC_ER			;そんなファンクションは無いぞ

	DB	1
	DW	SFC_A0			;func A0h (get information)
	DB	1
	DW	SFC_ER
	DB	2
	DW	SFC_A2			;func A2h (table read/write)
	DB	2
	DW	SFC_A4			;func A4h (table read/write)
	DB	2
	DW	SFC_A6			;func A6h (table read/write)
	DB	2
	DW	SFC_A8			;func A8h (table read/write)
	DB	2
	DW	SFC_AA			;func AAh (table read/write)
	DB	0c0h-0ach
	DW	SFC_ER			;そんなファンクションは無いぞ

	DB	3			;C0h-C2h
	DW	SCSI			;SCSI専用ファンクション

	DB	1
	DW	SEMAPH			;C3h せまふぉっふぉ
	DB	1
	DW	TE_DOS			;C4h ＤＯＳバンクに戻す
	DB	1
	DW	TE_DOS			;C5h 一時的にＤＯＳバンクに戻す
	DB	8			;C6h-CDh
	DW	SCSI			;SCSI専用ファンクション


__FF__:	DB	-1			;最後にダメ押し＆ダミーセマフォレジスタ
	DW	SFC_ER			;そんなファンクションは無いぞ

SFC_ER:	scf
	sbc	a,a
	ret
	

SFC_00:				;00h-07h
SFC_40:	call	CK_SUB		;40h-4Fh
	jp	nc,R_DKIO
	jp	S_DKIO
	
SFC_10:
SFC_50:	call	CK_SUB
	jp	DCG_R1
	
SFC_80:	rra
	jr	c,SFC_P1
SFC_O1:	push	hl			;パーティションテーブルのオープン
	rra
	ld	bc,800h
	ld	de,16
	ld	hl,5f00h
	jr	nc,SFC_O2
	sla	b
	dec	h
SFC_O2:	call	TERTBL
	or	a
	jr	z,SFC_O3
	add	hl,de
	inc	c
	djnz	SFC_O2
	pop	hl
	ld	a,0ch
	scf
	ret
SFC_O3:	pop	de			;80h,82h空きテーブルに書き込み
	push	bc
	ex	de,hl
	ld	a,01h
	call	SFC_T2			;A=1なので(DE)->(HL)の転送になる
	pop	bc
	ret

SFC_P1:	ld	hl,07fc0h		;テーブル開放
	rla				;ビット１は拡張テーブルのフラグ
SFC_84:	ld	d,5fh
	bit	1,a
	jr	z,SFC_T1
	dec	d
SFC_T1:	ld	b,a
	ld	a,c
	add	a,a
	add	a,a
	add	a,a
	add	a,a
	ld	e,a
	ld	a,b
SFC_T2:	ld	c,16
SFC_T3:	ld	b,0
	rra
	jr	nc,SFC_T4	;テーブル読み出し
;	scf			;		[SD_2]
;	call	SEMAPH
;	ret	c

	push	de		;de=table
	push	bc		;転送バイト数
	push	hl		;hl=buffer
	ex	de,hl		;hl=table
	call	FL_TSF		;テーブルデータを転送
	ld	h,0ffh
	add	hl,de		;書き換えるテーブルのアドレス
	pop	de		;buffer
	pop	bc		;バイト数
	ex	de,hl
	ldir			;目的のテーブルを書き換え
	call	FL_BUF		;flashルーチン転送
	pop	hl		;table
	ld	l,0		;5x00h
	call	FL_ERS		;flash消去&書き換え
	jr	nc,SFC_RE
	
	call	TE_DOS		;エラー
	ld	a,0ch
	scf
	ret

SFC_T4:	
;	scf			;		[SD_2]
;	call	SEMAPH
;	ret	c
	ex	de,hl		;hl=table
	ld	a,1fh
	and	h
	ld	(6000h),a
	ldir
SFC_RE:	xor	a		;正常終了
	jp	TE_DOS

SFC_A0:	xor	a
_TYPE:	ld	a,04h		;ハードウェア仕様コード
	ld	bc,0215h
	ret
SFC_A2:	ld	c,2
	ld	de,5f90h
	jr	SFC_T3
SFC_A4:	ld	c,4
	ld	de,5f98h
	jr	SFC_T3
SFC_A6:	ld	c,8
	ld	de,5f80h
	jr	SFC_T3
SFC_A8:	ld	c,8
	ld	de,5f88h
	jr	SFC_T3
SFC_AA:	ld	c,12
	ld	de,5fa0h
	jr	SFC_T3
SLT_WK:	ld	a,1		;固有ワークのアドレスを書き込む
	ld	c,2
	ld	de,5faeh
	jr	SFC_T3

;********************
;バンク0  4904hから呼ばれる

	RET


;***************************
;RDMBR    MBRの読み込み
;入力：	ix=ドライブ登録テーブルのポインタ

RDMBR:	xor	a		;		[SD_3]
	ld	(ix+02h),a	;sector h
	ld	(ix+03h),a	;sector m
	ld	(ix+04h),a	;sector l
	
RDMBR_:	ld	hl,(0f34dh)
	ld	c,0		;Read sector 0
	ld	de,0
	ld	b,01h
	call	TE_BNK		;ドライブ登録テーブルを出す
	ld	a,(ix+0)	;ID
	dec	a		;a=slot number
	call	TE_SPC
	ld	(5fffh),a	;slot change

DCG_A3:	call	DCG_A4		;セクターリード
	jp	TE_DOS		;BANK戻し
	
DCG_A5:	scf			;
DCG_A6:	ld	a,02h		;
	ret			;
DCG_A7:	call	INIT		;
	pop	bc		;
	pop	de		;
	pop	hl		;
	jr	nz,DCG_A5	;
	jr	c,DCG_A6	;
	ld	a,04h		;
	ret			;
	
DCG_A4:	push	hl		;
	push	de		;
	push	bc		;
	ld	b,051h		;
	ld	hl,04000h	;
	call	MMCCMD		;
	jr	c,DCG_A7	;time out
	pop	bc		;
	pop	de		;
	pop	hl		;
	or	a		;
	jr	nz,DCG_A5	;エラー Cy=0 Z=0
	push	de		;
	push	bc		;
	ex	de,hl		;de=転送先アドレス
	ld	bc,0200h	;バイト数
	ld	hl,04000h	;
DCG_A8:	ld	a,(hl)		;
	cp	0feh		;
	jr	nz,DCG_A8	;

	ldir			;
	ld	a,(hl)		;
	ld	a,(hl)		;
DCG_B1:	ex	de,hl		;
	pop	bc		;
	pop	de		;

	xor	a		;
	ret			;
	
	
;*****************************
;パーティション設定

GETPTN:	
	ld	hl,(0F34Dh)
FTYPE_:	ld	bc,0100h
FTYPE1:	ld	a,46h		;'F'
	cpir
	jr	z,FTYPE2
	jr	GETP_0		;HDDフォーマット
	
	
FTYPE2:	push	hl
	ld	d,(hl)
	inc	hl
	ld	e,(hl)
	ld	hl,4154h	;'A' 'T'
	or	a
	sbc	hl,de
	pop	hl
	jr	nz,FTYPE1
	ret			;「FAT」の文字があればsuper floppy
	
;----------------------------------
;パーティションテーブルを見つける
GETP_0:	
	ld	hl,(0F34Dh)	;

GETPT_:	ld	b,04h		;+1c6～1f6からパーティションテーブルを
	ld	de,01c6h
	add	hl,de
GETP_1:
	push	hl
	ld	e,(hl)		; l
	inc	hl
	ld	d,(hl)		; m
	inc	hl
	ld	c,(hl)		; h
	ld	a,c
	or	d
	or	e
	pop	hl
	jr	nz,GETP_2	;パーティションテーブル見つけた
	ld	de,0010h
	add	hl,de
	djnz	GETP_1
	scf			;見つからない　開始ブロックは00 00 00のまま
	ret			;

GETP_2:	
	ld	(ix+2),c		;開始ブロックh
	ld	(ix+3),d		;開始ブロックm
	ld	(ix+4),e		;開始ブロックl
	or	a			;開始ブロック設定
	ret
	
	

;***********
;いきなりFAT16
;バンク0 4B19hから呼ばれる
;CHKVOL
	CALL	05243H		;ブートセクタの+20hから「VOL_ID」を探す
	RET	Z		;見つかればMSX-DOS2
	CALL	CHK_V1		;無ければ「FAT1」を探す
	RET	NZ		;どちらも見つからない→DOS1起動
	LD	A,0C9H		;Windowsでフォーマットしたディスク
	LD	(0C01EH),A	;ブートセクタがMSXと違っているので細工する
	RET			;
	NOP			;
CHK_V1:	LD	HL,0C000H	;
	LD	BC,0100H	;
CHK_V5:	LD	A,046H		;
	CPIR			;
	JR	Z,CHK_V2	;
	RET			;
CHK_V2:	PUSH	HL		;
	PUSH	BC		;
	LD	B,03H		;
	LD	DE,MOJI		;
CHK_V4:	LD	A,(DE)		;
	XOR	(HL)		;
	JR	NZ,CHK_V3	;
	INC	HL		;
	INC	DE		;
	DJNZ	CHK_V4		;
CHK_V3:	POP	BC		;
	POP	HL		;
	JR	NZ,CHK_V5	;
	RET			;
	
MOJI:	DB	'AT1'		;

;***********************************************
;SD/MMCアクセスルーチン
;入力:　HL=DTA   C,DE=セクタ番号  B=セクタ数 IX=テーブル
;出力:  正常終了　Cy=0  Z=1  A=00
;       異常終了　Cy=1  Z=0  A=エラーコード

;*** MMC command handler
MMCCMD:	ld	a,(hl)		;dummy cycle 8 clocks
	nop			;			[SD_1]
	nop
	sla	e
	rl	d
	rl	c
	ld	(hl),b		;command
	nop
	ld	(hl),c		;sector(H)
	nop
	ld	(hl),d		;sector(M)
	nop
	ld	(hl),e		;sector(L)
	ld	(hl),0		;sector(0)
	ld	(hl),95h	;CRC
	ld	a,(hl)		
	ld	a,(hl)		;			[SD_1]
	ld	bc,10h
CMD_L1:	ld	a,(hl)
	cp	0ffh
	ccf
	ret	nc
	djnz	CMD_L1
	scf			;timeout error
	ret

;*** Initialize / set to SPI mode
INIT:	ld	b,10		;dummy cycle > 76 clocks
INIT_L:	ld	a,(5000h)
	djnz	INIT_L

	ld	bc,4000h	;CMD0
	ld	de,0
	ld	hl,4000h
	call	MMCCMD

	ret	c		;response timeout
	and	0f3h		;F7=>F3hに変更(Nokia MMC対応)
	cp	01h
	ret	nz


	
INIT_M:	
	
	ld	bc,4100h	;CMD1 for MMC
	ld	de,0
	ld	hl,4000h
	call	MMCCMD

	ret	c		;response timeout
	cp	01h
	jr	z,INIT_M
	or	a		;z=1: OK  z=0: error 
	ret


;*************************************************
RD_E1:	scf			;Cy=1 
RD_E2:	ld	a,02h		;error code
	ret


RD_R0:	call	INIT
	pop	bc
	pop	de
	pop	hl
	jr	nz,RD_E1	;response error
	jr	c,RD_E2		;command error
	
	ld	a,0ch		;Other error		[SD_1]
	scf			;カードが抜き差しされた
	ret

RD_R1:	
	push	hl
	push	de
	push	bc

	ld	b,51h+1		;CMD17 => CMD18
	ld	hl,4000h
	call	MMCCMD
	jr	c,RD_R0

	pop	bc
	pop	de
	pop	hl
	or	a
	jr	nz,RD_E1	;Cy=0 A=02
RD_X1:
	push	de
	push	bc

	bit	7,h
	jr	z,RD_X2

RDX_R1:	ex	de,hl
	ld	a,0feh		;start data token
	ld	l,b
	ld	c,0
RDX_R2:	ld	b,2
	ld	h,40h
RDX_R3:	cp	(hl)		;start data token ?
	jr	nz,RDX_R3
	call	0f398h		;ldir/ ret
	cp	(hl)		;CRC (dummy)
	cp	(hl)		;CRC (dummy)
	dec	l
	jr	nz,RDX_R2

	ld	b,4Ch		;CMD12 / stop multiblock read
	call	MMCCMD

	ex	de,hl
	pop	bc
	pop	de
	xor	a		;A=00 Cy=0 正常終了
	ret

RD_X2:	ex	de,hl
	ld	bc,200h
	ld	hl,04000h
	
RD_W1:	ld	a,(hl)		
	cp	0feh		;start data token
	jr	nz,RD_W1

	bit	7,d
	jr	nz,RD_W2
	dec	de
	ld	a,d
	inc	de
	cp	3eh
	jr	c,RD_W2
	push	de		;転送先アドレス
	ld	de,(0f34dh)
	push	de
	ldir
	ld	a,(hl)		;CRC(dummy)
	ld	a,(hl)

	pop	hl		;(f34dh)
	pop	de		;転送先アドレス
	ld	bc,200h
	call	TE_DOS		;bank 0
	call	0f36eh
	call	TE_SPC		;bank 40h
	jr	RD_W3

RD_W2:	call	0f398h		;ldir/ ret
	ld	a,(hl)		;CRC (dummy)
	ld	a,(hl)		;CRC (dummy)

RD_W3:	ex	de,hl
	pop	bc
	pop	de
	inc	de		;CDE = sector number
	ld	a,d
	or	e
	jr	nz,RD_W4
	inc	c
RD_W4:	djnz	RD_X1

	push	hl
	ld	b,4Ch		;CMD12 / stop multiblock read
	ld	hl,4000h
	call	MMCCMD
	pop	hl

	xor	a		;A=00 Cy=0 正常終了
	ret

;*************************************************
WR_E0:	pop	bc
	pop	de
	
WR_E1:	scf			;Cy=1 

WR_E2:	ld	a,02h		;Cy=1 A=02
	ret

WR_R0:	call	INIT
	pop	bc
	pop	de
	pop	hl
	jr	nz,WR_E1	;response error
	jr	c,WR_E2		;command error
	
WR_R1:	
	push	hl
	push	de
	push	bc
	ld	b,58h		;CMD24
	ld	hl,4000h
	call	MMCCMD
	jr	c,WR_R0
	pop	bc
	pop	de
	pop	hl
	or	a
	jr	nz,WR_E1	;command error

	push	de
	push	bc
	
	ld	bc,200h
	ld	de,04000h

	ld	a,(de)		;dummy
	ld	a,0feh		;start data token
	ld	(de),a

	bit	7,h
	jr	nz,WR_W2
	dec	hl
	ld	a,h
	inc	hl
	cp	3eh
	jr	c,WR_W2
	ld	de,(0f34dh)
	call	TE_DOS		;bank 0
	call	0f36eh
	call	TE_SPC		;bank 40h
	push	hl
	ld	hl,(0f34dh)
	ld	de,04000h
	ld	bc,200h
	ldir
	pop	hl
	jr	WR_W3
	

WR_W2:	call	0f398h		;ldir

WR_W3:	ex	(sp),hl
	ld	a,(de)		;CRC (dummy)
	ex	(sp),hl
	ld	a,(de)		;CRC (dummy)
	
	ld	a,(de)		;dummy
	ld	a,(de)		;receive data response

	and	01fh
	cp	005h
	jr	nz,WR_E0	;response error

WR_W1:	ld	a,(de)		;busy state
	cp	0ffh
	jr	nz,WR_W1

	pop	bc
	pop	de
	inc	de		;CDE = sector number
	ld	a,d
	or	e
	jr	nz,WR_W4
	inc	c
WR_W4:	djnz	WR_R1

	xor	a		;A=00 Cy=0 正常終了
	ret



	
;********************************
;フラッシュＲＯＭ書き込み

FL_TSF:	push	hl		;hlに設定したアドレスに
	ld	a,1fh		;対応するバンクに切り替え
	and	h
	ld	(6000h),a
	ld	l,0
	ld	de,(0f34dh)	;セクタバッファにデータ転送
	ld	bc,0100h
	ldir
	pop	hl		;hlは保存される
	ret

FL_BUF:				;flashルーチンをバッファに転送
	ld	hl,(0f34dh)
	ld	de,0100h
	add	hl,de
	push	hl
	pop	ix		;ix=sector buffer + 100h
	ld	de,FLASH
	ld	bc,FLASH6-FLASH
	ex	de,hl
	ldir
	ret
	
FL_ERS:	ld	de,(0f34dh)
	ex	de,hl
	ld	bc,0100h
	push	ix
	exx			;裏にする
	pop	hl
	ld	de,FLASH3-FLASH
	add	hl,de		;消去＆書き換えのアドレス
	jp	0f399h		;jp   (hl)
	
	

FL_BF2:				;1バイト書き換えルーチンの転送
	ld	hl,(0f34dh)
	ld	de,0100h
	add	hl,de
	push	hl
	pop	ix
	ld	de,FLASH
	ld	bc,FLASH_-FLASH
	ex	de,hl
	ldir
	ret
		
FL_BYT:	push	ix
	pop	hl
	ld	bc,FLASH5-FLASH
	add	hl,bc		;消去＆書き換えのアドレス
	jp	0f399h		;jp   (hl)
	
;*************************
FLASH:	
	push	bc		;ixはここ
	ld	c,a
FLASH1:	ld	a,(de)
	xor	c
	jr	z,FLASH2
	xor	c
	and	20h
	jr	z,FLASH1
	ld	a,(de)
	xor	c
	jr	z,FLASH2
	scf
FLASH2:	pop	bc
	ret
	
FLASH3:	di			;flash消去
	ld	a,0aah
	ld	(4555h),a
	ld	a,055h
	ld	(42aah),a
	ld	a,80h
	ld	(4555h),a
	ld	a,0aah
	ld	(4555h),a
	ld	a,55h
	ld	(42aah),a
	ld	a,30h
	ld	(4001h),a
	ld	a,0ffh
	ld	de,4000h
FLAS_a:	call	0f398h		;jp  (ix)
	ret	c
	
FLASH4:	exx			;表にする

FLASH5:	di
	ld	a,0aah
	ld	(4555h),a
	ld	a,055h
	ld	(42aah),a
	ld	a,0a0h
	ld	(4555h),a
FLASH_:	ld	a,(hl)
	ld	(de),a

FLAS_b:	call	0f398h		;jp   (ix)　書き込みチェックへ
	ret	c

	inc	hl
	inc	de
	dec	bc
	ld	a,b
	or	c
	jr	nz,FLASH5
	ret
	
FLASH6:	ld	a,(de)
	res	7,a
	ld	(de),a
	call	0f398h		;書き込みチェックへ
	ret
	
FLASH7:	ld	a,(0fb22h)
	ld	(de),a
	call	0f398h
	ret
FLASH8:

;**************************************
;仮想FD　キーテーブル

KTBL:	DB	39h,00h,49h,10h,59h,20h,69h,30h
	DB	79h,40h,0Ah,50h,1Ah,60h,2Ah,70h		;
	DB	3Ah,01h,4Ah,02h,7Ah,62h,6Ah,72h
	DB	19h,03h,5Ah,13h,09h,23h,29h,33h

	db	00,00
	db	'SD_3',00,00
	db	'2007 10/15'
	

	END

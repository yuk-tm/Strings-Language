# Strings-Language

## 概要

**Strings-Language** は「直感的な構文」「日本語の感覚に近いロジック」「シンプルな記述」を目指した実験的小言語です。  
変数、if-else分岐、文字列/数値演算、関数カテゴリ、外部コールなどを最小限の構文で記述できます。

- 文の区切りは `/`
- プログラムの終端は `//`
- if-else文や複数文も `/` 区切り
- 文字列も数値も `'...'` または `"..."` でOK
- 変数はすべて暗黙の型推論

---

## インストール・ビルド

```sh
git clone https://github.com/yuk-tm/Strings-Language.git
cd Strings-Language
gcc main.c lexer.c parser.c interpreter.c -o strings.exe -lm
```

---

## サンプルコード

### 変数定義と出力

```
a = '10' /
b = "abc" /
num write a /
num write b /
//
```

### 四則演算・比較

```
x = '5' /
y = '3' /
z = x + y /
num write z /
z = x * y /
num write z /
z = x > y /
num write z /
//
```

### if-else文

```
b = "abc" /
b == "abc" / ? write "equal" / ! write "not equal" //
//
```

### else省略

```
x = '100' /
x > '10' / ? write "大きい！" //
//
```

### 再代入(re文)

```
v = '5' /
num write v /
re v = '20' /
num write v /
//
```

### 複数文

```
a = '1' / b = '2' / num write a / num write b / //
```

### コメント

```
# コメント行
a = '10' / # ここもコメント
num write a /
//
```

### カテゴリ（関数） & run/call

```
func greet()
    write "Hello Category!" /
end
run greet /
//
call py "print('python code!')" / //
```

---

## 使い方

### スクリプトファイルとして実行

1. `test.str` などに上記サンプルをコピペ
2. 実行
    ```sh
    ./strings.exe test.str
    ```

### インタラクティブREPL

```sh
./strings.exe
```
- 1文ずつ `/` で区切って入力、終了は `//` を入力

---

## 構文仕様

- **文区切り**： `/`
- **プログラム終端**： `//`
- **if-else文**：  
  ```
  条件式 /
  ? then文 /
  ! else文 //
  ```
  else省略も可

- **再代入**： `re 変数 = 式 /`
- **共有変数化**： `sunum 変数 /`
- **カテゴリ定義**：  
  ```
  func name()
    ... /
    ... /
  end
  ```
- **カテゴリ呼出**： `run name /`
- **外部コード呼出**： `call py "print('hi')" /`

---

## トークン・演算子例

| 種別        | 記号        | 説明            |
|------------|------------|-----------------|
| 代入        | =          | 変数代入        |
| 足し算      | +          | 文字列連結/加算 |
| 引き算      | -          | 減算            |
| 掛け算      | * or @     | 乗算            |
| 割り算      | / or \\    | 除算            |
| 剰余        | %          | 剰余            |
| 比較        | >, <, >=, <=, ==, != | 比較（数値・文字列両対応） |
| AND/OR      | & , \|     | 論理積/和       |
| NOT         | ~          | 論理否定        |

---

## エラー時

- 構文エラーや未定義変数はエラーメッセージが表示されます。
- 文区切り・終端は必ず `/`, `//` を守ってください。

---

## ライセンス

MIT License

---
# シンボリックリンクの作り方

## シンボリックリンクとは

そこにフォルダがあるかのように振る舞う便利な奴

![SymbolicLink1](./Images/SymbolicLink1.png)![SymbolicLink2](./Images/SymbolicLink2.png)

例えば、**TestModule**のKiCadプロジェクトを作りたいとき

KiCadプロジェクトの実態を`H-57-Avionics/Projects/KiCad`以下に置く。

こうなる

H-57-Avionics/Projects/KiCad/TestModule/  
　TestModule.kicad_pcb  
　TestModule.kicad_pro  
　TestModule.kicad_sch

シンボリックリンクを使って`TestModule`以下が`H-57-Avionics/Components/TestModule/KiCad`以下にもあるように見せかける。

## 作り方

### 1. 管理者権限でコマンドプロンプトを起動する

### 2. Gitでシンボリックリンクを扱えるように設定する

`git config --global core.symlinks true`

### 3. コマンドを実行

`mklink /d <link> <target>`

`<link>`はシンボリックリンクを置きたい場所（相対または絶対パス）

**KiCad**と**TestModule**の例では、`.\Components\TestModule\KiCad`

`<target>`はリンク先のパス（`<link>`からの相対パス）

**KiCad**と**TestModule**の例では、`..\..\Projects\KiCad\TestModule\`

※注意  
`<target>`は`<link>`**からの絶対パス**で指定すること。Githubで共有した際に他の人が使えなくなるほか、あなたのユーザー名が晒されます。(一敗)

---

全部説明されてた

[シンボリックリンクの使い方 | Zenn](https://zenn.dev/kunosu/articles/f2a459431c3a4dfc48cb)
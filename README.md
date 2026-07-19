# ruff

## 目標
構文を単純化させた関数型言語

## ruff本体のファイルたち
ruff.c - メインのコード。これをコンパイラしたものがruff.exe  
ruff.bnf - ruffの構文を適当なbnf記法でまとめたもの  
bin/ruff.exe - この言語自体の実行ファイル。binを環境変数に入れるとruffで実行できる。「bin/ruff.exe ファイル名」でそのファイルを実行できる  
arena/arena.h - メモリ解放の速度を上げるためのArenaのhファイル  
arena/arena.c - メモリ解放の速度を上げるためのArenaのcファイル  
tokenizer/tokenizer.h - 字句解析器のhファイル  
tokenizer/tokenizer.c - 字句解析器のcファイル  
parser/parser.h - 構文解析器のhファイル  
parser/parser.c - 構文解析器のcファイル  

## 進み具合
字句解析器、構文解析器完成済み
意味解析（実行部分）は未実装
よってruff.cもこれから変わっていく

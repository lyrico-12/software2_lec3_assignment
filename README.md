# software2_lec3_assignment

## 課題１
```c
// 長方形を描く
void draw_rect(Canvas *c, const int x0, const int y0, const int width, const int height) {
    char pen = c->pen;

    // あとで修正する
    for (int w = 0; w < width; w++) {
        if (judge_in_canvas(c, x0 + w, y0)){
            c->canvas[x0 + w][y0] = pen;
        }
        if (judge_in_canvas(c, x0 + w, y0 + height - 1)) {
            c->canvas[x0 + w][y0 + height - 1] = pen;
        }
    }
    for (int h = 0; h < height; h++) {
        if (judge_in_canvas(c, x0, y0 + h)) {
            c->canvas[x0][y0 + h] = pen;
        }
        if (judge_in_canvas(c, x0 + width - 1, y0 + h)) {
            c->canvas[x0 + width - 1][y0 + h] = pen;
        }
    }
}

// 円を描く
void draw_circle(Canvas *c, const int x0, const int y0, const double r) {
    char pen = c->pen;
    const int width = c->width;
    const int height = c->height;
    int dist;

    for (int w = 0; w < width; w++) {
        for (int h = 0; h < height; h++) {
            dist = (w - x0) * (w - x0) + (h - y0) * (h - y0);
            if (dist > r * r - 5.0 && dist < r * r + 5.0 && judge_in_canvas(c, w, h)) {
                c->canvas[w][h] = pen;
            }
        }
    }
}
```

上の二つの関数を追加し、`intepret_command`内で、lineの処理と同じような実装をして描画の処理を作った。`main`関数で、`interpret_command`の返り値のResultがLINE, RECT, CIRCLEの場合は線形リストの末尾にノードを追加した。

## 課題２
```c
// 入力ファイルを読み込む。
void load_input_file(const char *filename, History *his){
    const char *default_input_file = "history.txt";
    if (filename == NULL) {
        filename = default_input_file;
    }
    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL) {
	    fprintf(stderr, "error: cannot open %s.\n", filename);
	    return;
    }
    char buf[his->bufsize];
    while(fgets(buf, his->bufsize, fp) != NULL) {
        his->begin = push_back(his->begin, buf, his->bufsize);// 末尾に追加
    }
    fclose(fp);
}
```
上のように、入力のtxtファイルを読み込んで、線形リストの末尾にノードを逐次追加する関数を作った。また、`interpret_command`関数内で、
```c
    if (strcmp(s, "load") == 0) {
        s = strtok(NULL, " ");
        load_input_file(s, his);
        
        reset_canvas(c);
        if (his->begin == NULL) {
            return LOAD;
        }

        // 再実行
        Command *cur = his->begin;
        while(cur != NULL) {
            interpret_command(cur->str, his, c);
            cur = cur->next;
        }
        return LOAD;
    }
```
このように、ノードを追加した後に再実行することで、loadを実装した。

## 課題３
```c
// ペンの種類を変える
void change_pen(Canvas *c, const char pen) {
    if (pen != ' ') {
        c->pen = pen;
    }
}
```
このような関数で、Canvasの文字設定を変えた。`main`関数内で、`interpret_command`の返り値がCHPENだった場合も末尾にノードを追加する処理を加えた。また、undo時の整合性を担保するために、
```c
    // 最初の履歴にデフォルトのペンを追加しておく。
    char s[] = "chpen *\n";
    his.begin = push_back(his.begin, s, his.bufsize);
```
このようにあらかじめデフォルトのペンを線形リストの先頭に入れた。
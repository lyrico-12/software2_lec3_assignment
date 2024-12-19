// コマンドラインペイントプログラム

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h> // for error catch
#include <assert.h>

// Structure for canvas
typedef struct canvas {
    int width;
    int height;
    char **canvas;
    char pen;
} Canvas;

typedef struct command {
    char *str;
    size_t bufsize;
    struct command *next;
} Command;

typedef struct history{
    Command *begin;
    size_t bufsize;
} History;


// functions for Canvas type
Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_canvas(Canvas *c);
void free_canvas(Canvas *c);

// display functions
void rewind_screen(unsigned int line);
void clear_command(void);
void clear_screen(void);

// enum for interpret_command results
// interpret_command の結果をより詳細に分割
typedef enum res{ EXIT, LINE, RECT, CIRCLE, LOAD, UNDO, SAVE, UNKNOWN, ERRNONINT, ERRLACKARGS} Result;
// Result 型に応じて出力するメッセージを返す
char *strresult(Result res);

int max(const int a, const int b);
int judge_in_canvas(Canvas *c, const int x0, const int y0);
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1);
void draw_rect(Canvas *c, const int x0, const int y0, const int width, const int height);
void draw_circle(Canvas *c, const int x0, const int y0, const double r);
Result interpret_command(const char *command, History *his, Canvas *c);
// load用のファイルを解釈する
void load_input_file(const char *filename, History *his);
void save_history(const char *filename, History *his);

// 末尾にノードを追加
Command* push_back(Command *begin, const char *str, size_t bufsize);
Command* pop_back(Command *begin);
int linear_list_len(Command *begin);

int main(int argc, char **argv) {
    //for history recording
    const int bufsize = 1000;
    History his = (History){.begin = NULL, .bufsize = bufsize};
    
    // his.commands = (char**)malloc(his.max_history * sizeof(char*));// 5個のコマンド
    // char* tmp = (char*) malloc(his.max_history * his.bufsize * sizeof(char));
    // for (int i = 0 ; i < his.max_history ; i++) {
    //     his.commands[i] = tmp + (i * his.bufsize);
    // }

    int width;
    int height;
    if (argc != 3){
	    fprintf(stderr,"usage: %s <width> <height>\n",argv[0]);
	    return EXIT_FAILURE;
    }
    else {
	    char *e;
	    long w = strtol(argv[1],&e,10);
	    if (*e != '\0'){
	        fprintf(stderr, "%s: irregular character found %s\n", argv[1],e);
	        return EXIT_FAILURE;
	    }
	    long h = strtol(argv[2],&e,10);
	    if (*e != '\0'){
	        fprintf(stderr, "%s: irregular character found %s\n", argv[2],e);
	        return EXIT_FAILURE;
	    }
	    width = (int)w;
	    height = (int)h;    
    }
    char pen = '*';
    
    char buf[his.bufsize];

    Canvas *c = init_canvas(width,height, pen);

    printf("\n"); // required especially for windows env
    
    while (1) {
	    size_t bufsize = his.bufsize;
        print_canvas(c);
        printf("%d> ", linear_list_len(his.begin));
	    if(fgets(buf, bufsize, stdin) == NULL) break;
	
        const Result r = interpret_command(buf, &his,c);

        if (r == EXIT) break;

        // 返ってきた結果に応じてコマンド結果を表示
        clear_command();
        printf("%s\n",strresult(r));
        // LINEの場合はHistory構造体に入れる
        if (r == LINE || r == RECT || r == CIRCLE) {
            his.begin = push_back(his.begin, buf, bufsize);   
        } else if (r == UNDO) {
            his.begin = pop_back(his.begin);
        } 
        rewind_screen(2); // command results
        clear_command(); // command itself
        rewind_screen(height+2); // rewind the screen to command input
	
    }
    
    clear_screen();
    free_canvas(c);
    
    return 0;
}

Canvas *init_canvas(int width,int height, char pen) {
    Canvas *new = (Canvas *)malloc(sizeof(Canvas));
    new->width = width;
    new->height = height;
    new->canvas = (char **)malloc(width * sizeof(char *));
    
    char *tmp = (char *)malloc(width*height*sizeof(char));
    memset(tmp, ' ', width*height*sizeof(char));//tmpのメモリ領域を' 'で初期化
    for (int i = 0 ; i < width ; i++) {
	    new->canvas[i] = tmp + i * height;
    }
    
    new->pen = pen;
    return new;
}

void reset_canvas(Canvas *c) {
    const int width = c->width;
    const int height = c->height;
    memset(c->canvas[0], ' ', width*height*sizeof(char));
}


void print_canvas(Canvas *c) {
    const int height = c->height;
    const int width = c->width;
    char **canvas = c->canvas;
    
    // 上の壁
    printf("+");
    for (int x = 0 ; x < width ; x++) {
        printf("-");
    }
    printf("+\n");
    
    // 外壁と内側
    for (int y = 0 ; y < height ; y++) {
	    printf("|");
	    for (int x = 0 ; x < width; x++){
	        const char c = canvas[x][y];
	        putchar(c);
	    }
	    printf("|\n");
    }
    
    // 下の壁
    printf("+");
    for (int x = 0 ; x < width ; x++){
        printf("-");
    }
    printf("+\n");
    fflush(stdout);
}

void free_canvas(Canvas *c) {
    free(c->canvas[0]); //  for 2-D array free
    free(c->canvas);
    free(c);
}

// line行だけターミナルのカーソルを上に移動する
void rewind_screen(unsigned int line) {
    printf("\e[%dA",line);
}

// 現在の行をクリアする
void clear_command(void) {
    printf("\e[2K");
}
// 画面全体をクリアする
void clear_screen(void) {
    printf("\e[2J");
}


int max(const int a, const int b) {
    return (a > b) ? a : b;
}

// ある点がCanvasの範囲内か調べる
int judge_in_canvas(Canvas *c, const int x, const int y) {
    const int  width = c->width;
    const int height = c->height;
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return 1;
    } else {
        return 0;
    }
}
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1) {
    const int width = c->width;
    const int height = c->height;
    char pen = c->pen;
    
    const int n = max(abs(x1 - x0), abs(y1 - y0));
    if (judge_in_canvas(c, x0, y0)) {
        c->canvas[x0][y0] = pen;
    }
	
    for (int i = 1; i <= n; i++) {
	    const int x = x0 + i * (x1 - x0) / n;// 二点間に線を引く
	    const int y = y0 + i * (y1 - y0) / n;
	    if (judge_in_canvas(c, x, y))
	        c->canvas[x][y] = pen;
    }
}

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

// ファイルに履歴を出力
void save_history(const char *filename, History *his) {
    const char *default_history_file = "history.txt";
    if (filename == NULL) {
        filename = default_history_file;
    }
    
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL) {
	    fprintf(stderr, "error: cannot open %s.\n", filename);
	    return;
    }
    Command *cur = his->begin;
    while (cur != NULL) {
	    fprintf(fp, "%s", cur->str);
        cur = cur->next;
    }
    
    fclose(fp);
}

Command* push_back(Command *begin, const char *str, size_t bufsize) {
    if (begin == NULL) {
        Command *p = (Command *)malloc(sizeof(Command));
        char *s = (char *)malloc(strlen(str) + 1);
        strcpy(s, str);
        *p = (Command){.str = s, .bufsize = bufsize, .next = NULL};
        return p;   
    } 

    Command *p = begin;
    while (p->next != NULL) {
        p = p->next;
    }
    Command *q = (Command*) malloc(sizeof(Command));
    char *s = (char *)malloc(strlen(str) + 1);
    strcpy(s, str);
    *q = (Command){.str = s, .bufsize = bufsize, .next = NULL};
    p->next = q;

    return begin;
}

Command*pop_back(Command *begin) {
    if (begin == NULL) {
        return NULL;
    }
    if (begin->next == NULL) {
        free(begin->str);
        free(begin);
        return NULL;
    }

    Command *p = begin;
    while(p->next->next != NULL) {
        p = p->next;
    }

    free(p->next->str);
    free(p->next);
    p->next = NULL;
    return begin;
}

int linear_list_len(Command *begin) {
    int num = 0;
    Command *cur = begin;
    while (cur != NULL) {
        num++;
        cur = cur->next;
    }
    return num;
}

Result interpret_command(const char *command, History *his, Canvas *c) {
    char buf[his->bufsize];
    strcpy(buf, command);
    // 最後の改行をヌル文字にする
    buf[strlen(buf) - 1] = 0; // remove the newline character at the end
    
    const char *s = strtok(buf, " ");
    if (s == NULL) { // 改行だけ入力された場合
	    return UNKNOWN;
    }
    // The first token corresponds to command
    if (strcmp(s, "line") == 0) {
	    int p[4] = {0}; // p[0]: x0, p[1]: y0, p[2]: x1, p[3]: x1 
	    char *b[4];
	    for (int i = 0 ; i < 4; i++){
	        b[i] = strtok(NULL, " ");// 第一引数は2回目以降の呼び出しではNULLでよい
	        if (b[i] == NULL){
		        return ERRLACKARGS;
	        }
	    }
	    for (int i = 0 ; i < 4 ; i++){
	        char *e;// *endptr
	        long v = strtol(b[i],&e, 10);//文字を整数にへんかん
	        if (*e != '\0'){
		        return ERRNONINT;
	        }
	        p[i] = (int)v;
	    }
	
	    draw_line(c,p[0],p[1],p[2],p[3]);
	    return LINE;
    }

    if (strcmp(s, "rect") == 0) {
	    int p[4] = {0}; // p[0]: x0, p[1]: y0, p[2]: width, p[3]: height 
	    char *b[4];
	    for (int i = 0 ; i < 4; i++){
	        b[i] = strtok(NULL, " ");// 第一引数は2回目以降の呼び出しではNULLでよい
	        if (b[i] == NULL){
		        return ERRLACKARGS;
	        }
	    }
	    for (int i = 0 ; i < 4 ; i++){
	        char *e;// *endptr
	        long v = strtol(b[i],&e, 10);//文字を整数にへんかん
	        if (*e != '\0'){
		        return ERRNONINT;
	        }
	        p[i] = (int)v;
	    }
	
	    draw_rect(c,p[0],p[1],p[2],p[3]);
        return RECT;
    }

    if (strcmp(s, "circle") == 0) {
        double p[3] = {0.0};
        char *b[3];
	    for (int i = 0 ; i < 3; i++){
	        b[i] = strtok(NULL, " ");// 第一引数は2回目以降の呼び出しではNULLでよい
	        if (b[i] == NULL){
		        return ERRLACKARGS;
	        }
	    } 
	    for (int i = 0 ; i < 3 ; i++){
	        char *e;// *endptr
	        long v = strtol(b[i],&e, 10);//文字を整数にへんかん
	        if (*e != '\0'){
		        return ERRNONINT;
	        }
	        p[i] = (double)v;
	    }
        draw_circle(c, (int)p[0], (int)p[1], p[2]);
        return CIRCLE;       
    }

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
    
    if (strcmp(s, "save") == 0) {
	    s = strtok(NULL, " ");// filename
	    save_history(s, his);
	    return SAVE;
    }
    
    if (strcmp(s, "undo") == 0) {
	    reset_canvas(c);
        if (his->begin == NULL) {
            return UNDO;
        }

        // 再実行
        Command *cur = his->begin;
        while (cur->next != NULL) {
            interpret_command(cur->str, his, c);
            cur = cur->next;
        }

	    return UNDO;
    }
    
    if (strcmp(s, "quit") == 0) {
	    return EXIT;
    }
    
    return UNKNOWN;
}



char *strresult(Result res) {
    switch(res) {
        case EXIT:
	        break;
        case SAVE:
	        return "history saved";
        case LINE:
	        return "1 line drawn";
        case RECT:
            return "1 rect drawn";
        case CIRCLE:
            return "1 circle drawn";
        case LOAD:
            return "history loaded";
        case UNDO:
	        return "undo!";
        case UNKNOWN:
	        return "error: unknown command";
        case ERRNONINT:
	        return "Non-int value is included";
        case ERRLACKARGS:
	        return "Too few arguments";
    }
    return NULL;
}
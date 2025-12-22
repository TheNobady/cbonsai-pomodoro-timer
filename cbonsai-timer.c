#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>

// --- Configuration ---
#define MAX_STEPS 30000 
#define COLOR_LEAF 1
#define COLOR_WOOD 2
#define COLOR_BASE 3
#define COLOR_TEXT 4

typedef struct {
    int x, y;
    char ch[64]; 
    int color;
    int bold;
} DrawStep;

DrawStep *history;
int history_count = 0;
int work_mins = 25;
int break_mins = 5;

// --- Helper: Save to Memory ---
void record_step(int y, int x, const char *s, int color, int bold) {
    if (history_count >= MAX_STEPS) return;
    if (y < 0 || x < 0) return; 
    
    history[history_count].x = x;
    history[history_count].y = y;
    history[history_count].color = color;
    history[history_count].bold = bold;
    
    strncpy(history[history_count].ch, s, 63);
    history[history_count].ch[63] = '\0';
    
    history_count++;
}

// --- 1. The Base ---
void generate_base(int rows, int cols) {
    int base_width = 31; 
    int start_x = (cols / 2) - (base_width / 2);
    int y = rows - 4; // Base sits 4 rows from the reference line

    // Row 1: The Rim & Dirt
    record_step(y, start_x, ":", COLOR_TEXT, 1);
    record_step(y, start_x + 1, "___________", COLOR_LEAF, 0);
    record_step(y, start_x + 12, "./~~~\\.", COLOR_WOOD, 1);
    record_step(y, start_x + 19, "___________", COLOR_LEAF, 0);
    record_step(y, start_x + 30, ":", COLOR_TEXT, 1);

    // Row 2: Bowl Upper
    record_step(y + 1, start_x, " \\", COLOR_TEXT, 0);
    record_step(y + 1, start_x + 29, "/ ", COLOR_TEXT, 0);

    // Row 3: Bowl Lower
    record_step(y + 2, start_x, "  \\_________________________/", COLOR_TEXT, 0);

    // Row 4: Feet
    record_step(y + 3, start_x, "  (_)", COLOR_TEXT, 0);
    record_step(y + 3, start_x + 26, "(_)", COLOR_TEXT, 0);
}

// --- 2. Tree Growth Logic ---
void generate_tree(int y, int x, int life, int multiplier) {
    if (life <= 0) return;

    int dx = (rand() % 3) - 1;
    int dy = (rand() % 10 > 3) ? -1 : 0; 

    char *s = malloc(4);
    if (dy == 0) strcpy(s, "/~");
    else if (dx < 0) strcpy(s, "\\|");
    else if (dx == 0) strcpy(s, "/|\\");
    else strcpy(s, "|/");

    int color = COLOR_WOOD;
    int bold = (rand() % 2);

    if (life < 4) {
        strcpy(s, "&"); 
        color = COLOR_LEAF;
        bold = 1;
    }

    record_step(y, x, s, color, bold);
    free(s);

    if (rand() % 10 > 7 && life > 5) { 
        generate_tree(y + dy, x + dx, life - 1, multiplier);
    }
    generate_tree(y + dy, x + dx, life - 1, multiplier);
}

// --- 3. Perfectly Centered Timer ---
void draw_status(int rows, int cols, int remaining_sec, const char* state) {
    int mins = remaining_sec / 60;
    int secs = remaining_sec % 60;
    
    // Create a temporary buffer to format the string
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[ %s: %02d:%02d ]", state, mins, secs);
    
    // Calculate exact center position
    int len = strlen(buffer);
    int start_x = (cols / 2) - (len / 2);
    int timer_y = rows - 1;

    // Clear the line to avoid artifacts
    move(timer_y, 0);
    clrtoeol();
    
    // Print centered
    attron(COLOR_PAIR(COLOR_TEXT) | A_BOLD);
    mvprintw(timer_y, start_x, "%s", buffer);
    attroff(COLOR_PAIR(COLOR_TEXT) | A_BOLD);
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    work_mins = 25;
    break_mins = 5;

    int opt;
    while ((opt = getopt(argc, argv, "w:b:")) != -1) {
        switch (opt) {
            case 'w': work_mins = atoi(optarg); break;
            case 'b': break_mins = atoi(optarg); break;
        }
    }

    initscr();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    start_color();
    use_default_colors();

    init_pair(COLOR_LEAF, COLOR_GREEN, -1);
    init_pair(COLOR_WOOD, COLOR_YELLOW, -1);
    init_pair(COLOR_BASE, COLOR_WHITE, -1);
    init_pair(COLOR_TEXT, COLOR_CYAN, -1);

    history = malloc(sizeof(DrawStep) * MAX_STEPS);
    srand(time(NULL));

    int rows, cols;
    
    while (1) {
        getmaxyx(stdscr, rows, cols);

        // --- PRE-CALCULATE ---
        history_count = 0;
        
        // 1. Generate Base (Pass rows-1 so feet sit above timer)
        generate_base(rows - 1, cols); 
        int base_end_idx = history_count;

        // 2. Generate Tree (Start 6 rows from bottom reference)
        generate_tree(rows - 6, cols / 2, 35, 5);

        // --- PHASE 1: WORK ---
        time_t start = time(NULL);
        int total_sec = work_mins * 60;

        while (1) {
            int elapsed = (int)difftime(time(NULL), start);
            int remaining = total_sec - elapsed;
            if (remaining < 0) break;

            erase();

            // Draw Base
            for(int i = 0; i < base_end_idx; i++) {
                if(history[i].bold) attron(A_BOLD);
                attron(COLOR_PAIR(history[i].color));
                mvprintw(history[i].y, history[i].x, "%s", history[i].ch);
                attroff(A_BOLD);
            }

            // Draw Tree
            int tree_total = history_count - base_end_idx;
            float percent = (float)elapsed / total_sec;
            if (percent > 1.0) percent = 1.0;
            
            int draw_limit = base_end_idx + (int)(tree_total * percent);
            
            for(int i = base_end_idx; i < draw_limit; i++) {
                if(history[i].bold) attron(A_BOLD);
                attron(COLOR_PAIR(history[i].color));
                mvprintw(history[i].y, history[i].x, "%s", history[i].ch);
                attroff(A_BOLD);
            }

            draw_status(rows, cols, remaining, "WORK");
            refresh();
            
            if (getch() == 'q') goto end;
            napms(100);
        }

        flash(); 

        // --- PHASE 2: BREAK ---
        start = time(NULL);
        total_sec = break_mins * 60;

        while (1) {
            int elapsed = (int)difftime(time(NULL), start);
            int remaining = total_sec - elapsed;
            if (remaining < 0) break;

            erase();

            // Draw Base
            for(int i = 0; i < base_end_idx; i++) {
                if(history[i].bold) attron(A_BOLD);
                attron(COLOR_PAIR(history[i].color));
                mvprintw(history[i].y, history[i].x, "%s", history[i].ch);
                attroff(A_BOLD);
            }

            // Draw Tree (Reverse)
            int tree_total = history_count - base_end_idx;
            float percent = (float)elapsed / total_sec;
            
            int draw_limit = base_end_idx + (int)(tree_total * (1.0 - percent));
            if (draw_limit < base_end_idx) draw_limit = base_end_idx;

            for(int i = base_end_idx; i < draw_limit; i++) {
                if(history[i].bold) attron(A_BOLD);
                attron(COLOR_PAIR(history[i].color));
                mvprintw(history[i].y, history[i].x, "%s", history[i].ch);
                attroff(A_BOLD);
            }

            draw_status(rows, cols, remaining, "BREAK");
            refresh();

            if (getch() == 'q') goto end;
            napms(100);
        }
        flash();
    }

end:
    free(history);
    endwin();
    return 0;
}

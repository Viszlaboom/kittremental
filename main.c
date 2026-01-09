#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  static void sleep_ms(int ms){ Sleep(ms); }
#else
  #include <unistd.h>
  static void sleep_ms(int ms){ usleep(ms * 1000); }
#endif

typedef struct {
    double yarn;
    int kittens;
    int bowls;
    double kitten_base_rate; // yarn per second per kitten
} Game;

static double kitten_cost(const Game *g) {
    // escalating cost: 10, 12, 15, 18...
    return 10.0 * (1.0 + 0.2 * g->kittens);
}

static double bowl_cost(const Game *g) {
    // escalating cost: 25, 35, 45...
    return 25.0 + 10.0 * g->bowls;
}

static double kitten_rate(const Game *g) {
    // bowls improve kitten output by +25% each
    double mult = 1.0 + 0.25 * g->bowls;
    return g->kitten_base_rate * mult;
}

static void print_ui(const Game *g) {
    printf("\n=== Kitten Idle (MVP) ===\n");
    printf("Yarn: %.2f\n", g->yarn);
    printf("Kittens: %d | Bowls: %d\n", g->kittens, g->bowls);
    printf("Passive rate: %.2f yarn/s\n", g->kittens * kitten_rate(g));
    printf("\nCommands:\n");
    printf("  g = gather yarn (+1)\n");
    printf("  b = buy kitten (cost %.2f)\n", kitten_cost(g));
    printf("  u = buy bowl upgrade (cost %.2f)\n", bowl_cost(g));
    printf("  s = save | l = load | q = quit\n");
    printf("> ");
    fflush(stdout);
}

static bool save_game(const Game *g, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    if (fwrite(g, sizeof(Game), 1, f) != 1) { fclose(f); return false; }
    fclose(f);
    return true;
}

static bool load_game(Game *g, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    Game tmp;
    if (fread(&tmp, sizeof(Game), 1, f) != 1) { fclose(f); return false; }
    fclose(f);
    *g = tmp;
    return true;
}

static int read_command_nonblocking(void) {
    // Minimal cross-platform approach: fall back to blocking input.
    // For MVP simplicity, we use blocking fgets in main loop below.
    // This function is unused, kept as a placeholder for future real-time input.
    return -1;
}

int main(void) {
    Game g = {
        .yarn = 0.0,
        .kittens = 0,
        .bowls = 0,
        .kitten_base_rate = 0.5
    };

    const char *save_path = "save.dat";

    // Tick settings
    const int tick_ms = 100;               // 0.1s
    const double dt = tick_ms / 1000.0;    // seconds per tick

    bool running = true;

    // For a simple MVP, we alternate:
    // - show UI
    // - read one command (blocking)
    // - simulate some passive time (a few ticks) so it feels alive
    //
    // Later, you can replace this with non-blocking input and real continuous ticks.
    while (running) {
        print_ui(&g);

        char buf[64];
        if (!fgets(buf, sizeof(buf), stdin)) break;

        char cmd = buf[0];

        switch (cmd) {
            case 'g':
            case 'G':
                g.yarn += 1.0;
                printf("You gathered yarn. +1\n");
                break;

            case 'b':
            case 'B': {
                double cost = kitten_cost(&g);
                if (g.yarn >= cost) {
                    g.yarn -= cost;
                    g.kittens += 1;
                    printf("A kitten joins! Kittens: %d\n", g.kittens);
                } else {
                    printf("Not enough yarn.\n");
                }
                break;
            }

            case 'u':
            case 'U': {
                double cost = bowl_cost(&g);
                if (g.yarn >= cost) {
                    g.yarn -= cost;
                    g.bowls += 1;
                    printf("Bought a food bowl. Bowls: %d\n", g.bowls);
                } else {
                    printf("Not enough yarn.\n");
                }
                break;
            }

            case 's':
            case 'S':
                if (save_game(&g, save_path)) printf("Saved to %s\n", save_path);
                else printf("Save failed.\n");
                break;

            case 'l':
            case 'L':
                if (load_game(&g, save_path)) printf("Loaded from %s\n", save_path);
                else printf("Load failed (no save yet?).\n");
                break;

            case 'q':
            case 'Q':
                running = false;
                continue;

            default:
                printf("Unknown command.\n");
                break;
        }

        // Simulate passive income for a short moment after each command,
        // so the game "moves" even with blocking input.
        // (Replace with true real-time loop later.)
        for (int i = 0; i < 5; i++) { // 0.5s total
            double rate = g.kittens * kitten_rate(&g);
            g.yarn += rate * dt;
            sleep_ms(tick_ms);
        }
    }

    printf("\nGoodbye.\n");
    return 0;
}


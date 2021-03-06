#include "eruutil/debug.h"
#include <boost/thread.hpp>
#include <SDL/SDL.h>
#include <limits.h>
#include <time.h>

unsigned
    SCREEN_WIDTH = 1200,
    SCREEN_HEIGHT = 800,
    SCREEN_BPP = 32,
    GRID_WIDTH = 2,
    GRID_HEIGHT = 2,
    UPDATES_PER_FRAME = 1,
    FRAME_RATE = 30,
    NR_PROCESSORS = boost::thread::hardware_concurrency();

const char WINDOW_CAPTION[] = "Conway SDL";

SDL_Surface *screen = NULL;

int fullscreen = 0,
    quit = 0,
    generation = 0;

struct cell_t
{
    char state;
    Uint8 r, g, b;
};

cell_t *world = NULL;
cell_t *newWorld, *oldWorld, *tmpWorld;

void cleanup()
{
    if (screen) SDL_FreeSurface(screen);
    if (newWorld) free(newWorld);
    if (oldWorld) free(oldWorld);
    SDL_Quit();
}

void init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
        exit(EXIT_FAILURE);
    atexit(cleanup);
    screen = SDL_SetVideoMode(
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SCREEN_BPP,
            SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) exit(EXIT_FAILURE);
    SDL_WM_SetCaption(WINDOW_CAPTION, NULL);
    int width = SCREEN_WIDTH / GRID_WIDTH;
    int height = SCREEN_HEIGHT / GRID_HEIGHT;
    newWorld = new cell_t[width * height];
    oldWorld = new cell_t[width * height];
    tmpWorld = new cell_t[width * height];
    if (!newWorld || !oldWorld) exit(EXIT_FAILURE);
    world = newWorld;
    /*
      cell_t cell = {1, 0xFF, 0, 0};
      world[0] = cell;
      world[(height - 1) * width] = cell;
      world[(height - 1) * width + width - 1] = cell;
      world[width - 1] = cell;
    */
    // random start
    srand(time(NULL));
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (rand() < RAND_MAX / 4)
            {
                cell_t cell = {
                    1,
                    //.r = (x * 0xFF) / width,
                    x >= width / 2 ? 0xFF : 0,
                   0,
                    //.b = (y * 0xFF) / height};
                    x < width / 2 ? 0xFF : 0};
                world[y * width + x] = cell;
            }

        }
    }
}

void events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
            quit = 1;
        } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
            fullscreen = !fullscreen;
            screen = SDL_SetVideoMode(
                                      SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
                                      SDL_HWSURFACE | (fullscreen ? SDL_FULLSCREEN : 0));
        }
    }
}

void *drawWorld(void *arg)
{
    cell_t *world = (cell_t *)arg;
    SDL_FillRect(screen, &screen->clip_rect,
                 //SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
                 SDL_MapRGB(screen->format, 0, 0, 0));
    int width = SCREEN_WIDTH / GRID_WIDTH;
    int height = SCREEN_HEIGHT / GRID_HEIGHT;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (world[y * width + x].state) {
                SDL_Rect rect =
                    {x * GRID_WIDTH, y * GRID_HEIGHT,
                     GRID_WIDTH, GRID_HEIGHT};
                SDL_FillRect(screen, &rect, SDL_MapRGB(
                                                       screen->format, world[y * width + x].r, world[y * width + x].g, world[y * width + x].b));
            }
        }
    }
    if (SDL_Flip(screen)) exit(EXIT_FAILURE);
    return NULL;
}

void draw()
{
    //if (drawThread) pthread_join(drawThread, NULL);
    //pthread_create(&drawThread, NULL, drawWorld, (void *)world);
    drawWorld(world);
}

int updateWorld(void *arg)
{ //cell_t *oldWorld, cell_t *newWorld, int firstRow, int lastRow
    int width = SCREEN_WIDTH / GRID_WIDTH;
    int height = SCREEN_HEIGHT / GRID_HEIGHT;
    long firstRow = (long)arg;
    int lastRow = firstRow + height / NR_PROCESSORS;
    for (int y = firstRow; y < lastRow; y++) {
        for (int x = 0; x < width; x++) {
            int adj = 0, r = 0, g = 0, b = 0;
            for (int j = y - 1; j <= y + 1; j++) {
                //if (j < 0 || j >= height) continue;
                for (int i = x - 1; i <= x + 1; i++) {
                    if (i == x && j == y) continue;
                    int n = j % height; if (n < 0) n += height;
                    int m = i % width; if (m < 0) m += width;
                    //if (i < 0 || i >= width) continue;
                    cell_t *cell = &oldWorld[n * width + m];
                    if (cell->state) {
                        //printf("(%d, %d) has neighbour (%d, %d)\n",
                        //  x, y, m, n);
                        adj++;
                        r += cell->r;
                        g += cell->g;
                        b += cell->b;
                    }
                }
            }
            cell_t cell;
                cell.r = UCHAR_MAX;
                cell.g = UCHAR_MAX;
                cell.b = UCHAR_MAX;
            if (adj < 2 || adj > 3) {
                cell.state = 0;
                cell.g = 0;
            } else if (adj == 2)
                cell = oldWorld[y * width + x];
            else if (adj == 3) {
                cell.state = 1;
                cell.r = r / 3;
                cell.b = b / 3;
                cell.g = 0xFF - cell.r - cell.b;
            }
            newWorld[y * width + x] = cell;
        }
    }
    return NULL;
}

void update()
{
    // swap world buffers
    world = oldWorld;
    oldWorld = newWorld;
    newWorld = world;

    SDL_Thread **threads = new SDL_Thread *[NR_PROCESSORS];
    // create update threads
    for (long t = 0; t < NR_PROCESSORS; t++)
    {
        threads[t] = SDL_CreateThread(
                updateWorld,
                reinterpret_cast<void *>(t * (SCREEN_HEIGHT / GRID_HEIGHT) / NR_PROCESSORS));
    }
    // join update threads
    for (long t = 0; t < NR_PROCESSORS; t++)
    {
        SDL_WaitThread(threads[t], NULL);
    }
    world = newWorld;
    generation++;
        delete[] threads;
}

void loop()
{
    int fpsStartFrame = generation;
    Uint32 fpsStartTicks = SDL_GetTicks();
    while (1) {
        draw();
        Uint32 fpsCurTicks = SDL_GetTicks();
        Uint32 waitDelay = 1000 / FRAME_RATE * (generation - fpsStartFrame + 1) - fpsCurTicks + fpsStartTicks;
        if (waitDelay > 1000 / FRAME_RATE) printf("%d\n", waitDelay);
        if (waitDelay > 1000 / FRAME_RATE) waitDelay = 1000 / FRAME_RATE;
        if (waitDelay > 0) SDL_Delay(waitDelay);
        if (fpsCurTicks - fpsStartTicks >= 1000)
                {
            printf("%d ticks have passed\n", fpsCurTicks - fpsStartTicks);
            printf("%d frames have passed\n", generation - fpsStartFrame);
            char caption[32];
            sprintf(caption, "%s - %.1f fps", WINDOW_CAPTION, ((float)(generation - fpsStartFrame)) / (((float)(fpsCurTicks - fpsStartTicks)) / 1000.f));
            fpsStartFrame = generation;
            fpsStartTicks = fpsCurTicks;
            SDL_WM_SetCaption(caption, NULL);
        }
        events();
        if (quit) break;
        for (int f = 0; f < UPDATES_PER_FRAME; f++)
            update();
    }
}

void check_variables()
{
    // these need to be multiples
    assert(SCREEN_HEIGHT % GRID_HEIGHT == 0);
    assert(SCREEN_WIDTH % GRID_WIDTH == 0);
    // work must be evenly divisible
    assert((SCREEN_HEIGHT / GRID_HEIGHT) % NR_PROCESSORS == 0);
    debug("%u", NR_PROCESSORS);
}

int main(int, char **)
{
        //((void(*)())0)();
    init();
    loop();
    return EXIT_SUCCESS;
}

#include <stdio.h>

// Let's use a custom malloc, because why not?

#define Heap 1024 * 1024 * 1024
#define Align   8

typedef struct Block {
        unsigned long size;
        unsigned long free;
        struct Block* next;
} Block;

static char heap[Heap];

static Block* FreeList = (Block*)heap;

static unsigned int align(unsigned int size)
{
        unsigned int rem = size % Align;
        return rem ? size + (Align - rem) : size;
}

static void InitHeap( void )
{
        FreeList->size = Heap - sizeof(Block);
        FreeList->free = 1;
        FreeList->next = 0;
}


void *Malloc(unsigned int size)
{
        if (!FreeList->size) InitHeap();

        size = align(size);
        Block *current = FreeList;

        while (current)
        {
                if (current-> free && current->size >= size)
                {
                        // split if big enough
                        if (current->size >= size + sizeof(Block) + Align)
                        {
                                Block *Newblock = (Block*) ((char*)current + sizeof(Block) + size);
                                Newblock->size = current->size - size - sizeof(Block);
                                Newblock->free = 1;
                                Newblock->next = current->next;
                                current->next = Newblock;
                                current->size = size;
                        }
                        current->free = 0;
                        return (char*)current + sizeof(Block);
                }
                current = current->next;
        }
        return 0; // out of memory

}

void Free(void* ptr)
{
        if (!ptr) return; // Can't let the program nuke itself -- it's a game engine after all

        Block *block = (Block*)((char*)ptr - sizeof(Block));
        block->free = 1;

        if (block->next && block->next->free)
        {
                block->size += sizeof(Block) + block->next->size;
                block->next = block->next->next;
        }

}


struct Map
{
        float Width;
        float Height;
        char toDrawWith; // What character we should use to render the board with.
};



struct Map *CreateMap(char tdw)
{
        // Allocate memory for the map
        struct Map *MapTR = (struct Map*) Malloc(sizeof(struct Map));
        if (!MapTR) return 0; // fail gracefully

        MapTR->Width = 80;
        MapTR->Height = 24;
        MapTR->toDrawWith = tdw;

        return MapTR;
}



// Now, it's a good idea for us to put a player on top of this large mass of hashtags so we get a 2D interface.

typedef struct Player
{
        float x;
        float y;

        float Height;
        float Width;

        char Sprite; // What the player will be rendered with.
        // That's all we need for 2D...

} Player;


Player *CreatePlayer(struct Map *map)
{
        Player *playerTR = (Player*) Malloc(sizeof(Player));
        if (!playerTR) return 0;

        playerTR->Width = 2;
        playerTR->Height = 5;

        playerTR->x = (map->Width / 2) - 1;

        // I guessed this calculation
        playerTR->y = map->Height - 3 - playerTR->Height;

        playerTR->Sprite = ';';

        return playerTR;
}






void DrawMapWithPlayer(struct Map *map, Player *player)
{
        for (int h = 0; h < map->Height; h++)
        {
                for (int w = 0; w < map->Width; w++)
                {

                        if (h >= map->Height - 3)
                        {
                                printf("%c", map->toDrawWith);
                        }

                        else if (h >= player->y && h < player->y + player->Height &&
                                w >= player->x && w < player->x + player->Width)
                        {
                                printf("%c", player->Sprite);
                        }
                        else
                        {
                                printf(" "); // empty space above floor
                        }
                }
                printf("\n");
        }
}


#include <termios.h>
#include <unistd.h>

// Change terminal settings to read input without Enter
void enableRawMode()
{
        struct termios t;
        tcgetattr(STDIN_FILENO, &t);
        t.c_lflag &= ~(ICANON | ECHO); // disable line buffering and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Restore normal terminal settings
void disableRawMode()
{
        struct termios t;
        tcgetattr(STDIN_FILENO, &t);
        t.c_lflag |= (ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
}



int main(void)
{
        struct Map *map = CreateMap('#');
        Player *player = CreatePlayer(map);

        if (!map || !player) return 1;

        for (int i = 0; i < player->Height; i++)
        {
                printf("\n");
        }

        DrawMapWithPlayer(map, player);

        enableRawMode();


        char c;

        while (1)
        {
                if (read(STDIN_FILENO, &c, 1) == 1)
                {
                        if (c == 'w')
                        {
                                player->x++;
                        }

                        else if (c == 's')
                        {
                                player->x--;
                        }

                        else if (c == 'a')
                        {
                                player->x--;
                        }

                        else if (c == 'd')
                        {
                                player->x++;
                        }

                        else if (c == '\t')
                        {
                                int jumpHeight = 3;
                                for (int i = 0; i < jumpHeight; i++) {
                                        player->y--;
                                        printf("\033[H\033[J"); // clear screen
                                        DrawMapWithPlayer(map, player);
                                        usleep(100000); // 0.1s
                                }
                                        for (int i = 0; i < jumpHeight; i++) {
                                        player->y++;
                                        printf("\033[H\033[J");
                                        DrawMapWithPlayer(map, player);
                                        usleep(100000);
                                }

                        }
                }
                printf("\033[H\033[J"); // linux ansi escape code
                DrawMapWithPlayer(map, player);
        }
}


// This took <1h30 minutes, even with rewriting and debugging!

#define fbuf ((volatile unsigned short*)0xB8000)
static unsigned long cursorx = 0;
static unsigned long cursory = 0;
typedef unsigned long size;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

void printc(char c)
{
        if (c == '\n')
        {
                cursorx = 0;
                if (++cursory >= 25)
                {
                        cursory = 24;
                }
                return;
        }

        fbuf[cursory * 80 + cursorx] = (unsigned short)(0x0F << 8 | c);

        if (++cursorx >= 80)
        {
                cursorx = 0;
                if(++cursory >= 25)
                {
                        cursory = 24;
                }
        }
}

void printf(const char *str)
{
        while (*str)
        {
                printc(*str++);
        }
}
// Let's use a custom malloc, because why not?

#define Heap 1024 * 1024
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

struct Enemy
{
        float x;
        float y;
};


struct Map *CreateMap(char tdw)
{
        // Allocate memory for the map
        struct Map *MapTR = (struct Map*) Malloc(sizeof(struct Map));
        if (!MapTR) return 0; // fail gracefully

        MapTR->Width = 50;
        MapTR->Height = 15;
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

        float Health;

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

        playerTR->Sprite = 'A';

        return playerTR;
}





static inline u8 inb(u16 port) {
    u8 ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

u8 getscn() {
    while (!(inb(0x64) & 1)) { }
    return inb(0x60);
}

static const char kbmp[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
    0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char getch() {
    u8 scan = getscn();
    if (scan < 128) {
        return kbmp[scan];
    }

    return 0;
}

void read(char* buffer, size bufsize) {
    size pos = 0;
    while (1) {
        char c = getch();
        if (c) {
            if (c == '\b' && pos > 0) {
                pos--;
                cursorx = (cursorx == 0) ? 80 - 1 : cursorx - 1;
                printf(" \b");
            }

            else if (c == '\n') {
                buffer[pos] = '\0';
                printc('\n');
                break;
            }


            else if (pos < bufsize - 1) {
                buffer[pos++] = c;
                printc(c);
            }
        }
    }
}




int cmpn(const char *str1, const char *str2, size n) {
    while (n && *str1 && (*str1 == *str2)) {
        n--;
        str1++;
        str2++;
    }

    if (n == 0) {
        return 0;
    }

    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}
// so below is stuff for easymd, booting, and comparisons.
int is_digit(char c) {
    return c >= '0' && c <= '9';
}

#define MAX_LINES 128

void itoa(int value, char* str) {
    char* p = str;
    char* p1, *p2;
    unsigned int abs = value;
    int len = 0;

    if (value < 0) {
        *p++ = '-';
        str++;
        abs = -value;
    }

    do {
        *p++ = '0' + (abs % 10);
        abs /= 10;
    } while (abs);

    *p = '\0';

    p1 = str;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

void clr() {
    for (size y = 0; y < 25; y++) {
        for (size x = 0; x < 80; x++) {
            fbuf[y * 80 + x] = (u16)' ' | (0x0F << 8);
        }
    }

    cursorx = 0;
    cursory = 0;
}


void DrawMapWithPlayer(struct Map *map, Player *player)
{
    for (int h = 0; h < map->Height; h++)
    {
        for (int w = 0; w < map->Width; w++)
        {
            if (h >= map->Height - 3)
            {
                printc(map->toDrawWith);
            }
            else if (h >= player->y && h < player->y + player->Height &&
                     w >= player->x && w < player->x + player->Width)
            {
                printc(player->Sprite);
            }
            else
            {
                printc(' ');   //ensure empty space is drawn
            }
        }
        printc('\n');
    }
}


int cmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }



    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}


int gamemain(void)
{
        struct Map *map = CreateMap('#');
        Player *player = CreatePlayer(map);

        if (!map || !player) return 1;

        for (int i = 0; i < player->Height; i++)
        {
                printf("\n");
        }

        DrawMapWithPlayer(map, player);


        char cbuf[67];

        while (1)
        {
                char c = getch();

                if (c == 'D') {
                        if (player->x + player->Width < map->Width) {
                                        player->x++;
                                }
                        }
                else if (c == 'A') {
                                if (player->x > 0) {
                                        player->x--;
                                }
                        }

                else if (c == '\t')
                {
                        int jh = 3;
                        for (int i = 0; i < jh; i++)
                        {
                                player->y--;
                                clr();
                                DrawMapWithPlayer(map, player);
                                // crude delay
                                for (int gh = 0; gh < 10000000; gh++);
                        }

                        for (int i = 0; i < jh; i++)
                        {
                                player->y++;
                                clr();
                                DrawMapWithPlayer(map, player);
                                for (int gh = 0; gh < 10000000; gh++);
                        }
                }


                clr();
                DrawMapWithPlayer(map, player);

                char buf[32];
                itoa((int)player->x, buf);
                printf("X:");
                printf(buf);
                printf("\n");


        }
}

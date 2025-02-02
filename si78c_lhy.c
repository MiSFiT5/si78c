//
// Space Invaders 1978 in C
// Jason McSweeney
/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * project for Software Quality Assurance
 * 
 * project address:https://github.com/MiSFiT5/si78c
 * 
 * timeline: from 2022/11 -> 2022/12
 * 
 * comments author: 刘和源 LIU HEYUAN 
 * studenID: 1909853L-I011-0062
 * 
 * Bachelor of Science (Software Engineering)
 * Faculty of Innovation Engineering
 * Macau University of Science and Technology
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdint.h>
#include <assert.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define  OP_BLEND         0
#define  OP_ERASE         1
#define  OP_COLLIDE       2
#define  OP_BLIT          3

#define  SHOT_ACTIVE      0x80
#define  SHOT_BLOWUP      0x1

#define  BEAM_VBLANK      0x80
#define  BEAM_MIDDLE      0x00
#define  XR_MID           0x80

#define  DIP3_SHIPS1      0x1
#define  DIP5_SHIPS2      0x2
#define  DIP6_BONUS       0x8
#define  DIP7_COININFO    0x80

#define  TILT_BIT         0x4
#define  COIN_BIT         0x1

#define  INIT             1
#define  PROMPT           2
#define  SHIELDS          4

#define  ROL_SHOT_PICEND  0xf9
#define  PLU_SHOT_PICEND  0xed
#define  SQU_SHOT_PICEND  0xdb

#define  PLAYER_ADDR            0x2010
#define  PLAYER_SIZE            16
#define  PLAYER_SHOT_ADDR       0x2020
#define  PLAYER_SHOT_DATA_ADDR  0x2025
#define  PLAYER_SHOT_DATA_SIZE  7
#define  ROLLING_SHOT_ADDR      0x2030
#define  ROLLING_SHOT_SIZE      16
#define  PLUNGER_SHOT_ADDR      0x2040
#define  PLUNGER_SHOT_SIZE      16
#define  SQUIGGLY_SHOT_ADDR     0x2050
#define  SQUIGGLY_SHOT_SIZE     16
#define  SAUCER_ADDR            0x2083
#define  SAUCER_SIZE            10
#define  P1_ADDR                0x2100
#define  P2_ADDR                0x2200
#define  SPLASH_DESC_ADDR       0x20c5

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "This code is Little Endian only."
#endif

struct Word
{
    union {

        uint16_t u16;
        struct {
            uint8_t l;
            uint8_t h;
        };

        struct {
            uint8_t y;
            uint8_t x;
        };

    };
} __attribute__ ((packed)); // 4

typedef struct Word Word;

struct SprDesc
{
    Word spr;

    union {
        Word pos;    // pixel position
        Word sc;     // screen address
    };

    uint8_t  n;

} __attribute__ ((packed)); // 5

typedef struct SprDesc SprDesc;

struct GameObjHeader
{
    uint8_t TimerMSB;
    uint8_t TimerLSB;
    uint8_t TimerExtra;
    Word Handler;

} __attribute__ ((packed)); // 5

typedef struct GameObjHeader GameObjHeader;

struct AShot
{
    uint8_t      Status;
    uint8_t      StepCnt;
    uint8_t      Track;
    Word         CFir;
    uint8_t      BlowCnt;
    SprDesc      Desc;

} __attribute__ ((packed)); // 11

typedef struct AShot AShot;

/**
 * si78c 是原版街机游戏像素级的复刻，
 * 甚至大部分的内存数据也是相等的，
 * 为了做到这一点 si78c 模拟了街机的一部分硬件：RAM、ROM 和 显存，
 * 它们在代码中被封装成了一个名为 Mem 的大结构体，
 * 内存分配如下：
 * 
 * 0000-1FFF 8K ROM
 * 2000-23FF 1K RAM
 * 2400-3FFF 7K Video RAM
 * 4000- RAM mirror
 * 
 */


struct Mem {

//这里的0000 到 1FFF 8k的空间分配给 ROM
uint8_t        pad_01[3063];                          //  0x0000
uint8_t        MSG_TAITO_COP[9];                      //  0x0bf7
uint8_t        pad_02[3601];                          //  0x0c00
uint8_t        soundDelayKey[16];                     //  0x1a11
uint8_t        soundDelayValue[16];                   //  0x1a21
uint8_t        pad_03[112];                           //  0x1a31
uint8_t        ShotReloadRate[5];                     //  0x1aa1
uint8_t        MSG_GAME_OVER__PLAYER___[20];          //  0x1aa6
uint8_t        MSG_1_OR_2PLAYERS_BUTTON[20];          //  0x1aba
uint8_t        pad_04;                                //  0x1ace
uint8_t        MSG_ONLY_1PLAYER__BUTTON[20];          //  0x1acf
uint8_t        pad_05;                                //  0x1ae3
uint8_t        MSG__SCORE_1__HI_SCORE_SCORE_2__[28];  //  0x1ae4
uint8_t        pad_06[112];                           //  0x1b00
uint8_t        MSG_PLAY_PLAYER_1_[14];                //  0x1b70
uint8_t        pad_07[66];                            //  0x1b7e
uint8_t        SPLASH_SHOT_OBJDATA[16];               //  0x1bc0
uint8_t        pad_08[144];                           //  0x1bd0
uint8_t        PLAYER_SPRITES[48];                    //  0x1c60
uint8_t        pad_09[9];                             //  0x1c90
uint8_t        MSG__10_POINTS[10];                    //  0x1c99
uint8_t        MSG__SCORE_ADVANCE_TABLE_[21];         //  0x1ca3
uint8_t        AReloadScoreTab[4];                    //  0x1cb8
uint8_t        MSG_TILT[4];                           //  0x1cbc
uint8_t        pad_10[28];                            //  0x1cc0
uint8_t        AlienShotExplodingSprite[6];           //  0x1cdc
uint8_t        pad_11[24];                            //  0x1ce2
uint8_t        MSG_PLAY2[5];                          //  0x1cfa
uint8_t        pad_12[33];                            //  0x1cff
uint8_t        SHIELD_SPRITE[44];                     //  0x1d20
uint8_t        SauScrValueTab[4];                     //  0x1d4c
uint8_t        SauScrStrTab[4];                       //  0x1d50
uint8_t        pad_13[40];                            //  0x1d54
uint8_t        SpriteSaucerExp[24];                   //  0x1d7c
uint8_t        MSG__50[3];                            //  0x1d94
uint8_t        MSG_100[3];                            //  0x1d97
uint8_t        MSG_150[3];                            //  0x1d9a
uint8_t        MSG_300[3];                            //  0x1d9d
uint8_t        AlienScores[3];                        //  0x1da0
uint8_t        AlienStartTable[8];                    //  0x1da3
uint8_t        MSG_PLAY[4];                           //  0x1dab
uint8_t        MSG_SPACE__INVADERS[15];               //  0x1daf
uint8_t        SCORE_ADV_SPRITE_LIST[17];             //  0x1dbe
uint8_t        SCORE_ADV_MSG_LIST[17];                //  0x1dcf
uint8_t        MSG____MYSTERY[10];                    //  0x1de0
uint8_t        MSG__30_POINTS[10];                    //  0x1dea
uint8_t        MSG__20_POINTS[10];                    //  0x1df4
uint8_t        pad_14[338];                           //  0x1dfe
uint8_t        MSG__1_OR_2_PLAYERS___[18];            //  0x1f50
uint8_t        pad_15[46];                            //  0x1f62
uint8_t        MSG_INSERT__COIN[12];                  //  0x1f90
uint8_t        CREDIT_TABLE[4];                       //  0x1f9c
uint8_t        CREDIT_TABLE_COINS[9];                 //  0x1fa0
uint8_t        MSG_CREDIT_[7];                        //  0x1fa9
uint8_t        pad_16[67];                            //  0x1fb0
uint8_t        MSG_PUSH[4];                           //  0x1ff3
uint8_t        pad_17[9];                             //  0x1ff7

// start of ram mirror
// 这里的2000 到 23FF 1K空间分配给 RAM， RAM 镜像的开始
uint8_t        waitOnDraw;                            //  0x2000
uint8_t        pad_18;                                //  0x2001
uint8_t        alienIsExploding;                      //  0x2002
uint8_t        expAlienTimer;                         //  0x2003
uint8_t        alienRow;                              //  0x2004
uint8_t        alienFrame;                            //  0x2005
uint8_t        alienCurIndex;                         //  0x2006
Word           refAlienDelta;                         //  0x2007
Word           refAlienPos;                           //  0x2009
Word           alienPos;                              //  0x200b
uint8_t        rackDirection;                         //  0x200d
uint8_t        rackDownDelta;                         //  0x200e
uint8_t        pad_19;                                //  0x200f
GameObjHeader  playerHeader;                          //  0x2010
uint8_t        playerAlive;                           //  0x2015
uint8_t        expAnimateTimer;                       //  0x2016
uint8_t        expAnimateCnt;                         //  0x2017
SprDesc        playerDesc;                            //  0x2018
uint8_t        nextDemoCmd;                           //  0x201d
uint8_t        hidMessSeq;                            //  0x201e
uint8_t        pad_20;                                //  0x201f
GameObjHeader  plyrShotHeader;                        //  0x2020
uint8_t        plyrShotStatus;                        //  0x2025
uint8_t        blowUpTimer;                           //  0x2026
SprDesc        playerShotDesc;                        //  0x2027
uint8_t        shotDeltaYr;                           //  0x202c
uint8_t        fireBounce;                            //  0x202d
uint8_t        pad_21[2];                             //  0x202e
GameObjHeader  rolShotHeader;                         //  0x2030
AShot          rolShotData;                           //  0x2035
GameObjHeader  pluShotHeader;                         //  0x2040
AShot          pluShotData;                           //  0x2045
GameObjHeader  squShotHeader;                         //  0x2050
AShot          squShotData;                           //  0x2055
uint8_t        pad_22;                                //  0x2060
uint8_t        collision;                             //  0x2061
SprDesc        expAlien;                              //  0x2062
uint8_t        playerDataMSB;                         //  0x2067
uint8_t        playerOK;                              //  0x2068
uint8_t        enableAlienFire;                       //  0x2069
uint8_t        alienFireDelay;                        //  0x206a
uint8_t        pad_23;                                //  0x206b
uint8_t        temp206C;                              //  0x206c
uint8_t        invaded;                               //  0x206d
uint8_t        skipPlunger;                           //  0x206e
uint8_t        pad_24;                                //  0x206f
uint8_t        otherShot1;                            //  0x2070
uint8_t        otherShot2;                            //  0x2071
uint8_t        vblankStatus;                          //  0x2072
AShot          aShot;                                 //  0x2073
uint8_t        alienShotDelta;                        //  0x207e
uint8_t        shotPicEnd;                            //  0x207f
uint8_t        shotSync;                              //  0x2080
uint8_t        tmp2081;                               //  0x2081
uint8_t        numAliens;                             //  0x2082
uint8_t        saucerStart;                           //  0x2083
uint8_t        saucerActive;                          //  0x2084
uint8_t        saucerHit;                             //  0x2085
uint8_t        saucerHitTime;                         //  0x2086
SprDesc        saucerDesc;                            //  0x2087
uint8_t        saucerDXr;                             //  0x208c
Word           sauScore;                              //  0x208d
Word           shotCount;                             //  0x208f
Word           saucerTimer;                           //  0x2091
uint8_t        waitStartLoop;                         //  0x2093
uint8_t        soundPort3;                            //  0x2094
uint8_t        changeFleetSnd;                        //  0x2095
uint8_t        fleetSndCnt;                           //  0x2096
uint8_t        fleetSndReload;                        //  0x2097
uint8_t        soundPort5;                            //  0x2098
uint8_t        extraHold;                             //  0x2099
uint8_t        tilt;                                  //  0x209a
uint8_t        fleetSndHold;                          //  0x209b
uint8_t        pad_25[36];                            //  0x209c

// end of partial ram restore at 0x20c0

uint8_t        isrDelay;                              //  0x20c0
uint8_t        isrSplashTask;                         //  0x20c1
uint8_t        splashAnForm;                          //  0x20c2
Word           splashDelta;                           //  0x20c3
Word           splashPos;                             //  0x20c5
Word           splashPic;                             //  0x20c7
uint8_t        splashPicSize;                         //  0x20c9
uint8_t        splashTargetX;                         //  0x20ca
uint8_t        splashReached;                         //  0x20cb
Word           splashImRest;                          //  0x20cc
uint8_t        twoPlayers;                            //  0x20ce
uint8_t        aShotReloadRate;                       //  0x20cf
uint8_t        pad_26[21];                            //  0x20d0
Word           playerExtras;                          //  0x20e5
Word           playerStates;                          //  0x20e7
uint8_t        gameTasksRunning;                      //  0x20e9
uint8_t        coinSwitch;                            //  0x20ea
uint8_t        numCoins;                              //  0x20eb
uint8_t        splashAnimate;                         //  0x20ec
Word           demoCmdPtr;                            //  0x20ed
uint8_t        gameMode;                              //  0x20ef
uint8_t        pad_27;                                //  0x20f0
uint8_t        adjustScore;                           //  0x20f1
Word           scoreDelta;                            //  0x20f2
Word           HiScor;                                //  0x20f4
uint8_t        pad_28[2];                             //  0x20f6
Word           P1Scor;                                //  0x20f8
uint8_t        pad_29[2];                             //  0x20fa
Word           P2Scor;                                //  0x20fc
uint8_t        pad_30[68];                            //  0x20fe

// end of ram mirror at 0x2100 ram 镜像止于2100

uint8_t        p1ShieldBuffer[176];                   //  0x2142
uint8_t        pad_31[9];                             //  0x21f2
uint8_t        p1RefAlienDX;                          //  0x21fb
Word           p1RefAlienPos;                         //  0x21fc
uint8_t        p1RackCnt;                             //  0x21fe
uint8_t        p1ShipsRem;                            //  0x21ff
uint8_t        pad_32[66];                            //  0x2200
uint8_t        p2ShieldBuffer[176];                   //  0x2242
uint8_t        pad_33[9];                             //  0x22f2
uint8_t        p2RefAlienDX;                          //  0x22fb
Word           p2RefAlienPos;                         //  0x22fc
uint8_t        p2RackCnt;                             //  0x22fe
uint8_t        p2ShipsRem;                            //  0x22ff
uint8_t        pad_34[256];                           //  0x2300
uint8_t        vram[7168];                            //  0x2400

//2400-3FFF 7K 分配给Video RAM，但是未在其中有所体现,会在边使用这些地址

// Technically the region below is supposed to be a mirror, but AFAICT,
// that property is not used by the SI code.
//
// The 'PLAy' animation does do some oob writes during DrawSprite,
// which effectively do nothing because they end up trying to write to ROM
//
// So, as a catchall, we just reserve this area up to the end of the address space.

uint8_t        oob[49152];                            //  0x4000

} __attribute  ((packed));

typedef struct Mem Mem;

typedef struct PriCursor
{
    uint8_t* src;
    Word sc;
    uint8_t* obj;
} PriCursor;

typedef struct ShieldBufferCursor
{
    Word sc;
    uint8_t* iter;
} ShieldBufferCursor;

typedef enum YieldReason
{
    YIELD_INIT = 0,
    YIELD_TIMESLICE,
    YIELD_INTFIN,
    YIELD_WAIT_FOR_START,
    YIELD_PLAYER_DEATH,
    YIELD_INVADED,
    YIELD_TILT,
    YIELD_UNKNOWN,
} YieldReason;

enum Keys {
    KEYS_LEFT      =  1,
    KEYS_RIGHT     =  2,
    KEYS_START     =  4,
    KEYS_START2    =  8,
    KEYS_FIRE      =  16,
    KEYS_COIN      =  32,
    KEYS_TILT      =  64,
    KEYS_DIP6      =  128,
    KEYS_DIP7      =  256,
    KEYS_SPECIAL1  =  512,
    KEYS_SPECIAL2  =  1024,
    KEYS_QUIT      =  2048
};

#define KEY_LIST \
    KEY_MAP('a', KEYS_LEFT); \
    KEY_MAP('d', KEYS_RIGHT); \
    KEY_MAP('1', KEYS_START); \
    KEY_MAP('2', KEYS_START2); \
    KEY_MAP('j', KEYS_FIRE); \
    KEY_MAP('5', KEYS_COIN); \
    KEY_MAP('t', KEYS_TILT); \
    KEY_MAP('6', KEYS_DIP6); \
    KEY_MAP('7', KEYS_DIP7); \
    KEY_MAP('z', KEYS_SPECIAL1); \
    KEY_MAP('x', KEYS_SPECIAL2); \
    KEY_MAP(SDLK_ESCAPE, KEYS_QUIT);

#define TRUE 1
#define FALSE 0

static void do_logprintf(const char *file, unsigned line, const char* format, ...);
#define logprintf(...) { do_logprintf(__FILE__, __LINE__, __VA_ARGS__); }

#include "si78c_proto.h"

// Coordinate Systems
// ------------------
//
// Natural Units
// -------------
//
// For readability, this codebase uses the following coordinate system
// where possible.
//
// The origin is at the bottom left corner.
//
// X goes +ve towards the rhs of the screen.
// Y goes +ve towards the top of the screen.
//
//  (0,256)
//    ^
//    |
//  y |
//    |
//    |----------> (224,0)
//  (0,0)  x
//
// Game Units
// -------------
//
// The game uses two different coordinate systems, both of which
// fit into a 16-bit word, and come with an offset.
//
// pix  - Pixel positions between 0x2000 and 0xffff 
//pix  - 像素坐标 取值范围是 0x2000 到 0xffff （大小为 16bit）
// sc   - Screen RAM addresses between 0x2400 and 0x3fff 
//显示屏的RAM 地址在2400 到 3fff区间，前边留下的2400以后的内容，3fff后及是mirror空间
//
// pix coordinates are used to move and draw objects that require per pixel shifting,
// like the aliens and the bullets.
//
// sc coordinates are used for drawing text and other simple sprites, and also when
// blitting sprites after they have been shifted.
//
// The following macros are used to convert between natural units and game units.

// 屏幕地址信息的转换
#define xysc(x, y)     ((x)*32 + (y)/8)
//屏幕xysc到sc描述的2400 - 3fff 的地址的转换
#define xytosc(x, y)   u16_to_word(0x2400 + xysc((x),(y)))
// 像素信息的转换
#define xpix(x)        ((x)+32)
// 转到 pix对应的地址
#define xytopix(x, y)  u16_to_word((xpix((x)) << 8) | (y))


// 协程运行栈的大小
#define STACK_SIZE 65536

// 控制main_ctx运行时间的两个魔数，为什么是这两个数字
#define CRED1 17152 // from Jacky Chen, 端口号，只需要大于9999即可，满足随机分配 
//0x4300, 网上有说法是占用CPU最高的线程TID
#define CRED2 16384
//sysctl -w net.core.rmem_max=16384
//sysctl -w net.core.wmem_max=16384 

/*这个示例程序中，uc_stack是context用到的栈空间，设置为16KB；线程最小stack空间定义在pthread.h中，由常量PTHREAD_STACK_MIN指定，Debian GNU/Linux 8上为#define PTHREAD_STACK_MIN 16384.

作者：Haoqian
链接：https://www.jianshu.com/p/fbfebe426741
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
*/

static Mem m;
static uint8_t *rawmem = (uint8_t*) &m;

static int64_t ticks;
static int im;
static int irq_state;
static int irq_vector;

static uint16_t shift_data;
static uint8_t shift_count;


/*
si78c 使用了 ucontex 库的 协程 模拟原版街机的进程调度和中断操作。

协程：协程更加轻便快捷、节省资源，协程 对于 线程 就相当于 线程 对于 进程。

其中 ucontext 提供了 getcontext()、makecontext()、swapcontext() 以及 setcontext() 函数实现协程的创建和切换，si78c 中的初始化函数为 init_thread。下面我们直接来看源码中的例子：

*/

//切换协程时的中间变量
static ucontext_t frontend_ctx;
//游戏的主要逻辑
static ucontext_t main_ctx;
//游戏中断逻辑协程
static ucontext_t int_ctx;

//用于切换两协程
static ucontext_t *prev_ctx;
static ucontext_t *curr_ctx;

static uint8_t main_ctx_stack[STACK_SIZE];
static uint8_t int_ctx_stack[STACK_SIZE];

static YieldReason yield_reason;

static SDL_Window *window;
static SDL_Renderer *renderer;

static const int renderscale = 2;

static uint64_t keystate;

static int exited;

static uint8_t port1;
static uint8_t port2;


// 从 main () 函数开始执行
int main(int argc, char **argv)
{
    //初始化 SDL 和 游戏窗口
    init_renderer();
    //初始化游戏
    init_game();

    int credit = 0;
    size_t frame = -1;

    // 开始游戏协程调度与模拟触发中断
    while (1)
    {
        frame++;
        //处理按键输入
        input();
        // 如果退出标志置位退出循环清理游戏内存
        if (exited) break;

        // preserves timing compatibility with MAME
        //保留与 MAME（一种街机）的时序兼容性
        if (frame == 1)
            credit--;




        /**
         *  执行其他进程大概 CRED1 and CRED2 的时间17152 16348
         */

        // up to mid  在这基础上随机分配一个（9999） from Jacky chen

        credit += CRED1;
        // 设置场中间中断标志位，在下面的 loop_core()中会切换到 int_ctx 执行一次，然后清除标志
        loop_core(&credit);
        // 道理同上
        irq(0xcf);// 两个中断

        // up to vblank

        credit += CRED2;

        loop_core(&credit);
        //设置垂直消隐中断标志位，下个循环时候 loop_core() 中会切换到 int_ctx 执行一次，然后清除标志
        irq(0xd7);// 实现两个中断
        //绘制游戏界面
        render();
    }

    fini_game();
    fini_renderer();

    return 0;
}


static void input()
{
    // 取出所有未决的事件（最多64个）存储到 event_buffer 中
    SDL_Event event_buffer[64];
    size_t num = 0;

    while (num < 64)
    {
        int has = SDL_PollEvent(&event_buffer[num]);
        if (!has) break;
        num++;
    }

    for (size_t i = 0; i < num; ++i)
    {
        SDL_Event e = event_buffer[i];

        // 如果是退出事件就转化为 ESC 键被按下
        if (e.type == SDL_QUIT)
        {
            e.type = SDL_KEYDOWN;
            e.key.keysym.sym = SDLK_ESCAPE;
        }

        // 不是按键事件则跳过
        if (! (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)) continue;


        // 重置按键掩码为0
        uint64_t mask = 0;

        // f 标志 这是不是一个 “按键被按下” 事件
        uint64_t f = e.type == SDL_KEYDOWN;

        // 获取当前按键对应的掩码(mask)
        switch (e.key.keysym.sym)
        {

// 这个宏定义表示 如果输入的是 X，则 mask 就是 y
#define KEY_MAP(x, y) case x: mask = y; break;
            KEY_LIST

        /** 这里的 KEY_LIST 会展开成一大陀 case 语句，比如
         * KEY_MAP('a', KEYS_LEFT) 会扩展成
         * case 'a':
         *      mask = KEYS_LEFT
         *      break;
         */

#undef KEY_MAP
        }

        /** (keystate & ~mask) 先将 mask 对应按键的标志位清零
         * 后边or (-f & mask) 表示 如果按键按下则置位 mask 对应的按键标志位为 1，否则不管
         */
        keystate = (keystate & ~mask) | (-f & mask);
    }
 
 /** 这个宏用两个叹号从而将非 0 值转为 1 而 0 还是 0
     * 比如 x = 250
     * !!x 就是 1
     * x = 0
     * !!x 还是 0
     */
#define BIT(x) (!!(keystate & (x)))  

    /**
     * 将不同的标志位分别存储到两个模拟端口
     * https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
     * 其中 Inputs 和  Output 揭示了每个 bit 含义
     * 
     */
    port1 = (BIT(KEYS_RIGHT) << 6) |
            (BIT(KEYS_LEFT) << 5) |
            (BIT(KEYS_FIRE) << 4) |
            (1 << 3) |
            (BIT(KEYS_START) << 2) |
            (BIT(KEYS_START2) << 1) |
            (BIT(KEYS_COIN) << 0);

    port2 = (BIT(KEYS_DIP7) << 7) |
            (BIT(KEYS_RIGHT) << 6) |
            (BIT(KEYS_LEFT) << 5) |
            (BIT(KEYS_FIRE) << 4) |
            (BIT(KEYS_DIP6) << 3) |
            (BIT(KEYS_TILT) << 2);

    exited = BIT(KEYS_QUIT);
}

static void init_renderer()
{
    //初始化SDL子功能
    int rc = SDL_Init(SDL_INIT_EVERYTHING);
    assert(rc == 0);

    // 新建窗口，标题为 si78c 窗口位置在屏幕正中间，大小为 宽 224 * renderscale 高 256 * renderscale
    window = SDL_CreateWindow("si78c", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            224 * renderscale, 256 * renderscale, 0);
    assert(window);
    // 隐藏光标
    SDL_ShowCursor(SDL_DISABLE);
    // 在上面新建的窗口上启动一个渲染器，使用硬件加速并和屏幕刷新率相同
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    assert(renderer);
}

static void fini_renderer()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


/**
 * 渲染游戏画面，模拟显卡的操作（将显存的内容展示到屏幕上）
 * 可以看 https://computerarcheology.com/Arcade/SpaceInvaders/Code.html
 * 的 Text Character Sprites 部分，里面展示了像素是怎么存储的
 */

static void render()
{
    // 设置绘制颜色 黑色 不透明
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // 清空渲染器
    SDL_RenderClear(renderer);
    // 设置渲染器颜色 白色 不透明
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // 让 iter 指向 m.vram 就是模拟硬件的显存区域
    const uint8_t *iter = rawmem + 0x2400;
    // 遍历窗口，逐个像素绘制
    for (int y = 0; y < 224; ++y)
    {
        for (int xi = 0; xi < 32; ++xi)
        {
            // 获取当前现存的 8bit 内容并让现存指针指向下一个地址
            uint8_t byte = *iter++;
            // 每个 bit 都代表当前是否存在像素块，所以逐位判断
            for (int i = 0; i < 8; ++i)
            {
                int x = xi * 8 + i;
                //and操作判断像素存在
                int on = (byte >> i) & 0x1;
                // 如果当前存在像素就画一个像素块
                if (on)
                {
                    SDL_Rect rect;
                    rect.x = y * renderscale;
                    rect.y = (256 - x - 1) * renderscale;
                    rect.w = renderscale;
                    rect.h = renderscale;
                    // 绘制一个正方体充当像素块
                    SDL_RenderDrawRect(renderer, &rect);
                }
            }
        }
    }

    // 更新屏幕以显示最新的内容
    SDL_RenderPresent(renderer);
}

static void loop_core(int *credit)
{
    int allowed = 1;


    /**
     * 在 credit 归零前一直执行
     * credit 每次减去执行 execute 花费的 tick(人为指定大小)
     * 以此到达给其他进程分配足够执行时间的效果
    */

    while (*credit > 0)//归0以前一直执行
        *credit -= execute(allowed);
}

static void init_game()
{
    assert(sizeof(m) == 0x10000);
     // 加载 ROM 中游戏资源
    load_rom(&m);

    // 校验读取内容
    assert(checksum(&m) == 0x6dfbd7cc);

    // 初始化协程，让 run_main_ctx 参数是 YIELD_INIT
    init_threads(YIELD_INIT);
}

static void fini_game()
{
}

//初始化游戏进程

static void init_threads(YieldReason entry_point)
{
    //获取当前上下文，存储在main_crx中
    int rc = getcontext(&main_ctx);
    assert(rc == 0);
    //指定栈空间
    main_ctx.uc_stack.ss_sp = main_ctx_stack;
    //指定栈空间大小
    main_ctx.uc_stack.ss_size = STACK_SIZE;
    //设置后继上下文
    main_ctx.uc_link = &frontend_ctx;

    //修改main_ctx的上下文，并使其指向 run_main_ctx 函数
    makecontext(&main_ctx, (void (*)()) run_main_ctx, 1, entry_point);




    /*
     * 以上内容相当于新建了一个叫 main_cxt 的协程，运行 run_main_ctx 函数, frontend_ctx 为后继上下文
     * (run_main_ctx 运行完毕之后会接着运行 frontend_ctx 记录的上下文）
     * 协程 对于 线程，就相当于 线程 对于 进程
     * 只是协程切换开销更小，用起来更加轻便
     */




    //获取当前的上下文，存储在int_ctx当中
    rc = getcontext(&int_ctx);

    //指定栈空间
    int_ctx.uc_stack.ss_sp = &int_ctx_stack;
    //指定栈空间大小
    int_ctx.uc_stack.ss_size = STACK_SIZE;
    //设置后继的上下文
    int_ctx.uc_link = &frontend_ctx;

    //修改上下文，使其指向 run_init_ctx 函数
    makecontext(&int_ctx, run_int_ctx, 0);


    /*
     * 以上内容相当于新建了一个叫 int_cxt 的协程，运行 run_main_ctx 函数, frontend_ctx 为后继上下文
     * (run_main_ctx 运行完毕之后会接着运行 frontend_ctx 记录的上下文）
     * 协程 对于 线程，就相当于 线程 对于 进程
     * 只是协程切换开销更小，用起来更加轻便
     */



    //给 pre_ctx 初始值，在第一次调用 timeslice() 的时候能够切换到 main_ctx 上运行
    prev_ctx = &main_ctx;
    //给 curr_ctx 初始值， 这个时候 frontend_ctx 还是空的
    //frontend_ctx会在上下文切换时保存上一个协程的状态
    curr_ctx = &frontend_ctx;
}

/*
 * 之后每次调用 yield() 都会使用 swapcontext() 来进行两个协程间的切换
*/



// main_ctx 协程，用于处理游戏开始结束等情况
static void run_main_ctx(YieldReason entry)
{
    switch(entry)
    {
        case YIELD_INIT:           reset(); break;
        case YIELD_WAIT_FOR_START: WaitForStart(); break;// 投币了才会到这里
        case YIELD_PLAYER_DEATH:   player_death(0); break;
        case YIELD_INVADED:        on_invaded(); break;
        case YIELD_TILT:           on_tilt(); break;
        default: assert(FALSE);
    }
}


// 处理中断的函数
static void run_int_ctx()
{
    while (1)
    {
        // 0xcf = RST 1 opcode (call 0x8)
        // 0xd7 = RST 2 opcode (call 0x16)

        if (irq_vector == 0xcf)
            midscreen_int();
        else if (irq_vector == 0xd7)
            vblank_int();
        // 使能中断
        enable_interrupts();
        yield(YIELD_INTFIN);
    }
}

// 计算checksum校验信息
static unsigned checksum(Mem *m)
{
    assert(sizeof(*m) == 0x10000);
    assert((uintptr_t) m % 4 == 0);
    unsigned *ptr = (unsigned*) m;
    size_t n = sizeof(*m) / 4;

    unsigned sum = 0;

    for (size_t i = 0; i < n; ++i)
        sum += ptr[i];

    return sum;
}

// 从 ROM 中加载数据
static void rom_load(void *mem, const char *name, size_t offset, size_t len)
{
    // 根据文件名生成相应的文件路径
    char fbuf[256];
    sprintf(fbuf, "inv1/%s", name);

    // 打开 ROM 文件
    FILE *romfile = fopen(fbuf, "r");
    assert(romfile);

    // 读取 ROM 中的数据并存放在 mem 的指定位置中
    ssize_t rn = fread((char *)mem + offset, 1, len, romfile);
    assert((size_t)rn == len);
    // 释放文件
    fclose(romfile);
}



// 加载 ROM 游戏数据（图片等内容）
static void load_rom(void *mem)
{
    rom_load(mem, "invaders.h", 0x0000, 0x0800);
    rom_load(mem, "invaders.g", 0x0800, 0x0800);
    rom_load(mem, "invaders.f", 0x1000, 0x0800);
    rom_load(mem, "invaders.e", 0x1800, 0x0800);
}


// 根据游戏状态标志切换到相应的上下文
static int execute(int allowed)
{
    int64_t start = ticks;

    ucontext_t *next = NULL;

    switch (yield_reason)
    {
        //刚启动时，yield_reason 是 0 来表示 YIELD_INIT
        case YIELD_INIT:
        // 当需要延迟的时候会调用 timeslice() 将 yield_reason 切换为 TIELD_TIMESLICE
        // 模拟时间片轮转，这个时候会切换回上一个运行的任务（总共只有两个协程），实现时间片轮转
        case YIELD_TIMESLICE:
            next = prev_ctx;
            break;
        case YIELD_INTFIN:
        // 处理完中断后让 int_ctx 休眠，重新运行 main_ctx
            next = &main_ctx;
            break;

        //玩家死亡·等待开始·外星人入侵状态
        case YIELD_PLAYER_DEATH:
        case YIELD_WAIT_FOR_START:
        case YIELD_INVADED:
            init_threads(yield_reason);
            enable_interrupts();
            next = &main_ctx;
            break;
        //退出游戏
        case YIELD_TILT:
            init_threads(yield_reason);
            next = &main_ctx;
            break;
        default:
            assert(FALSE);
    }

    yield_reason = YIELD_UNKNOWN;

    //如果有中断产生
    if (allowed && interrupted())
    {
        next = &int_ctx;
    }

    switch_to(next);

    return ticks - start;
}



/*  需要注意的是，在 execute() 中进行了协程的切换，
    这个时候 execute() 的运行状态就被保存在了变量 frontend_ctx 之中，
    指针 prev_ctx 更新为指向 frontend_ctx，
    指针 curr_ctx 更新为指向 main_ctx
*/



// 协程切换函数
static void switch_to(ucontext_t *to)
{
    //给co_swtich 包装了一层，简化了代码量
    co_switch(curr_ctx, to);
}

// swtich_to 内的协程切换函数
static void co_switch(ucontext_t *prev, ucontext_t *next)
{
    prev_ctx = prev;
    curr_ctx = next;

    //切换到 next 指向的 上下文，并将当前的上下文保存在prev当中
    swapcontext(prev, next);
}

static void timeslice()
{
    ticks += 30;

    yield(YIELD_TIMESLICE);
}





static void yield(YieldReason reason)
{
    // 调度原因
    yield_reason = reason;
    // 调度到另一个协程上
    switch_to(&frontend_ctx);
}



static uint8_t get_input(int64_t ticks, uint8_t port)
{
    if (port == 1)
        return port1;

    if (port == 2)
        return port2;

    fatalerror("unknown port %d\n", port);
    return 0;
}

/**
 * 以下 read_port 和 write_port 模拟了8080 处理器读写硬件的操作
 * 这里作者只保留了对 移位寄存器 的模拟，其他诸如 看门狗、声音等都没有实现
 * 所有 IO 口请见
 * https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
 * 中的 I/O Ports 条目
 * 
 * 关于移位寄存器：
 * 8080 指令集不包括用于移位的操作码。必须将 8 位像素图像转换为 16 位字
 * 以用于屏幕上所需的位置。Space Invaders 添加了一个硬件移位寄存器来帮助计算
 * 
 */

// 模拟读硬件 IO 接口的操作
static uint8_t read_port(uint8_t port)
{
    // 读取偏移寄存器内容，只读取 16 bit 中的 8 到 15 - shift_count 位
    if (port == 3)
        return (shift_data << shift_count) >> 8;

    uint8_t val = get_input(ticks, port);
    return val;
}

// 模拟写硬件 IO 接口操作
static void write_port(uint16_t port, uint8_t v)
{
    if (port == 2)
    {
        // 二号口是位于寄存器偏移量， 与上 0x7 是因为这个寄存器现实中就只用 3 bit
        shift_count = v & 0x7;
    }
    else if (port == 4)
    {
        // 四号口是位移寄存器，先把原来的数据右移 8 bit，然后把新数据写在高 8 bit
        shift_data = (v << 8) | (shift_data >> 8);
    }

    timeslice();
}

// 使能中断
static void enable_interrupts()
{
    im = 1;
}

static void irq(uint8_t v)
{
    irq_vector = v;
    irq_state = 1;
}

static int interrupted()
{
    // The two interrupts correspond to midscreen, and start of vblank.
    // 0xcf = RST 1 opcode (call 0x8)
    // 0xd7 = RST 2 opcode (call 0x16)
    // 这两个中断对应到 midscreen（场中间），并且开始垂直消隐

    // 如果使能了中断并且存在中断
    if (irq_state && im)
    {
        assert(irq_vector == 0xcf || irq_vector == 0xd7);
        // 重置中断状态 重新使能
        irq_state = 0;
        im = 0;
        return TRUE;
    }

    return FALSE;
}

static void fatalerror(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);

    fflush(stdout);
    fflush(stderr);

    exit(1);
}


/**
 * 以下函数用于 word 和 虚拟内存地址的转换
 */


static inline Word u16_to_word(uint16_t u)
{
    Word w; w.u16 = u; return w;
}

static inline Word u8_u8_to_word(uint8_t h, uint8_t l)
{
    return u16_to_word((h << 8) | l);
}

static inline uint16_t ptr_to_u16(uint8_t *ptr)
{
    return (uint16_t) (ptr - (uint8_t*) &m);
}

static inline Word ptr_to_word(uint8_t *ptr)
{
    return u16_to_word(ptr_to_u16(ptr));
}

static inline uint8_t* u16_to_ptr(uint16_t u)
{
    return ((uint8_t*) &m) + u;
}

static inline uint8_t* word_to_ptr(Word w)
{
    return u16_to_ptr(w.u16);
}

static int is_godmode()
{
    uint16_t addr = 0x060f;
    uint8_t nops[] = {0,0,0};

    return memcmp(((uint8_t*) &m) + addr, nops, 3) == 0;
}



// 从 ram 计算出 rom 地址
static uint8_t* rompos(uint8_t* ram)
{
    return ram - 0x500;
}


// BCD码操作
static uint8_t bcd_add(uint8_t bcd, uint8_t a, uint8_t *carry)
{
    // Add the given number to the given bcd value, as per ADI / ADDB etc
    int q = bcd + a + *carry;
    *carry = (q >> 8) & 0x1;
    int aux = (bcd ^ q ^ a) & 0x10;
    bcd = q;

    // Adjust the result back into bcd as per DAA
    uint8_t w = bcd;

    if (aux || ((bcd & 0xf) > 9)) w += 6;
    if ((*carry) || (bcd > 0x99)) w += 0x60;
    *carry |= bcd > 0x99;
    bcd = w;

    return bcd;
}

static void do_logprintf(const char *file, unsigned line, const char* format, ...)
{
    fprintf(stdout, "%s:%d: ", file, line);

    va_list ap;
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);

    fflush(stdout);
}

static void DebugMessage(Word sc, uint8_t* msg, uint8_t n)
{
    uint16_t raw = sc.u16 - 0x2400;

    uint16_t x = raw / 32;
    uint16_t y = (raw % 32) * 8;
    static const char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<> =*...............?......-";

    char sbuf[256];

    for (size_t i = 0; i < n; ++i)
        sbuf[i] = alpha[msg[i]];

    sbuf[n] = '\0';

    logprintf("Print Message %04x %d (%d,%d) \"%s\"\n", ptr_to_word(msg).u16, n, x, y, sbuf);
}

// GAMECODE START

// The entry point on reset or power up 重启或启动的入口
static void reset()
{
    // xref 0000
    main_init(0);
}




/**
 * 在光将要击中屏幕中间（应该是模拟老式街机的现实原理）时由中断触发
 * 主要处理游戏对象的移动、开火、碰撞等等的检测更新与绘制（具体看函数 GameObj0到4）
 * 以及确定下一个将要绘制哪个外星人，检测外星人是不是入侵成功了
 */

// Executed via interrupt when the beam hits the middle of the screen
static void midscreen_int()
{
    // xref 0008
    // xref 008c
    // 更新 vblank 标志位
    m.vblankStatus = BEAM_MIDDLE;
    // 如果没有运动的游戏对象， 返回
    if (m.gameTasksRunning == 0) 
        return;
    // 在欢迎界面 且 没有在演示模式，返回（旨在游戏模式 和 demo模式下继续运行）
    if (!m.gameMode && !(m.isrSplashTask & 0x1)) 
        return;

    // xref 00a5
    // Run game objects but skip over first entry (player)
    //运行 game objects 但是略过第一个入口（玩家）
    RunGameObjs(u16_to_ptr(PLAYER_SHOT_ADDR));
    // 确定下一个将要绘制的外星人
    CursorNextAlien();
}


/*
 * 在这一部分中 RunGameObjs() 函数基本上包括了玩家的移动和绘制，
 * 玩家子弹和外星人子弹的移动、碰撞检测、绘制等等所有游戏逻辑的处理，
 * CursorNextAlien() 则找到要绘制的下一个活着的外星人设置标志位等待绘制，
 * 并且检测外星飞船是否碰到了屏幕底端。
 * 运行结束后会返回到 run_int_ctx() 继续运行直到 yield(YIELD_INTFIN) 表示协程切换回 execute()，
 * 并在 execute() 中重新将 next 设定为 main_ctx 使 main_init() 能够继续运行
*/





/** 
 * 当光击中屏幕最后一点（模拟老式街机原理）时触发
 * 主要处理游戏结束、投币、游戏中各种事件处理、播放演示动画
 */

// Executed via interrupt when the beam hits the end of the screen
static void vblank_int()
{
    // xref 0010
    // 更新标志位
    m.vblankStatus = BEAM_VBLANK;
    // 计时器减少
    m.isrDelay--;
    // 看看是不是投币了
    CheckHandleTilt();
    //检查是否投币
    vblank_coins();
    // 如果游戏任务没有运行，返回
    if (m.gameTasksRunning == 0)
        return;
    // 如果在游戏中的话
    if (m.gameMode)
    {
        // xref 006f
        TimeFleetSound();
        m.shotSync = m.rolShotHeader.TimerExtra;
        DrawAlien();
        RunGameObjs(u16_to_ptr(PLAYER_ADDR));
        TimeToSaucer();
        return;
    }

    // 如果投过币了
    if (m.numCoins != 0)
    {
        // xref 005d
        if (m.waitStartLoop) 
            return;
        
        m.waitStartLoop = 1;
        // 切换协程到等待开始循环
        yield(YIELD_WAIT_FOR_START);
        assert(FALSE);//不会再返回
    }
    // 如果以上都没发生，播放演示动画
    ISRSplTasks();
}

// Read coin input and handle debouncing
// 读取硬币输入操作并处理防抖动
static void vblank_coins()
{
    // xref 0020

    if (read_port(1) & COIN_BIT)
    {
        // xref 0067
        m.coinSwitch = 1; // Remember switch state for debounce

        // xref 003f
        // useless update
        m.coinSwitch = 1;
        return;
    }

    // xref 0026
    // Skip registering the credit if prev and current states were both zero
     // 如果当前按键和上一次按键的电平都是 0 则跳过本次按键操作（防止一直按着硬币不断增加）
    if (m.coinSwitch == 0)
        return;

    uint8_t bcd = m.numCoins;

    // Add credit if it won't cause a rollover
    // 增加 credit 防止硬币数溢出
    if (bcd != 0x99)
    {
        uint8_t carry = 0;
        // 硬币数加 1 ，这里是 BCD 码的加法操作
        m.numCoins = bcd_add(bcd, 1, &carry);
        // 绘制硬币数值
        DrawNumCredits();
    }

    m.coinSwitch = 0;
}

// Initialize the alien rack speed and position using the current player's alien rack data
// 用当前玩家的外星飞船数据初始化外星飞船的速度和位置
static void InitRack()
{
    // xref 00b1

    uint8_t* al_ref_ptr = GetAlRefPtr();

    Word refpos;
    refpos.y = *(al_ref_ptr);
    refpos.x = *(al_ref_ptr + 1);

    m.refAlienPos = refpos;
    m.alienPos = refpos;

    uint8_t dxr = *(al_ref_ptr - 1);

    if (dxr == 3)
        --dxr;

    m.refAlienDelta.x = dxr;
    m.rackDirection = (dxr == (uint8_t) -2) ? 1 : 0;
}

// Set up P1 and P2 Rack data
// 初始化 P1 和 P2 船数据
static void InitAlienRacks()
{
    // xref 00d7

    // Added for si78c testing infrastructure
    // In normal operation, dx is hardcoded to 2
    // 为 si78c 测试添加，一般情况下 dx 硬编码为 2
    uint8_t dx = ((uint8_t*) &m)[0x00d8];

    m.p1RefAlienDX = dx;
    m.p2RefAlienDX = dx;

    // xref 08e4
    if (m.twoPlayers)
        return;

    ClearSmallSprite(xytosc(168, 224), 32, 0);
}

// Draw the alien, called via vblank_int()
// Only one alien is ever drawn per frame, causing a ripple effect.
// This also somewhat determines the game speed.
//
// The alien rack is effectively moving at
// (2.0 / num_aliens) pixels per frame
//
// If 55 aliens are alive, then it will take almost one second to move all the aliens by 2 pixels.
// If  1 alien is alive, then it will only take one frame to move 2 pixels.



/**
 * 从 vblank_int() 被调应用，绘制外星人
 * 每一帧只绘制一个外星人，造成连锁反应
 * 这里某种程度上决定了游戏速度
 * 外型飞船以每帧（2.0 / num_aliens)像素的速度移动
 * 
 * 如果 55 艘飞船都存活，那么要花费将近一秒的时间去将所有的飞船移动两个像素
 * 如果只有 1 艘飞船存活，那么每一帧都会移动两个像素
 * 
 */


static void DrawAlien()
{
    // xref 0100

    // 如果有外星人正在爆炸
    if (m.alienIsExploding)
    {
        if (--m.expAlienTimer) return;


        // 飞船爆炸计时结束，飞船爆炸完成
        EraseSimpleSprite(m.expAlien.pos, 16);

        m.plyrShotStatus = 4;
        m.alienIsExploding = 0;
        SoundBits3Off(0xf7);
        return;
    }


    // 获取当前外星人指针
    uint8_t ai = m.alienCurIndex;
    uint8_t* alienptr = u16_to_ptr(m.playerDataMSB << 8 | ai);

    if (*alienptr == 0)
    {
        // The alien is dead, so skip it and tell CursorNextAlien to advance.
        m.waitOnDraw = 0;
        return;
    }

    // Look up the correct alien sprite based on row and anim frame

    uint8_t row = m.alienRow;
    static const uint8_t mul[] = {0,0,16,16,32};
    uint8_t* sprite = u16_to_ptr(0x1c00 + mul[row] + m.alienFrame * 48);

    SprDesc desc;
    desc.pos = m.alienPos;
    desc.spr = ptr_to_word(sprite);
    desc.n = 16;

    DrawSprite(&desc);
    m.waitOnDraw = 0; // This flag is synced with CursorNextAlien
}

// Find the next live alien to draw. Also detects whether rack has reached the bottom.
static void CursorNextAlien()
{
    // xref 0141
    // 玩家阵亡
    if (m.playerOK == 0) return;
    // 下一个外星人生成未准备完成
    if (m.waitOnDraw != 0) return;

    Word ai;
    ai.h = m.playerDataMSB;
    ai.l = m.alienCurIndex;
    uint8_t movecnt = 0x02; // limits MoveRefAlien to be called once

    // Advance the cursor until we find a live alien to draw.
    // If the cursor reaches the end, move ref alien, flip anim and reset cursor to zero.

    // 让飞船下标增加来寻找一个活着的外星人来绘制
    // 如果下标到达了末尾（标号 54），移动参考外星人，翻转动画帧，重置下标为 0


    while (TRUE)
    {
        // 协程切换，应该是为了内部的 tick 更新
        timeslice();
        ++ai.l;

        // inlined MoveRefAlien
        // 内联函数 MoveRefAlien
        if (ai.l == 55)
        {
            if (--movecnt == 0) return;

            m.alienCurIndex = 0;
            ai.l = 0;

            uint8_t dy = m.refAlienDelta.y;
            m.refAlienDelta.y = 0;
            AddDelta(&m.refAlienDelta.y, dy);

            m.alienFrame = !m.alienFrame;
            (void) (uint8_t) (m.playerDataMSB); // unused
        }

        // If we found live alien to draw, then break
        // 如果找到了活着的外星人，跳出循环以进行绘制
        if (*word_to_ptr(ai)) break;
    }
    // 将要绘制外星人的编号（下标）
    m.alienCurIndex = ai.l;

    uint8_t row = 0;
    Word pixnum = GetAlienCoords(&row, ai.l);
    m.alienPos = pixnum;


    // 如果外星人入侵成功
    if (pixnum.y < 40)
    {
        // xref 1971
        // kill the player due to invasion
        // 外星人入侵成功，杀死玩家
        m.invaded = 1;
        yield(YIELD_INVADED);
        assert(FALSE);
    }

    // 将要绘制外星人所在的行
    m.alienRow = row;
    // 更新标志位防止当前外星人还没绘制就被跳过了
    m.waitOnDraw = 1;
}

// Given alien index k, return alien position and row
// 通过外星人的索引 k 返回外星人所在的行和 xy 坐标
static Word GetAlienCoords(uint8_t *rowout, uint8_t k)
{
    // xref 017a

    uint8_t row = k / 11;
    uint8_t col = k % 11;

    uint8_t y = m.refAlienPos.y + 16 * row;
    uint8_t x = m.refAlienPos.x + 16 * col;

    *rowout = row;
    return u8_u8_to_word(x, y);
}

// Init all the aliens at dest to alive
// 复活（初始化） dest 指向的外星人
static void InitAliensSub(uint8_t* dest)
{
    // xref 01c0
    for (int i = 0; i < 55; ++i)
        *dest++ = 1;
}

// Init all the aliens for P1
// 为 P1 初始化所有外星人
static void InitAliensP1()
{
    // xref 01c0
    InitAliensSub(u16_to_ptr(P1_ADDR));
}

// Draw a one pixel (leftmost/bottommost in byte) stripe across the screen (224 pix wide)
// 16 pixels above the origin.
/**
 * 绘制一个 1 像素宽的直线在屏幕下面（224 像素长）
 * 在原点上方 16 个像素
 * 
 * 就是游戏区域和生命数分隔的横线
 */
static void DrawBottomLine()
{
    // xref 01cf
    ClearSmallSprite(xytosc(0,16), 224, 1);
}

// Given four bytes at vecptr treat them as two vecs - dy dx, y x
// and do this:
//
//     (y,x) += (c, dx)
//
// Used to move objects

/**
 * 将 vecptr 指向的地址视为两个 2 byte 的向量 —— (dx, dy) 和 (y, x)
 * 并且作如下操作：
 * （y, x) += (c, dx)
 * 
 * 用于移动对象
 */
static uint8_t AddDelta(uint8_t* vecptr, uint8_t c)
{
    vecptr++; // skip dy

    uint8_t dy = c;
    uint8_t dx = *vecptr++;

    uint8_t y = *vecptr + dy;
    *vecptr++ = y;

    uint8_t x = *vecptr + dx;
    *vecptr++ = x;

    return x;
}

// Restore into RAM mirror (addr, n) from ROM
// 从 ROM 中恢复 RAM 镜像
static void RestoreFromROM(uint8_t* addr, size_t n)
{
    BlockCopy(addr, rompos(addr), n);
}


// Restore the entire RAM mirror (256 bytes), used at startup
// 在启动时使用，用于恢复全部 256 bytes 的 RAM 镜像
static void CopyRomtoRam()
{
    // xref 01e6
    RestoreFromROM(u16_to_ptr(0x2000), 256);
}

// Partially restore the RAM mirror (first 192 bytes)
// The last 64 bytes are managed elsewhere, and some are
// persistent across games (hi scores etc).

/**
 * 以上内容翻译
 * --------------------
 * 专门用于恢复 RAM 镜像（前 192 bytes）
 * 最后的 64 bytes 被其他地方管理，并且有些部分始终贯穿游戏（比如 最高分）
 */
static void CopyRAMMirror()
{
    // xref 01e4
    RestoreFromROM(u16_to_ptr(0x2000), 192);
}

// Initialize P1 shields into off screen buffer
// 初始化 P1 玩家前面的盾牌
static void DrawShieldP1()
{
    // xref 01ef
    DrawShieldCommon(m.p1ShieldBuffer);
}

// Initialize P2 shields into off screen buffer
// 初始化 P2 玩家前面的盾牌
static void DrawShieldP2()
{
    // xref 01f5
    DrawShieldCommon(m.p2ShieldBuffer);
}

// Initialize shields into given buffer
static void DrawShieldCommon(uint8_t* dest)
{
    // xref 01f8
    size_t n = 44;

    for (int i = 0; i < 4; ++i, dest += n)
        BlockCopy(dest, m.SHIELD_SPRITE, n);
}

// Copy on screen shields into P1 off screen buffer to remember damage
// 将屏幕上的护盾复制到P1屏幕外缓冲区，以记住损坏情况
static void RememberShieldsP1()
{
    // xref 0209
    CopyShields(1, m.p1ShieldBuffer);
}

// Copy on screen shields into P2 off screen buffer to remember damage
static void RememberShieldsP2()
{
    // xref 020e
    CopyShields(1, m.p2ShieldBuffer);
}

// Copy off screen shields from P2 buffer back to screen
static void RestoreShieldsP2()
{
    // xref 0213
    CopyShields(0, m.p2ShieldBuffer);
}

// Copy off screen shields from P1 buffer back to screen
static void RestoreShieldsP1()
{
    // xref 021a
    CopyShields(0, m.p1ShieldBuffer);
}

// Generic shield copy routine. Copy into or out of given buffer.
// dir = 0 - buffer to screen
// dir = 1 - screen to buffer

/**
 * 通用盾牌复制函数，从给定 buffer 中复制出（入）
 * dir = 0 - 从 buffer 复制到 屏幕
 * dir = 1 - 从 屏幕 复制到 buffer
 * 
 * 用于更新屏幕上飞船前的盾牌
 */

static void CopyShields(uint8_t dir, uint8_t* sprbuf)
{
    // xref 021e
    m.tmp2081 = dir;

    Word sprsz = u8_u8_to_word(22, 2); // 22 rows, 2 bytes per row

    ShieldBufferCursor cursor;
    cursor.sc = xytosc(32, 48);
    cursor.iter = sprbuf;

    for (int i = 0; i < 4; ++i)
    {
        uint8_t unused = m.tmp2081;
        (void) (unused);

        // 执行内存复制工作
        CopyShieldBuffer(&cursor, sprsz, dir);
        // 更新屏幕指针
        cursor.sc.u16 += xysc(23, 0);
    }
}

// Do the generic (base class) handling for the 5 game objects.
// Checks the timer flags to see if object is ready to run, and if so
// finds the appropriate subclass handler and calls it.
/**
 * 处理五种游戏对象的通用方法（GameObj0-4)
 * 检查 timer 标志区设置对象是否准备运行，如果准备好了则找到合适的子类调用
 * 基本上包括了玩家的移动和绘制，玩家子弹和外星人子弹的移动、碰撞检测、绘制
 */

static void RunGameObjs(uint8_t* ptr)
{
    // xref 0248

    for ( ; ; ptr += 16)
    {
        GameObjHeader* obj = (GameObjHeader*)(ptr);
        uint8_t timer_hi = obj->TimerMSB;

        // end of list check
        if (timer_hi == 0xff) return;

        // object skip check
        if (timer_hi == 0xfe) continue;

        uint8_t timer_lo = obj->TimerLSB;

        // decrement timer if its not zero
        if (timer_hi | timer_lo)
        {
            // xref 0277
            // timers are big endian for some reason.

            if (timer_lo == 0) --timer_hi;   // decrement msb if necc
            --timer_lo;                      // decrement lsb

            obj->TimerLSB = timer_lo;
            obj->TimerMSB = timer_hi;

            continue;
        }

        uint8_t timer_extra = obj->TimerExtra;

        if (timer_extra != 0)
        {
            // xref 0288
            obj->TimerExtra--;
            continue;
        }

        // The object is ready to run, so grab the handler and run it
        // 当前对象已经准备好运行了，获取它的句柄并运行
        uint16_t fnlo = obj->Handler.l;
        uint16_t fnhi = obj->Handler.h;
        // 将两个 8 bit 合成 16 bit，区别调用不同的函数
        uint16_t fn = fnhi << 8 | fnlo;

        // 获取数据区域指针（RAM 中数据区域在 GameObjHeader 后面）
        // 实际上并未用到
        uint8_t *data = (ptr + sizeof(GameObjHeader));

        // 根据句柄区分当前应该调用哪个函数

        switch(fn)
        {
            case 0x028e: GameObj0(data); break;
            case 0x03bb: GameObj1(data); break;
            case 0x0476: GameObj2(data); break;
            case 0x04b6: GameObj3(data); break;
            case 0x0682: GameObj4(data); break;
            case 0x050e: ProcessSquigglyShot(); break; // splash anim
            default:
                 assert(FALSE);
                 break;
        }
    }
}

// Handles player movement and rendering
// Unlike other game objects, this is only run during the vblank interrupt, and
// runs on each vblank interrupt

/**
 * 处理玩家移动和绘制
 * 不同于其他游戏结构体，这个函数只在 vblank 中断期间运行，每个 vblank 中断都会运行
 */
static void GameObj0(uint8_t* unused)
{
    // xref 028e
    uint8_t pstate = m.playerAlive;

    if (pstate != 0xff) // 0xff means player is alive 活着
    {
        HandleBlowingUpPlayer(pstate);// 执行玩家爆炸操作
        return;
    }

    // xref 033b
    m.playerOK = 1;

    // xref 03b0
    // 判断外星人能不能开火
    if (m.enableAlienFire == 0)
    {
        // xref 03b3
        // 更新开火许可
        if (--m.alienFireDelay == 0)
            m.enableAlienFire = 1;
    }

    // xref 034a
    uint8_t x = m.playerDesc.pos.x;
    uint8_t input = 0;
    // 如果是演示模式则从内存中获取指令
    if (m.gameMode == 0)
    {
        //演示模式的下一个命令
        input = m.nextDemoCmd;
    }
    else // 游戏中读取玩家操作
    {
        input = ReadInputs();

        // Map joystick controls into the same domain as the demo commands
        // 将操纵杆的控制指令和 demo 的操作指令映射到相同的数字上
        if ((input >> 6) & 0x1)
            input = 1;
        else if ((input >> 5) & 0x1)
            input = 2;
        else
            input = 0;
    }

    if (input == 0)
    {
        // Do nothing
    }
    else if (input == 1)
    {
        // Move player to the right 向右
        if (x != xpix(185))
        {
            ++x;
            m.playerDesc.pos.x = x;
        }
    }
    else if (input == 2)
    {
        // Move player to the left 向左
        if (x != xpix(16))
        {
            --x;
            m.playerDesc.pos.x = x;
        }
    }
    //xpix 从16 到 185
    else
    {
        assert(FALSE);
    }

    DrawPlayer();
}

// Handle the player's death animation. Resets the stack once complete,
// and re-entry is through player_death(0)
/**
 * 处理玩家死亡动画，并完全重置一次栈
 * 从 player_death(0) 再次进入
 */
static void HandleBlowingUpPlayer(uint8_t anim)
{
    // xref 0296
    if (--m.expAnimateTimer != 0)
        return;

    // 0 代表爆炸
    m.playerOK = 0;
    // 0 代表外星人不能射击
    m.enableAlienFire = 0;
    // 外星人可以开火倒计时
    m.alienFireDelay = 48;
    // 下一帧爆炸画面更新时间
    m.expAnimateTimer = 5;

    //判断死亡动画是否播放完毕
    if (--m.expAnimateCnt != 0)
    {
        // Still animating the explosion
        // 还在动画
        anim = !anim;
        // 0 1 翻转代表播放动画
        m.playerAlive = anim;

        m.playerDesc.spr = ptr_to_word((m.PLAYER_SPRITES + (anim+1) * 16));
        DrawPlayer();
        return;
    }

    // 擦除玩家角色
    EraseSimpleSprite(m.playerDesc.pos, 16);
    // 从 ROM 中恢复玩家数据
    RestoreFromROM(u16_to_ptr(PLAYER_ADDR), PLAYER_SIZE);

    //协程调度
    SoundBits3Off(0);

    if (m.invaded)
        return;

    // Return to splash screens in demo mode
    // 由demo mode返回 初始屏幕
    if (m.gameMode == 0)
        return;

    // 由角色死亡 引发的 协程调度
    yield(YIELD_PLAYER_DEATH);
    assert(FALSE);
}

// Handle cleanup tasks related to player dying, such as lives and score adjustment,
// switching players and calling GameOver if necessary.
// If invaded is one, the player loses the game, regardless of number of lives left.
// This routine is called after a stack reset, see on_invaded() and HandleBlowingUpPlayer()

/**
 * 处理有关玩家死亡时的清理任务，比如生命数、调整得分、切换玩家或者在必要时调用 GameOver
 * 如果被入侵了，玩家不管还有多少条命都会游戏失败
 * 这个函数在栈重置后会被调用， 见 on_invaded() 和 HandleBlowingUpPlayer()
 */
static void player_death(int invaded)
{
    // xref 02d4
    // 此处的变量switch_players，和下边的if(!switch_players) 这段代码 表示是否应该切换玩家，
    int switch_players = player_death_sub(invaded);

    if (!switch_players)
    {
        if (!*(GetOtherPlayerAliveFlag()) || !m.twoPlayers)
        {
            RemoveShip();
            NewGame(0, 0, INIT | PROMPT | SHIELDS);
        }
    }

    uint8_t pnum = m.playerDataMSB;
    // 判断 MSB(由pnum传过来的)的末尾1bit是1 or 0
    // 由此判断玩家是player1 还是 player2
    if (pnum & 0x1)
        RememberShieldsP1();
    else
        RememberShieldsP2();

    uint8_t adx = 0;
    Word apos;
    uint8_t* aref = GetRefAlienInfo(&adx, &apos);

    *aref = apos.y;
    *(aref+1) = apos.x;

    *(aref-1) = adx;

    // about to switch players.
    // 即将切换玩家
    //此处存疑，为什么没在if 里
    CopyRAMMirror();

    // 重置为 p1 的 各项数值
    uint8_t carry = pnum & 0x1;
    uint8_t pmsb = 0x21; // p1
    uint8_t cbit = 0;
    // 如果是 carry == 1时，表明是player2，将各项数据转换成player2的
    if (carry)
    {
        cbit = 0x20; // cocktail bit=1
        pmsb = 0x22;
    }
    // 更新对应的玩家到MSB，究竟是player1 还是player2
    m.playerDataMSB = pmsb; // change players
    TwoSecDelay();
    // 清零计时器LSB
    m.playerHeader.TimerLSB = 0; // clear player object timer
    write_port(5, cbit);

    m.soundPort5 = (cbit + 1);
    ClearPlayField();
    RemoveShip();

    // jmp to 079b. (newgame + skip)
    NewGame(0, 0, INIT);
}

// Player death cleanup subroutine. Game is lost immediately if invaded=1
// Returns true if caller should switch players, false otherwise
// This subroutine will not return if all players have lost the game, instead it
// will call through to GameOver, which will start a new game
/** 翻译
 * 玩家死亡清理子程序，如果 invaded=1 游戏立即失败
 * 如果应该切换玩家返回 真，否则返回 假
 * 这个子程序在玩家游戏失败的情况下不会返回，而是通过调用 GameOver 开始新游戏
 * 
 * 主要处理更新最高分，是否直接游戏结束或者返回是不是要更换游戏玩家
 */
static int player_death_sub(int invaded)
{
    if (!invaded)
    {
        DsableGameTasks();

        uint8_t* unused;

        // still got some ships, keep going
        // 还有剩余船（多条命），游戏继续
        if (GetNumberOfShips(&unused) != 0)
            return FALSE;

        PrintNumShips(0);
    }

    // handle losing the game
    // 游戏失败
    *CurPlyAlive() = 0;
    // 更新最高分
    uint8_t* sc = GetScoreDescriptor();
    sc++;

    uint8_t* hi = &m.HiScor.h;

    uint8_t hi_score_msb = *(hi);
    uint8_t pl_score_msb = *(sc);

    hi--;
    sc--;

    uint8_t hi_score_lsb = *(hi);
    uint8_t pl_score_lsb = 0;

    int higher = FALSE;

    if (hi_score_msb == pl_score_msb)
    {
        // same msb, must check lower
        pl_score_lsb = *(sc);
        higher = pl_score_lsb > hi_score_lsb;
    }
    else
    {
        higher = pl_score_msb > hi_score_msb;
    }
    // 当前得分大于先前最高分
    if (higher)
    {
        *hi++ = *sc++;
        *hi = *sc;
        PrintHiScore();
    }

    // xref 1698
    // m.twoPlayers == 1，另一个玩家存活，需要再判断
    if (m.twoPlayers)
    {
        // Game over player<n>
        Word sc = xytosc(32, 24);
        sc = PrintMessageDel(sc, m.MSG_GAME_OVER__PLAYER___, 0x14);

        sc.u16 -= xysc(16, 0); // back up to player indicator
        uint8_t b = (m.playerDataMSB & 0x1) ? 0x1b : 0x1c;

        sc = DrawChar(sc, b);  // print player num
        OneSecDelay();
        // 判断是否游戏结束
        if (*(GetOtherPlayerAliveFlag()) == 0)
        {
            // 没有存活玩家，游戏结束，不再返回
            GameOver();     // won't return.
            assert(FALSE);
        }

        // switch players
        return TRUE;
    }
    // m.twoPlayers == 0，另一个玩家不存活，直接game over
    else
    {
        GameOver(); // won't return
        assert(FALSE);
    }

    return FALSE;
}

// Generic player shot drawing routine, originally multiple small fragments
// 通用玩家射击绘制程序，原始程序里分成了多个小碎片
static void DrawPlayerShot(int op)
{
    // xref 0404
    // xref 03f4
    SprDesc plyshot = ReadPlyShot();

    if (op == OP_BLEND)
    // plot 子弹
        DrawShiftedSprite(&plyshot);
    else if (op == OP_ERASE)
    // 子弹消失
        EraseShifted(&plyshot);
    else
        assert(FALSE);
}

// Handles player bullet movement, collision detection and rendering.
// At the end of the routine, the player's shot count is used to
// set up the next saucer direction and score.
/**翻译
 * 处理玩家子弹移动、碰撞检测和绘制
 * 在执行的最后，玩家射击计数被用于设定下一个飞碟的方向和得分
 */
static void GameObj1(uint8_t* unused)
{
    // xref 03bb
    //
    // Shot states:
    //
    // :Available(0), :Initiated(1), :Moving(2), :HitNotAlien(3),
    // :AlienExploded(4), :AlienExploding(5)

    // 射击状态
    // :可以开火（0），:初始化射击（1），:移动中（2），:击中非外星人物体（3），
    // :外星人爆炸（4），:外星人爆炸中（5）


    // 绘制完成，返回
    if (!CompXrToBeam(&m.playerShotDesc.pos.x))
        return;

    uint8_t status = m.plyrShotStatus;

    // 处于准备射击状态，返回
    if (status == 0)
        return;
    
    //处于射击状态，对玩家射击动作进行初始化
    if (status == 1)
    {
        // xref 03fa InitPlyShot
        //设定了标志位为 2
        m.plyrShotStatus = 2;
        // x 坐标定为 玩家的中心
        m.playerShotDesc.pos.x = m.playerDesc.pos.x + 8;
        // plot子弹
        DrawPlayerShot(OP_BLEND);
        return;
    }

    // 处于移动中的状态
    else if (status == 2)
    {
        // xref 040a MovePlyShot
        SprDesc copy = ReadPlyShot();
        EraseShifted(&copy);

        // 更新copy中子弹在y 的位置
        copy.pos.y += m.shotDeltaYr;
        // 更新Desc子弹Y轴的位置
        m.playerShotDesc.pos.y = copy.pos.y;
        // 检查子弹是否碰撞，plot子弹
        DrawSprCollision(&copy);
        uint8_t collided = m.collision;

        //如果没有碰撞的话，返回
        if (!collided) return;

        // 更新外星人的正在爆炸标志位
        m.alienIsExploding = collided;
        return;
    }


    // 处理外星人爆炸
    if (status != 3)
    {
        // xref 042a
        // 外星人处于爆炸中
        if (status == 5) return;

        // continues at EndOfBlowup
        // 在本函数后面的 EndOfBlowup 继续
    }
    // 击中的目标并不是外星人
    else
    {
        // xref 03d7
        // 爆炸时间
        if (--m.blowUpTimer != 0)
        {
            // The shot is blowing up
            //子弹处于正在爆炸状态
            if (m.blowUpTimer != 0x0f) return;

            // Draw the explosion the first time through
            // xref 03df
            // 删除子弹
            DrawPlayerShot(OP_ERASE);

            // Change the shot sprite to the explosion.
            // 切换为爆炸图片
            m.playerShotDesc.spr.l++;
            // 为爆炸修改位置
            // Modify the coords slightly for the explosion
            m.playerShotDesc.pos.y--; // y -= 2
            m.playerShotDesc.pos.y--;

            m.playerShotDesc.pos.x--; // x -= 3
            m.playerShotDesc.pos.x--;
            m.playerShotDesc.pos.x--;

            m.playerShotDesc.n = 8;

            //plot子弹爆炸
            DrawPlayerShot(OP_BLEND);
            return;
        }
    }

    // xref 0436 EndOfBlowup
    // 消除爆炸图
    DrawPlayerShot(OP_ERASE);

    // xref 0444
    // reset the shot
    // 重置shot (RAM)
    RestoreFromROM(u16_to_ptr(PLAYER_SHOT_DATA_ADDR), PLAYER_SHOT_DATA_SIZE);

    // The remaining code in GameObj0 is to do with adjusting the saucer bonus and
    // direction, which is changed up on every player shot fired.

    /**翻译
     * GameObj0 余下的代码用于在玩家每次射击后调整飞碟得分和方向
     */

    // Adjust the saucer bonus.
    // 调整saucer的bonus
    {
        Word table = m.sauScore;

        table.l++;

        if (table.l >= 0x63)
            table.l = 0x54;

        m.sauScore = table;
    }

    Word shots = m.shotCount;

    shots.l++;
    m.shotCount = shots;

    // xref 0461
    // If saucer still on screen, don't reset the direction.
    // saucer 仍在屏幕，不重设 方向
    if (m.saucerActive)
        return;

    // xref 0462
    //
    // This code is using the shot counter as an index into the ROM
    // (where some code resides), for the purposes of random number generation.
    //
    // For the saucer direction logic, only bit 0 of each bytecode is used.
    //
    // The 256 bytes used reside at 0800 -> 08FF
    //
    // If you check bit 0 of each byte in that ROM section, you will find that there is no bias,
    // and there are exactly 128 0's and 128 1's.
    //
    // It seems unlikely that this was an accident, I think Nishikado deliberately constructed
    // the ROM this way, and used some well placed NOPs to achieve fair balance.
    //
    // E.g. these NOPs
    //
    // 0854: 00 00 00
    // 0883: 00 00 00

    // This information can be exploited to the player's advantage.
    //
    // If using the shot counting trick to get high scores, the
    // expected saucer direction for the first 6 saucers (if counting),
    // will be as follows:
    //
    // [22,37,52,67,82,97]
    // [0, 1, 1, 0, 1, 1]
    // [L, R, R, L, R, R]

    /**
     * 以上内容翻译
     * ------------------------
     * 这部分代码用 shot counter 作为 ROM（某些代码存储的地方） 的索引来生成随机数
     * 
     * 只有 0 号比特位用于飞碟方向的判断
     * 
     * 使用的是位于 0x800 - 0x08FF 处的 256 bytes
     * 
     * 如果你检查相关 ROM 中每一字节的 0 号比特位，你会发现 1 和 0 各有 128 个
     * 
     * 这看起来不像巧合，我相信 Nishikado 是故意将这部分 ROM 组织成这样的，并且巧妙的放置了以下 NOP 指令来达到 0 1 平均
     * 
     * 比如这些 NOPs：
     * 0x0854: 00 00 00
     * 0x0883: 00 00 00
     * 
     * 这些信息可以为玩家带来好处
     * 
     * 比如用利用 shot counting 来得到高分，前 6 个飞碟的与其方向如下所示：
     * 
     * [22,37,52,67,82,97]
     * [0, 1, 1, 0, 1, 1]
     * [L, R, R, L, R, R]
     * 
     */

    uint8_t v = *(word_to_ptr(shots));

    // if lo bit of res is 0, then delta = -2, x = 192 (moving left from rhs)
    // if lo bit of res is 1, then delta = +2, x = 9   (moving right from lhs)

    uint8_t delta = -2;
    uint8_t     x = xpix(192);

    if (v & 0x1)
    {
        delta = 2;
        x     = xpix(9);
    }

    m.saucerDesc.pos.x = x;
    m.saucerDXr = delta;
}

// Return a copy of the player shot sprite descriptor
// 返回一个玩家射击精灵描述符的副本
static SprDesc ReadPlyShot()
{
    // xref 0430
    return ReadDesc(&m.playerShotDesc);
}

// Handles alien rolling shot firing, movement, collision detection and rendering.
// This is the shot that specifically targets the player.
// Most of the logic is shared between the 3 types inside HandleAlienShot.

/**翻译
 * 处理外星人 rolling shot 的开火、运动、碰撞检测、绘制
 * 这种射击会针对玩家
 * 游戏中 3 种射击类型都共用 HandleAlienShot 函数
 */

static void GameObj2(uint8_t* unused1)
{
    // xref 0476
    // reset the ROM
    RestoreFromROM(&m.rolShotHeader.TimerExtra, 1);

    if (m.rolShotData.CFir.u16 == 0)
    {
        // The rolling shot doesn't use a firing table to choose a column to fire
        // from, because it specifically targets the player.
        //
        // It just uses this member as a flag to delay firing the first rolling shot
        
        /**
         * 没有使用 firing table 去选择是哪列射击，而是根据玩家的位置决定射击的列
         * 
         * 这个成员仅作为滚动射击第一次开火的延迟标志位
         */

        m.rolShotData.CFir.u16 = 0xffff;
        return;
    }

    // 设定 rolling shot 射击动画内存
    ToShotStruct(&m.rolShotData, ROL_SHOT_PICEND);

    // 两种设计模式的更新
    m.otherShot1 = m.pluShotData.StepCnt;
    m.otherShot2 = m.squShotData.StepCnt;

    // 外星人开火
    HandleAlienShot();

    if (m.aShot.BlowCnt != 0)
    {
        // shot still running, copy updated data from active -> rolling and return.
        // 射击还在运行中，从 active 更新 rolling 数据然后返回
        FromShotStruct(&m.rolShotData);
        return;
    }
    // 恢复ROM内存
    RestoreFromROM(u16_to_ptr(ROLLING_SHOT_ADDR), ROLLING_SHOT_SIZE);
}

// Handles alien plunger shot firing, movement, collision detection and rendering.
// 处理外星人 plunger shot 的开火、移动、碰撞检测和绘制
static void GameObj3(uint8_t* unused)
{
    // xref 04b6
    // 如果跳过 plunger shot
    if (m.skipPlunger) return;
    // 同步更新控制
    if (m.shotSync != 1) return;

    // 设置 plunger shot 的动画内存
    ToShotStruct(&m.pluShotData, PLU_SHOT_PICEND);
    
    // 更新其他两种射击状态的信息
    m.otherShot1 = m.rolShotData.StepCnt;
    m.otherShot2 = m.squShotData.StepCnt;
    
    // 专门处理开火
    HandleAlienShot();

    if (m.aShot.CFir.l >= 16)
    {
        // 重设fire table
        m.aShot.CFir.l = *(rompos(&m.pluShotData.CFir.l));
    }
    
    // 如果是true,代表还没爆炸完毕，存储当前状态
    if (m.aShot.BlowCnt)
    {
        FromShotStruct(&m.pluShotData);
        return;
    }

    //从ROM恢复内存
    RestoreFromROM(u16_to_ptr(PLUNGER_SHOT_ADDR), PLUNGER_SHOT_SIZE);

    if (m.numAliens == 1)
        m.skipPlunger = 1;

    // 同上，更新 Fire table
    m.pluShotData.CFir = m.aShot.CFir;
}

// Handles alien squiggly shot firing, movement, collision detection and rendering.
// This is very similar logic to the plunger shot except the column firing table
// is different.

/**翻译
 * 处理外星人 squiggly shot 的开火、移动、碰撞检测、绘制
 * 这里 和 plunger shot 的逻辑非常相似，只是 firing table 不一样
 */

// 这里结构一致不做多余的备注
static void ProcessSquigglyShot()
{
    // xref 050f
    ToShotStruct(&m.squShotData, SQU_SHOT_PICEND);

    m.otherShot1 = m.pluShotData.StepCnt;
    m.otherShot2 = m.rolShotData.StepCnt;

    HandleAlienShot();

    if (m.aShot.CFir.l >= 21)
    {
        // Restores to the rom lsb values of '6'
        m.aShot.CFir.l = *(rompos(&m.squShotData.CFir.l));
    }

    if (m.aShot.BlowCnt)
    {
        FromShotStruct(&m.squShotData);
        return;
    }

    RestoreFromROM(u16_to_ptr(SQUIGGLY_SHOT_ADDR), SQUIGGLY_SHOT_SIZE);
    m.squShotData.CFir = m.aShot.CFir;
}

// Copy an alien shot structure from src into the active alien shot structure,
// and configure the shot animation.

/**翻译
 * 从源码中复制射击结构体替换当前活动外星人的射击结构体，并初始化射击动画
 *
 */
 
static void ToShotStruct(AShot* src, uint8_t picend)
{
    // xref 0550
    m.shotPicEnd = picend;
    BlockCopy(&m.aShot, src, 11);
}

// Copy the active alien shot structure into dest
// 将当前活动外星人的 射击structure 复制到 des

static void FromShotStruct(AShot* dest)
{
    // xref 055b
    BlockCopy( dest,  &m.aShot, 11);
}

// This logic is shared between the 3 shot types.
// Handles shot firing, movement, collision detection and rendering.

/**
 * 这部分逻辑被 3 种射击类型（Squigly、Plunger、Rolling）共用
 * 处理子弹的开火、移动、碰撞检测、绘制
 */

static void HandleAlienShot()
{
    // xref 0563
    // 进入条件为当前有外星人设计动作
    if ((m.aShot.Status & SHOT_ACTIVE) != 0)
    {
        // 先处理好当前外星人开火的操作
        HandleAlienShotMove();
        return;
    }
    // 开始界面动画中射击 CCOIN 的多余字母 "C"
    uint8_t shooting_c = (m.isrSplashTask == 0x04);
    uint8_t fire_enabled = m.enableAlienFire;

    if (shooting_c)
    {
        // Special case for the splash animation
        // 飞溅动画的特殊情况
        m.aShot.Status |= SHOT_ACTIVE;
        m.aShot.StepCnt++;

        return;
    }

    // fire_enabled == 1 表示可以发射
    if (!fire_enabled)
        return;

    // 允许发射时 重设标志位
    m.aShot.StepCnt = 0;


    // 控制射速步伐
    {
        uint8_t steps = m.otherShot1;
        if (steps && steps <= m.aShotReloadRate) return;
    }

    {
        uint8_t steps = m.otherShot2;
        if (steps && steps <= m.aShotReloadRate) return;
    }

    uint8_t col = 0;

    if (m.aShot.Track == 0)
    {
        // xref 061b
        // Make a tracking shot, by finding the column that is above the player
        // 找到玩家在屏幕上所处的列来跟踪玩家发射子弹

        Word res = FindColumn(m.playerDesc.pos.x + 8); // find column over centre of player
        col = res.h; // res.l unused

        // 一共只有11colomn，超出了就更正为列数边界
        if (col >= 12)
            col = 11;
    }
    else
    {
        // xref 059c
        // Use the firing table pointer to pick the column, and advance it
        // 使用 firing table（ROM 中 0x1D00 - 0x1D10）  指针挑选一列开火
        
        uint8_t* hl = word_to_ptr(m.aShot.CFir);
        col = *hl++;
        m.aShot.CFir = ptr_to_word(hl);
    }

    // xref 05a5
    uint8_t k = 0;

    // 当前列中最靠前的外星人设定为目标，如果是0则代表没有外星人在当前列
    uint8_t found = FindInColumn(&k, col);

    // 返回的是0， 没有外星人在当前列，返回
    if (!found)
        return;

    uint8_t row_unused = 0;
    // 返回外星人当前的二位坐标系location
    Word pixnum = GetAlienCoords(&row_unused, k);

    // 在location 基础上做偏移，到alien ship的炮塔位置
    pixnum.x += 7;
    pixnum.y -= 10;

    // 将炮塔位置，设置为射击相关标志位
    m.aShot.Desc.pos = pixnum;
    m.aShot.Status |= SHOT_ACTIVE;
    m.aShot.StepCnt++;

    return;
}

// Handle moving the alien shot and some collision detection response.
// Returns 1 if shot status needs to be set to blowing up, 0 if not.

/**翻译
 * 处理外星人的射击、子弹碰撞检测、动画绘制
 * 如果子弹状态需要设为爆炸中则返回 1，否则返回 0
 */

static int DoHandleAlienShotMove()
{
    // 如果plot未完成，返回
    if (!CompXrToBeam(&m.aShot.Desc.pos.x))
        return 0;


    // 检查子弹状态是否是要爆炸
    if (m.aShot.Status & SHOT_BLOWUP)
    {
        // plot 子弹爆炸的动画
        ShotBlowingUp();
        return 0;
    }

    // xref 05cf
    // 抹除alien射击的子弹和动画
    m.aShot.StepCnt++;
    EraseAlienShot();

    // Animate the shot
    // plot射击动画
    uint8_t shotpic = m.aShot.Desc.spr.l + 3;

    // 动画循环播放，在到boundary之前，处在循环之中
    if (shotpic > m.shotPicEnd)
        shotpic -= 12;

    // 子弹移动的偏移计算
    m.aShot.Desc.spr.l = shotpic;
    m.aShot.Desc.pos.y = m.aShot.Desc.pos.y + m.alienShotDelta;
    // 绘制alien射击动画
    DrawAlienShot();

    // xref 05f3
    uint8_t y = m.aShot.Desc.pos.y;

    // 21为屏幕的lower bound，超出要爆炸
    if (y < 21)
        return 1;// 爆炸

    // 没有出现碰撞，返回0，0在函数前的注释中已经指出，维持原状态继续飞行
    if (!m.collision)
        return 0;

    /* 
     *注意：因为上面已经判断过是否碰撞了，所以下面部分代码默认发生了碰撞
    */ 

    y = m.aShot.Desc.pos.y;

    // below or above players area ?
    // 处在player的上或者下（不在玩家处）
    if (y < 30 || y >= 39)
        return 1;


    // 玩家处发生爆炸，玩家死亡（30-39）之间
    if (!is_godmode())
        m.playerAlive = 0;

    return 1;
}

// Handle moving the alien shot and some collision detection response.
// 处理外星人的射击状态更新和爆炸方向反馈

static void HandleAlienShotMove()
{
    // xref 05c1
    int exploded = DoHandleAlienShotMove();

    // 爆炸，重新设置标志位
    if (exploded)
        m.aShot.Status |= SHOT_BLOWUP;
}

// Find a live alien in the given column.
// 在当前列找一个存活alien
static uint8_t FindInColumn(uint8_t *out, uint8_t col)
{
    // xref 062f
    Word hl;

    // 玩家MSB 中的外星人存活list
    hl.h = m.playerDataMSB;
    hl.l = col - 1;

    int found = 0;

    // 检查5个是否活着
    for (int i = 0; i < 5; ++i)
    {
        if (*word_to_ptr(hl))
        {
            found = 1;
            break;
        }
        切换至下一行
        hl.l += 11;
    }

    // 找到外星人的location返回
    *out = hl.l;
    return found;
}

// Handle alien shot explosion animation
// 处理外星人子弹爆炸动画

static void ShotBlowingUp()
{
    // xref 0644
    m.aShot.BlowCnt--;

    uint8_t blowcnt = m.aShot.BlowCnt;

    if (blowcnt == 3)
    {
        EraseAlienShot();
        m.aShot.Desc.spr = ptr_to_word(m.AlienShotExplodingSprite);

        // Offset the explision sprite from the shot by (-2,-2)
        // 将爆炸动画偏移(-2, -2)

        m.aShot.Desc.pos.x--;
        m.aShot.Desc.pos.x--;

        m.aShot.Desc.pos.y--;
        m.aShot.Desc.pos.y--;

        m.aShot.Desc.n = 6;

        // 绘制动画并检测碰撞
        DrawAlienShot();
        return;
    }

    if (blowcnt)
        return;
    // 擦除子弹的动画
    EraseAlienShot();
}

// Draw the active alien shot and do collision detection
// 绘制当前活动外星人的射击，并进行碰撞检测

static void DrawAlienShot()
{
    // xref 066c
    // 获取图像控制块
    SprDesc desc = ReadDesc(&m.aShot.Desc);
    // plot与处理碰撞
    DrawSprCollision(&desc);
    return;
}

// Erase the active alien shot
// 擦除外星人射击
static void EraseAlienShot()
{
    // xref 0675
    SprDesc desc = ReadDesc(&m.aShot.Desc);
    EraseShifted(&desc);
    return;
}

// Handles either the Squiggly shot or the Saucer, depending on the saucer timer.
// See ProcessSquigglyShot for squiggly shot logic.
// The bulk of this routine handles saucer movement, collision response, rendering
// and scoring.

/**
 * 依据 saucer timer 处理 squiggly shot 或 飞碟相关事件
 * 查看 ProcessSquigglyShot 函数了解 squiggly shot 的逻辑
 * 这里大部分逻辑在处理飞碟的移动、碰撞、绘制、得分
 *
 */

static void GameObj4(uint8_t* unused)
{
    // xref 0682
    // 同步控制
    if (m.shotSync != 2) return;

    // 如果saucer 还没开始
    if (m.saucerStart == 0)
    {
        ProcessSquigglyShot();
        return;
    }

    // 如果开火时间合格
    if (m.squShotData.StepCnt)
    {
        ProcessSquigglyShot();
        return;
    }

    // 屏幕上没有saucer活动
    if (!m.saucerActive)
    {
        // 外星人数量小于8
        if (m.numAliens < 8)
        {
            ProcessSquigglyShot();
            return;
        }

        // 更新状态为有saucer存货
        m.saucerActive = 1;
        DrawSaucer();
    }

    // carry 是判断是否Plot完成的变量
    uint8_t carry = CompXrToBeam(&m.saucerDesc.pos.x);

    // 未完成，返回
    if (!carry)
        return;

    // 如果saucer没有被击打，更新saucer的位置
    if (!m.saucerHit)
    {
        uint8_t x  = m.saucerDesc.pos.x;
        uint8_t dx = m.saucerDXr;

        m.saucerDesc.pos.x = x + dx;
        DrawSaucer();

        x = m.saucerDesc.pos.x;

        // check edges
        // 检查 saucer 是否在x轴bound，在的话移除
        if (x < xpix(8))
        {
            RemoveSaucer();
            return;
        }

        if (x >= xpix(193))
        {
            RemoveSaucer();
            return;
        }

        return;
    }

    // 关闭saucer的声音
    SoundBits3Off(0xfe); // turn off saucer sound

    // -----如果飞碟被打了----------- //

    m.saucerHitTime--;
    uint8_t timer = m.saucerHitTime;
    // 计数达到31时，开始爆炸
    if (timer == 31)
    {
        // xref 074b
        // Turn on the sound and draw the saucer explosion
        // 打开音效并且plot saucer爆炸
        uint8_t snd = m.soundPort5 | 16;
        m.soundPort5 = snd;
        SetSoundWithoutFleet(snd);

        m.saucerDesc.spr = ptr_to_word(m.SpriteSaucerExp);
        DrawSaucer();

        return;
    }

    // plot 得分动画
    if (timer == 24)
    {
        // xref 070c
        m.adjustScore = 1;

        // Get the score for the saucer which is set based on shots fired in GameObj0
        // 获取从 GameObj0 处得到的开火次数决定的得分
        uint8_t score = *(word_to_ptr(m.sauScore));

        // Find the index of the score in the table
        // 找到表中score的索引
        int i = 0;
        for (i = 0; i < 4; ++i)
        {
            if (m.SauScrValueTab[i] == score)
                break;
        }

        // Use it to find the matching LSB for the score text, and set it in saucerDesc
        // 找到得分文本的LSB来设定 saucerDesc
        m.saucerDesc.spr.l = m.SauScrStrTab[i];

        // Multiply the score by 16 (i.e. bcd shift left one digit), to get 50,100,150,300 in BCD
        // BCD 码左移，来得到 BCD 码的 50,100,150,300（得分×16）
        m.scoreDelta.u16 = score * 16;

        // Print the bonus score message, using pointer set in saucerDesc.spr above
        // 用上面设定好的描述符打印对应的得分信息
        SprDesc desc = GetSaucerInfo();
        PrintMessage(desc.sc, word_to_ptr(desc.spr), 3);

        return;
    }

    // xref 06e8
    // 未结束
    if (timer != 0)
        return;

    // 已结束，重设声音
    uint8_t snd = m.soundPort5 & 0xef;
    m.soundPort5 = snd;
    write_port(5, snd & 0x20);

    // 移除飞碟
    RemoveSaucer();
}

// Saucer cleanup tasks
// 移除飞碟
static void RemoveSaucer()
{
    // xref 06f9
    SprDesc desc = ReadDesc(&m.saucerDesc);
    ClearSmallSprite(ConvToScr(desc.pos), desc.n, 0);
    RestoreFromROM(u16_to_ptr(SAUCER_ADDR), SAUCER_SIZE);
    // 协程调度
    SoundBits3Off(0xfe);
}

// Grab a copy of the saucer sprite descriptor, and set it up
// for rendering before returning it.

/**翻译
 * 取得一个 飞碟精灵 的描述符并在返回前设定好
 */

static SprDesc GetSaucerInfo()
{
    // xref 0742
    SprDesc desc = ReadDesc(&m.saucerDesc);
    desc.sc = ConvToScr(desc.pos);
    return desc;
}

// Draw the player sprite
// 绘制玩家
static void DrawPlayer()
{
    // xref 036f
    SprDesc desc = ReadDesc(&m.playerDesc);

    desc.sc = ConvToScr(desc.pos);
    DrawSimpSprite(&desc);

    m.playerHeader.TimerExtra = 0;
}

// Draw the saucer sprite
// 绘制saucer
static void DrawSaucer()
{
    // xref 073c
    // xref 0742
    SprDesc desc = GetSaucerInfo();
    DrawSimpSprite(&desc);
}

// Wait for the player to press 1P or 2P, and then start the game
// with the appropriate flags.
// This loop is entered after the player has inserted a coin outside of game mode, see vblank_int()

/**翻译
 * 伴随着和是的标志位等待玩家按 1P 或者 2P 然后开始游戏
 * 这个循环在玩家于 游戏模式 外投币以后才会进入，具体看 vblank_int()
 */

static void WaitForStart()
{
    // xref 076e
    {
        // xref 1979
        // SuspendGameTasks
        // 任务挂起
        DsableGameTasks();
        DrawNumCredits();
        PrintCreditLabel();
    }

    ClearPlayField();
    // 打印出“PUSH”在屏幕上
    PrintMessage(xytosc(96, 152), m.MSG_PUSH, 4);

    //已经投币才会调用WFS（当前func）所以无需判断
    while (TRUE)
    {
        timeslice();

        //当硬币数>1 时
        if ((m.numCoins - 1) != 0)
        {
            // Enough credits for either 1P or 2P start
            // p1 p2 都可以重新开始
            PrintMessage(xytosc(32, 128), m.MSG_1_OR_2PLAYERS_BUTTON, 20);

            // Handle 1P or 2P
            uint8_t inp = read_port(1);

            // 处理p1
            if (inp & 0x2) NewGame(1, 0x98, 0);
            // 处理p2
            if (inp & 0x4) NewGame(0, 0x99, 0);

            continue;
        }

        // Only enough credits for 1P start
        // 只有p1可以重新开始时
        PrintMessage(xytosc(32, 128), m.MSG_ONLY_1PLAYER__BUTTON, 20);

        // Break if 1P start hit.
        // 开始时选择的模式是 1P 即只有一个玩家
        if (read_port(1) & 0x4)
            break;
    }

    NewGame(0, 0x99, 0);
}

// Starts a new game, and runs the game loop.
// This routine is entered via either the WaitForStart() loop after inserting coins
// outside of game mode, or is entered after the player dies via player_death()
// to continue the game.
// is2p - set to true if 2P was pressed
// cost - credits to deduct in bcd (0x99=1, 0x98=2 credits)
// skip - used to skip certain parts of initialization, used for continue

/**翻译
 * 启动一个新游戏，并运行游戏循环
 * 这个函数会在投币后由 WaitForStart() 进入，或玩家死后由 player_death() 进入来继续游戏
 * is2p - 如果2p按下设为真
 * cost - 扣除硬币数的 bcd 编码(0x99=1, 0x98=2 credits)
 * skip - 用于略过一部分初始化操作，用于继续游戏
 */

static void NewGame(uint8_t is2p, uint8_t cost, int skip)
{
    // xref 0798
    // xref 079b
    int flags = ~skip;

    if (flags & INIT)
    {
        m.twoPlayers = is2p;

        {
            uint8_t unused_carry = 0;
            m.numCoins = bcd_add(m.numCoins, cost, &unused_carry);
        }

        DrawNumCredits();

        m.P1Scor.u16 = 0;
        m.P2Scor.u16 = 0;

        PrintP1Score();
        PrintP2Score();

        DsableGameTasks();

        m.gameMode = 1;

        // 两个玩家都活
        m.playerStates.u16 = 0x0101;  // Both players alive
        // 两个玩家都可以得分
        m.playerExtras.u16 = 0x0101;  // Both players bonus available

        DrawStatus();
        DrawShieldP1();
        DrawShieldP2();

        uint8_t ships = GetShipsPerCred();

        m.p1ShipsRem = ships;
        m.p2ShipsRem = ships;

        InitAlienRacks();

        m.p1RackCnt = 0;
        m.p2RackCnt = 0;

        InitAliensP1();
        InitAliensP2();

        m.p1RefAlienPos = xytopix(24, 120);
        m.p2RefAlienPos = xytopix(24, 120);

        CopyRAMMirror();
        RemoveShip();
    }

    if (flags & PROMPT)
    {
        PromptPlayer();
        ClearPlayField();
        m.isrSplashTask = 0;
    }

    // xref 0804 top of new game loop
    // 新游戏循环顶
    while (TRUE)
    {
        if (flags & SHIELDS)
        {
            DrawBottomLine();

            // 判断当前是玩家player1 还是player2
            if (m.playerDataMSB & 0x1)
            {
                // 识别为player1 的操作
                RestoreShieldsP1();
            }
            else
            {
                // 识别为player2的操作
                RestoreShieldsP2();
                // 默认单人游戏时只有player1 所以识别到player2时说明双player游戏
                //所以画底线
                DrawBottomLine();
            }

            // xref 0814
            InitRack();
        }
        else
        {
            flags |= SHIELDS; // don't skip next time
        }

        EnableGameTasks();
        SoundBits3On(0x20);

        // xref 081f game loop

        while (TRUE)
        {
            PlrFireOrDemo();
            PlyrShotAndBump();
            CountAliens();

            AdjustScore();

            if (m.numAliens == 0)
            {
                HandleEndOfTurn();
                break;
            }

            AShotReloadRate();
            CheckAndHandleExtraShipAward();
            SpeedShots();
            ShotSound();

            if (! IsPlayerAlive())
                SoundBits3On(0x04); // Turn on player hit sound

            uint8_t w = FleetDelayExShip();
            write_port(6, w);                   // Feed the watchdog
            CtrlSaucerSound();
        }
    }
}

// Get reference alien velocity, position and pointer for the current player
// 获取当前玩家外星人的参考速度、位置和指针
static uint8_t* GetRefAlienInfo(uint8_t *dxr, Word *pos)
{
    // xref 0878
    // 参考速度
    *dxr = m.refAlienDelta.x;
    // 参考位置，由*pos 直接指向当前位置
    *pos = m.refAlienPos;
    return GetAlRefPtr();
}

// Get reference alien pointer for the current player
// 获取指针指向玩家的参考外星飞船数据
static uint8_t* GetAlRefPtr()
{
    // xref 0886
    return (m.playerDataMSB & 0x1) ? &m.p1RefAlienPos.l :
                                                &m.p2RefAlienPos.l;
}

// Print "PLAY PLAYER<n>" and flash the score at 15 hz for 3 seconds
// This is done upon starting a NewGame in 1P mode, or at the start
// of every turn in 2P mode.

/**翻译
 * 打印 "PLAY PLAYER<n>" 并且以 15hz 的频率闪烁得分 3 秒
 * 这是在以1P模式开始新游戏时完成的，或者在以2P模式开始每回合时完成的。
 */

static void PromptPlayer()
{
    // xref 088d

    // "PLAY PLAYER<1>"
    PrintMessage(xytosc(56,136), m.MSG_PLAY_PLAYER_1_, 14);

    // replace <1> with <2>
    // 使用<2>代替<1>，表示当前有两位玩家
    if ((m.playerDataMSB & 0x1) == 0)
        DrawChar(xytosc(152, 136), 0x1c);
        // 三秒延迟
    m.isrDelay = 176; // 3 sec delay

    // xref 08a9
    while (TRUE)
    {
        timeslice();
        uint8_t isrtick = m.isrDelay;

        if (isrtick == 0)
            return;

        // Flash player score every 4 isrs
        if (isrtick & 0x4)
        {
            // xref 08bc
            Word sc = (m.playerDataMSB & 0x1) ? xytosc(24,224) : xytosc(168,224);
            ClearSmallSprite(sc, 32, 0);
            continue;
        }

        DrawScore(GetScoreDescriptor());
    }
}

// DIP5 and DIP3 control the number of extra lives the player starts with.
// DIP5 and DIP3 are wired into bits 1 and 0 of port 2 respectively.
//
// When read together as a two digit binary number, this is meant to be
// interpreted as the number of extra lives above the default of 3 that
// the player gets.
//
// 0 0 - 3 lives
// 0 1 - 4 lives
// 1 0 - 5 lives
// 1 1 - 6 lives

/**翻译
 * DIP5 和 DIP3 控制角色开始游戏时的额外生命值
 * DIP5 和 DIP3 分别链接 port 2 的 1 和 0 号比特位
 * 
 * 当作为一个2位二进制数读取的时候，代表玩家除默认生命值外额外获得的生命数
 * 
 * 0 0 - 3 条命
 * 0 1 - 4 条命
 * 1 0 - 5 条命
 * 1 1 - 6 条命
 */

static uint8_t GetShipsPerCred()
{
    // xref 08d1
    //默认有3命
    return (read_port(2) & (DIP5_SHIPS2 | DIP3_SHIPS1)) + 3;
}

// Increase alien shot speed when there are less than nine aliens on screen
// 当前屏幕上alien数目小于 9 个,加快alien射击速度
static void SpeedShots()
{
    // xref 08d8
    //大于9跳过
    if (m.numAliens >= 9)
        return;
    //小于9射速从-4 到 -5
    m.alienShotDelta = -5; // from -4 to -5
}

// Prints a text message (msg, n) on the screen at pos
// Used to print all the splash screen text, and other game messages

/**翻译
 * 在屏幕的 pos 位置上打印 message(msg, n)
 * 用于打印所有启动界面文本和其他游戏信息
 */

static void PrintMessage(Word sc, uint8_t* msg, size_t n)
{
    // xref 08f3
    // DebugMessage(sc, msg, n);
    // 打印信息
    for (size_t i = 0; i < n; ++i)
    {
        uint8_t c = msg[i];
        sc = DrawChar(sc, c);
    }
}

// Draw a text character c on the screen at pos
// Used by PrintMessage()

/**翻译
 * 在屏幕的 pos 位置绘制一个字符 c
 * 由 PrintMessage() 使用
 */

static Word DrawChar(Word sc, uint8_t c)
{
    // xref 08ff
    SprDesc desc;
    desc.sc = sc;

     /**
     * 0x1e00 开始是字体表，具体在 
     * https://computerarcheology.com/Arcade/SpaceInvaders/Code.html
     * 见 Text Character Sprites 部分
     * 
     * 这里就是对应编号为 c 的字符
     */

    desc.spr = u16_to_word(0x1e00 + c*8);
    desc.n = 8;

    return DrawSimpSprite(&desc);
}

// Timing logic that controls when the saucer appears (Every 25.6 secs)
// Called via vblank_int()

/**
 * 控制飞碟出现的时间（每 25.6s)
 * 被 vblank_int() 调用
 */

static void TimeToSaucer()
{
    // xref 0913
    // No ticking until alien rack has dropped down a bit
    // 在alien下降之前不要计时
    if (m.refAlienPos.y >= 120)
        return;

    uint16_t timer = m.saucerTimer.u16;

    if (timer == 0)
    {
        // 将游戏时间重置为 1536 游戏循环（25.6s)
        timer         = 0x600; // reset timer to 1536 game loops (25.6s)
        m.saucerStart = 1;
    }

    m.saucerTimer.u16 = timer - 1;
}

// Get number of lives for the current player
// 获取船的数目，也就是当前玩家的命数
static uint8_t GetNumberOfShips(uint8_t* *ptr)
{
    // xref 092e

    /**
     * 玩家 1 的信息存储在模拟内存的 0X2100-0X21FF
     * 玩家 2 的信息存储在模拟内存的 0X2200-0X22FF
     * 
     * 调用的时候并不知道当前是哪个玩家，所以使用 GetPlayerDataPtr 
     * 获取玩家数据内存地址
     * 
     * 其中 0X2xFF 位置存储的是当前飞船剩余生命数
     * 所以在指针末尾加上 0xff
     */

    *ptr = (GetPlayerDataPtr() + 0xff);
    return *(*ptr);
}

// Award the one and only bonus life if the player's score is high enough,
// and fix up the lives indicators to reflect that.

/**翻译
 * 在玩家分数足够高的时候奖励一条命并修改生命指示来显示
 */

static void CheckAndHandleExtraShipAward()
{
    // xref 0935
    if (*(CurPlyAlive() - 2) == 0)
        return;

    // Bonus dip bit - award at 1000 or 1500
    uint8_t b = (read_port(2) & DIP6_BONUS) ? 0x10 : 0x15;
    uint8_t score_msb = *(GetScoreDescriptor() + 1);

    // score not high enough for bonus yet
    // 得分不够bonus
    if (score_msb < b)
        return;

    uint8_t* nships_ptr;
    GetNumberOfShips(&nships_ptr);

    // Award the bonus life
    // 得分够了，奖励1 ship 即一条命
    (*nships_ptr)++;

    int nships = *nships_ptr;

    SprDesc desc;
    desc.sc = xytosc(8 + 16 * nships, 8);
    desc.spr = ptr_to_word(m.PLAYER_SPRITES);
    desc.n = 16;

    DrawSimpSprite(&desc);

    PrintNumShips(nships+1);

    // 设定已获得额外生命标志位
    *(CurPlyAlive() - 2) = 0;    // Flag extra ship has been awarded
    // 处理额外生命奖励音乐
    m.extraHold = 0xff;          // Handle Extra-ship sound
    SoundBits3On(0x10);
}

// Lookup score for alien based on the given row
// 根据给定的行查询外星人的分数
static uint8_t* AlienScoreValue(uint8_t row)
{
    // xref 097c
    uint8_t si = 0;

    if      (row < 2) si = 0;
    else if (row < 4) si = 1;
    else              si = 2;

    return  (m.AlienScores + si);
}

// Add the score delta to the current player's score, and draw it.
// Called as part of the game loop.
//
// scoreDelta is modified in two places:
//     PlayerShotHit() upon killing an alien    (main thread)
//     GameObj4()      upon hitting the saucer  (either vblank or mid depending on saucer x pos)

/**翻译
 * 添加 分数增量 到当前玩家的分数上，并绘制
 * 作为 游戏循环 的一部分被调用
 * socreDelta 在两个地方被修改：
 * PlayerShotHit() - 在击杀外星人被杀死的时候（主线程）
 * GameObj4()      - 在击中飞碟的时候（依据 x 位置在 vblank 或 mid）
 */

static void AdjustScore()
{
    // xref 0988
    uint8_t* sptr = GetScoreDescriptor();
    if (m.adjustScore == 0) return;

    m.adjustScore = 0;

    Word adj = m.scoreDelta;
    uint8_t carry = 0;

    Word score;

    score.l = *(sptr);
    score.l = bcd_add(score.l, adj.l, &carry);
    *sptr = score.l;

    score.h = *(sptr+1);
    score.h = bcd_add(score.h, adj.h, &carry);
    *(sptr+1) = score.h;

    Word sc;
    sc.l = *(sptr+2);
    sc.h = *(sptr+3);

    Print4Digits(sc, score);
}

// Print 4 digits using the bcd values in val.h and val.l
// Called via DrawScore and AdjustScore

/**翻译
 * 用 BCD 码的 val.h 和 val.l 打印 4 个数字
 * 从 DrawScore 和 AdjustScore 调用
 */

static void Print4Digits(Word sc, Word val)
{
    // xref 09ad
    sc = DrawHexByte(sc, val.h);
    sc = DrawHexByte(sc, val.l);
}

// Draw the the hi and lo nibble of the bcd value in c at sc
// 在 sc 中绘制 BCD 码的 hi 和 lo 半字节
static Word DrawHexByte(Word sc, uint8_t c)
{
    // xref 09b2
    sc = DrawHexByteSub(sc, c >> 4);
    sc = DrawHexByteSub(sc, c & 0xf);
    return sc;
}

// Draw the digit in c at sc
// 在 sc 的位置绘制数字 c
static Word DrawHexByteSub(Word sc, uint8_t c)
{
    // xref 09c5
    // 游戏字母表中 数字 0 的起始位置是 0x1A
    return DrawChar(sc, c + 0x1a);
}

// Return a pointer to the score info for the current player
// 返回指向当前玩家得分信息的指针
static uint8_t* GetScoreDescriptor()
{
    // xref 09ca
    return (m.playerDataMSB & 0x1) ? &m.P1Scor.l : &m.P2Scor.l;
}

// Clear the play field in the center of the screen.
// Horizontally, the play field is the full width of the screen.
// Vertically, the play field is the area above the lives and credits (16 pixels)
// and below the scores (32 pixels).

/**翻译
 * 清理游戏区域中间的内容
 * 在水平方向上清理所有区域
 * 在垂直方向上清理 lives 、 credits 以上的区域（16 像素）
 * 以及得分以下的区域（32 像素)
 */

static void ClearPlayField()
{
    // xref 09d6
    uint8_t* screen = m.vram;

    for (int x = 0; x < 224; ++x)
    {
        screen += 2;

        for (int b = 0; b < 26; ++b)
            *screen++ = 0;

        screen += 4;
    }
}

// Called from the game loop when the player has killed all aliens in the rack.
// 当玩家杀死了所有外星飞船的时候由游戏循环调用
static void HandleEndOfTurn()
{
    // xref 09ef
    HandleEndOfTurnSub(); // wait for player to finish dying if necessary

    m.gameTasksRunning = 0;
    ClearPlayField();

    uint8_t pnum = m.playerDataMSB;

    CopyRAMMirror();

    m.playerDataMSB = pnum;
    // 冗余加载
    pnum = m.playerDataMSB; // redundant load

    uint8_t rack_cnt = 0;

    uint8_t* rcptr = u16_to_ptr(pnum << 8 | 0xfe);
    rack_cnt = (*rcptr % 8) + 1;
    *rcptr = rack_cnt;

    // Starting Y coord for rack for new level
    // 启动新级别飞船的 Y 坐标
    uint8_t y = m.AlienStartTable[rack_cnt-1];

    uint8_t* refy = u16_to_ptr(pnum << 8 | 0xfc);
    uint8_t* refx = refy + 1;

    *refy = y;
    *refx = 56;

    // 识别pnum代表的是p1还是p2
    if (!(pnum & 0x1))
    {
        // 是p1
        // 用第一个声音开启舰队
        m.soundPort5 = 0x21;  // start fleet with first sound
        DrawShieldP2();
        InitAliensP2();
        //给p2装上盾
        //对p2进行外星人化处理
    }
    else
    {
        // 是p2
        DrawShieldP1();
        InitAliensP1();
        //给p1装上盾
        //对p1进行外星人化处理
    }
}

// Called at start of HandleEndOfTurnSub() to handle the
// case of the player dying at the end of turn.
// (i.e. last alien and player both kill each other)

/**翻译
 * 在 HandleEndOfTurnSub() 的开始被调用，处理玩家在最后死亡的情况
 * （比如最后一个外星人和玩家同时杀死了对方）
 */

static void HandleEndOfTurnSub()
{
    // xref 0a3c

    if (IsPlayerAlive())
    {
        m.isrDelay = 48; // wait up to 3/4 of a sec

        do
        {
            // xref 0a47
            timeslice(); // spin
            // 延迟为0 返回
            if (m.isrDelay == 0)
                return;

        } while (IsPlayerAlive());// 只要存活就一直在循环中
    }

    // If player is not alive, wait for resurrection
    // 如果玩家阵亡等待复活
    while (!IsPlayerAlive())
    {
        // xref 0a52
        timeslice(); // spin
    }
}

// Returns 1 if player is alive, 0 otherwise
// 如果玩家存活返回 1，否则返回 0
static uint8_t IsPlayerAlive()
{
    // xref 0a59
    return m.playerAlive == 0xff;
}

// Called as part of the player bullet collision response in PlayerShotHit()
// when the player bullet kills an alien.

/**翻译
 * 当玩家的子弹击杀外星人的时候作为 PlayerShotHit() 的一部分被调用
 */

static void ScoreForAlien(uint8_t row)
{
    // xref 0a5f
    if (!m.gameMode)
        return;

    SoundBits3On(0x08);

    uint8_t score = *(AlienScoreValue(row));

    m.scoreDelta.h = 0;
    m.scoreDelta.l = score;
    m.adjustScore = 1;
}

// Companion routine to SplashSprite
// Called from the main thread to initiate and wait for splash animations (CCOIN / PLAy).

/**翻译
 * 用于主界面动画播放
 * 在主线程
 */

static void Animate()
{
    // xref 0a80
    // Directs ISRSplTasks() (in vblank_int()) to call SplashSprite()
    
    /**
     * 让协程在切换到 int_ctx 且执行 vblank_int() 时能在 ISRSplTasks() 函数中执行 SplashSprite()
     * 看着绕口的话就去 run_int_ctx 里头看函数调用，看一遍就懂了
     */
    
    m.isrSplashTask = 2;

    // Spin until sprite in animation reaches its target position
    do {
        // xref 0a85
        
        /**
         * 如果是真实硬件用于重置看门狗
         * 这里只起到调用 timeslice() 切换协程的作用
         */

        write_port(6, 2); // feed watchdog and spin
    }
    while (!m.splashReached); // 当前动画播放完毕标志位，没播放完成就一直阻塞在这里

    // Directs ISRSplTasks() to do nothing
    m.isrSplashTask = 0;
}

// Prints the animated text messages (such as "PLAY" "SPACE INVADERS"),
// by drawing the characters that make up the message with a short
// delay between them. (7 frames per character)

/**翻译
 * 绘制动态文字信息（比如 “PLAY” “SPACE INVADERS”)
 * 用短字符画出组成信息的字符，字符之间有一个很短的延迟（每个字符间 7 帧）
 */

static Word PrintMessageDel(Word sc, uint8_t* str, uint8_t n)
{
    // xref 0a93
    // DebugMessage(sc, str, n);

    for (int i = 0; i < n; ++i)
    {
        // 显示一个字符并且 sc 指针往前移动
        sc = DrawChar(sc, str[i]);
        // 延迟 7 帧
        m.isrDelay = 7;
        while (m.isrDelay != 1) { timeslice(); } // spin
    }
    // 返回屏幕指针（最后一个字符的下一个位置）
    return sc;
}

// Need for the shooting C in CCOIN animation.
// Initiated in AnimateShootingSplashAlien(), and called
// via ISRSplTasks() during vblank_int()

/**翻译
 * 用于在射击 CCOIN 中 字母 C 动画的时候
 * 在 AnimateShootingSplashAlien() 中初始化
 * 在 vblank_int() 中由 ISRSplTasks() 调用
 */

static void SplashSquiggly()
{
    // xref 0aab
    // this works because this is the last game object.
    // 最后一个游戏对象
    RunGameObjs(u16_to_ptr(SQUIGGLY_SHOT_ADDR));
}

// Wait for approximately one second. (64 vblanks).
// 等待大概 1s（64 个vblanks)
static void OneSecDelay()
{
    // xref 0ab1
    WaitOnDelay(64);
}

// Wait for approximately two seconds. (128 vblanks).
// 等待大概 2s（128 个vblanks）
static void TwoSecDelay()
{
    // xref 0ab6
    WaitOnDelay(128);
}

// Runs the game objects in demo mode to attract players.
// Initiated from the main thread and called
// via ISRSplTasks() during vblank_int()

/**
 * 在演示模式运行 game objects 来吸引玩家
 * 主线程中初始化
 * 在 vblank_int() 中由ISRSplTasks()调用
 */

static void SplashDemo()
{
    // xref 0abb
    // xref 0072
    m.shotSync = m.rolShotHeader.TimerExtra;

    DrawAlien();
    RunGameObjs(u16_to_ptr(PLAYER_ADDR)); // incl player
    TimeToSaucer();
}

// Runs the appropriate splash screen task (from vblank_int())
// 运行 isrSplashTask 标志位对应演示动画（被 vblank_int() 调用)
static void ISRSplTasks()
{
    // xref 0abf
    switch (m.isrSplashTask)
    {
        case 1: SplashDemo(); break;         // Attract players with game demo
        case 2: SplashSprite(); break;       // Moves a sprite to a target location for an animation
        case 4: SplashSquiggly(); break;     // Run an alien shot for an animation ( CCOIN )
    }
}

// Print an animated message in the center of the screen.
// 在屏幕中心打印动态信息
static void MessageToCenterOfScreen(uint8_t* str)
{
    // xref 0acf
    //(56,160)是屏幕中心的坐标
    PrintMessageDel(xytosc(56,160), str, 0x0f);
}

// Wait for n vblank interrupts to occur, using m.isrDelay
// 利用 m.isrDelay 等待 n 个 vblank 中断发生
static void WaitOnDelay(uint8_t n)
{
    // xref 0ad7
    // Wait on ISR counter to reach 0
    // 等待ISR计数器归0
    m.isrDelay = n;

    while (m.isrDelay != 0)  { timeslice(); } // spin
}

// Copy src into the splash animation structure.
// The four animations copied this way are
//
// (for PLAy animation)
//
// 0x1a95 - Move alien left to grab y
// 0x1bb0 - Move alien (with y) to right edge
// 0x1fc9 - Bring alien back (with Y) to message
//
// (for CCOIN animation)
//
// 0x1fd5 - Move alien to point above extra 'C'

/**翻译
 * 将 src 中的内容拷贝到模拟 RAM 中的 splash animation 结构体
 * 有四个动画经由这种方式拷贝，它们分别是
 * 
 * （“PLAy” 字母动画）
 * 具体内容请见 https://computerarcheology.com/Arcade/SpaceInvaders/Code.html 中 Alien Pulling Upside Down 'Y'
 * 0x1a95 - 外星人来左边抓倒立的 Y
 * 0x1bb0 - 外星人带着倒立的 Y 从右边离开
 * 0x1fc9 - 外星人带着正确的 Y 放回到 PLAy 上
 * 
 * （CCOIN 动画）
 * 0x1fd5 - 外星人移动到 CCOIN 的第一个字母 ‘C’ 头上并攻击消除第一个 ‘C’
 */

static void IniSplashAni(uint8_t* src)
{
    // xref 0ae2
    BlockCopy(&m.splashAnForm, src, 12);
}

// Called during (splash screens) to do some miscellaneous tasks
//   a) Player shot collision response
//   b) Detecting and handling the alien rack bumping the screen edges
//   c) Checking for TAITO COP input sequence

/**翻译
 * 用于做一些杂事，在初始屏幕环节被调用
 *  a) 玩家子弹碰撞反馈
 *  b) 检测并处理外星飞船碰到屏幕（改变其移动方向）
 *  c) 检查 "TAITO COP" 彩蛋触发
 */

static uint8_t CheckPlyrShotAndBump()
{
    // xref 0bf1
    // 判定玩家射击和alien是否到boundry
    PlyrShotAndBump();
    // 检查是否触发彩蛋
    CheckHiddenMes();
    return 0xff;
}

// Erases a sprite by clearing the four bytes it
// could possibly be in.

/**翻译
 * 擦除给定范围内的图像
 */

static void EraseSimpleSprite(Word pos, uint8_t n)
{
    // xref 1424
    Word sc = CnvtPixNumber(pos);

    for (int i = 0; i < n; ++i, sc.u16 += xysc(1, 0))
    {
        uint8_t* screen = word_to_ptr(sc);

        *screen++ = 0;
        *screen++ = 0;
    }
}

// Draws a non shifted sprite from desc->spr horizontally
// across the screen at desc->pos for desc->n bytes.
// Each byte of the sprite is a vertical 8 pixel strip

/**翻译
 * 不加变换且水平地绘制 desc->spr 指向的精灵
 * 在屏幕上的 desc->pos 处绘制 desc->n bytes
 * 每个精灵的 byte 在竖直方向上占 8 像素
 */

static Word DrawSimpSprite(SprDesc *desc)
{
    // xref 1439
    // sc 对应到虚拟硬件中的地址
    uint8_t* screen = word_to_ptr(desc->sc);
    // spr 在虚拟硬件中的地址
    uint8_t* sprite = word_to_ptr(desc->spr);

    // 将需要显示的信息写到虚拟硬件的ROM当中
    for (size_t i = 0; i < desc->n; ++i, screen += xysc(1, 0))
        *screen = sprite[i];

    return ptr_to_word(screen);
}

// Using pixnum, set the shift count on the hardware shifter
// and return the screen coordinates for rendering

/**翻译
 * 用像素编号设定硬件偏移寄存器的计数器，并为渲染返回一个 sc 坐标
 * 就是将 像素坐标 转为 sc 地址
 */

static Word CnvtPixNumber(Word pos)
{
    // xref 1474
    // 设定硬件偏移（模拟街机硬件），保留低2位
    write_port(2, (pos.u16 & 0xff) & 0x07);
    return ConvToScr(pos);
}

// Draw a shifted sprite to the screen, blending with screen contents
// 在屏幕上绘制一个移动过的sprite，以与屏幕上的内容混合的方式绘制
static void DrawShiftedSprite(struct SprDesc *desc)
{
    // xref 1400
    DrawSpriteGeneric(desc, OP_BLEND);
}

// Erase a shifted sprite from the screen, zeroing screen contents
// 擦除屏幕上一个移动过的sprited，清除指定屏幕内容
static void EraseShifted(struct SprDesc *desc)
{
    // xref 1452
    DrawSpriteGeneric(desc, OP_ERASE);
}

// Draw a shifted sprite to the screen, blending with screen contents,
// and detect if drawn sprite collided with existing pixels

/**费用
 * 在屏幕上绘制一个移动的精灵，与屏幕上的内容混合
 * 并且检测当前精灵是否和存在的像素碰撞
 */

static void DrawSprCollision(struct SprDesc *desc)
{
    // xref 1491
    DrawSpriteGeneric(desc, OP_COLLIDE);
}

// Draw a shifted sprite to the screen, overwriting screen contents.
// 以覆盖的方式绘制一个移动的精灵在屏幕上
// 在绘制的使用使用了移位寄存器转换像素坐标和 sc 坐标
static void DrawSprite(struct SprDesc *desc)
{
    // xref 15d3
    DrawSpriteGeneric(desc, OP_BLIT);
}

// Generic sprite drawing routine for shifted spries
//   desc->spr      - source pointer
//   desc->pixnum   - pixel position to draw at
//   desc->n        - width
//   op             - erase | blit | blend | collide

/**翻译
 * 绘制移动精灵的通用函数
 * 
 * desc->spr    - 源的指针
 * desc->pixnum - 像素绘制位置
 * desc->n      - 宽度
 * op           - 擦除 | 块传送 | 混合 | 相撞
 * 
 */

static void DrawSpriteGeneric(struct SprDesc *desc, int op)
{
    // 将像素坐标 转为 sc 地址
    Word sc = CnvtPixNumber(desc->pos);
    // 获取sprite内存地址
    uint8_t* sprite = word_to_ptr(desc->spr);

    // 重置碰撞标志
    if (op == OP_COLLIDE)
        m.collision = 0;

    for (int i = 0; i < desc->n; ++i, sc.u16 += xysc(1,0))
    {
        // 获取ROM地址
        uint8_t* screen = word_to_ptr(sc);

        // 偏移寄存器 register
        uint8_t shift_in[2];
        shift_in[0] = sprite[i];
        shift_in[1] = 0;

        for (int j = 0; j < 2; ++j)
        {
            // 数据写入移位寄存器，偏移多少由 pos 的右边三位指定
            // 主要影响竖直方向上动画播放，用于类似子弹在垂直方向上的动画
            write_port(4, shift_in[j]);             // write into shift reg
            uint8_t shifted = read_port(3);  // get the shifted pixels (shift based on pix num)


            // 碰撞检测
            if (op == OP_COLLIDE && (shifted & *screen))
                m.collision = 1;// 检测到碰撞，设置标志位

            if (op == OP_COLLIDE || op == OP_BLEND)
                *screen = shifted | *screen;
            else if (op == OP_BLIT)
                *screen = shifted;
            else if (op == OP_ERASE)
                *screen = (shifted ^ 0xff) & *screen;
            // 转移到下一 sc 坐标，依次看所有坐标碰撞情况
            screen++;
        }
    }
}

// Repeat (width n) the pixel strip in byte v horizontally across the screen
// 将屏幕上水平宽度为 n 的内容用 v 覆盖
static Word ClearSmallSprite(Word sc, uint8_t n, uint8_t v)
{
    // xref 14cb
    for (int i = 0; i < n; ++i, sc.u16 += xysc(1,0))
        *(word_to_ptr(sc)) = v;

    return sc;
}

// Player bullet collision response
// 玩家子弹碰撞响应
static void PlayerShotHit()
{
    // xref 14d8
    uint8_t status = m.plyrShotStatus;

    // if alien explosion state, bail
    // 如果外星飞船爆炸阶段，返回
    if (status == 5)
        return;

    // if not normal movement, bail
    // 如果不是普通移动，立即返回
    if (status != 2)
        return;


    //后便只剩下status == 2 即普通移动情况


    // Get the Y coord
    // 获取子弹 Y 坐标
    uint8_t shoty = m.playerShotDesc.pos.y;

    if (shoty >= 216)
    {
        // missed and hit top of screen
        // 未击中，撞击屏幕顶端，子弹自动爆炸（设置标志位为 3）
        m.plyrShotStatus = 3;
        m.alienIsExploding = 0;
        // 协程调度
        SoundBits3Off(0xf7);
        return;
    }

    // xref 14ea
    // 检测是否有外星飞船正在爆炸
    if (!m.alienIsExploding)
        return;

    //没有则继续后续代码
    if (shoty >= 206)
    {
        // hit the saucer
        // 击中飞碟
        // xref 1579
        // 设定飞碟击中
        m.saucerHit = 1;
        // 设定射击状态标志位为外形飞船爆炸
        m.plyrShotStatus = 4;

        // xref 154a
        // 设定此时外星人未爆炸
        m.alienIsExploding = 0;
        // 协程调度
        SoundBits3Off(0xf7);
        return;
    }

    shoty += 6;

    {
        uint8_t refy = m.refAlienPos.y;

        // refy can wrap around, if the topmost alien row gets near the bottom of the screen
        // in usual play, refy will be < 144.
        // 如果最顶端的外星人接近屏幕底端 refy 可以环绕
        // 正常游玩时，refy 会 < 144
        if ((refy < 144) && (refy >= shoty))
        {
            // hit the shields
            // 击中盾牌，更新标志位
            m.plyrShotStatus = 3;
            m.alienIsExploding = 0;
            // 协程调度
            SoundBits3Off(0xf7);
            return;
        }
    }

    // xref 1504
    // Get here if player shot hits an alien or an alien shot.
    // There is a subtle bug here, see CodeBug1 in CA

    /**翻译
     *  如果玩家击中外星飞船或者外星飞船的子弹，程序会到达这里
     * 这里有一个很微妙的错误，见
    */ https://computerarcheology.com/Arcade/SpaceInvaders/Code.html#CodeBug1
    
    // 找对应的行
    Word res = FindRow(shoty);

    uint8_t row = res.h;
    uint8_t ay = res.l;
    // 找对应的列
    res = FindColumn(m.playerShotDesc.pos.x);

    uint8_t col = res.h;
    uint8_t ax  = res.l;

    m.expAlien.pos = u8_u8_to_word(ax, ay);
    m.plyrShotStatus = 5;

    uint8_t* alienptr = GetAlienStatPtr(row, col);

    if (*alienptr == 0)
    {
        // If alien is dead, then the player shot must have hit an alien shot
        // 如果当前alien早就死了，那么玩家必定击中的是alien的子弹
        m.plyrShotStatus = 3;
        m.alienIsExploding = 0;
        SoundBits3Off(0xf7);
        return;
    }

    // Kill the alien
    // 击杀alien
    *alienptr = 0;
    ScoreForAlien(row);
    SprDesc desc = ReadDesc(&m.expAlien);
    DrawSprite(&desc);
    m.expAlienTimer = 16;
}

// Counts the number of 16s between *v and target.
// This is roughly (tgt - *v) / 16.

/**翻译
 * 计算 tgt 和 *V 之间有多少个 16 像素
 * 外型人图片宽度是 16 个像素，所以数出来有多少个 16 像素就知道在第几列（行）了
 */

static uint8_t Cnt16s(uint8_t *v, uint8_t tgt)
{
    // xref 1554
    uint8_t n = 0;

    if ((*v) >= tgt)
    {
        do
        {
            // wrap ref
            n++;
            (*v) += 16;

        } while ((*v) & 0x80);
    }

    while ((*v) < tgt)
    {
        (*v) += 16;
        n++;
    }

    return n;
}

// Find alien row given y pos
// 通过给定的 Y 找到alien所属的行
static Word FindRow(uint8_t y)
{
    // xref 1562
    uint8_t ry    = m.refAlienPos.y;
    uint8_t rnum  = Cnt16s(&ry, y) - 1;
    uint8_t coord = (ry - 16);

    return u8_u8_to_word(rnum, coord);
}

// Find alien column given x pos
// 用给定的 x 坐标找到 alien所属的列
static Word FindColumn(uint8_t x)
{
    // xref 156f
    uint8_t rx    = m.refAlienPos.x;
    // 对应的colomn
    uint8_t cnum  = Cnt16s(&rx, x);
    uint8_t coord = (rx - 16);

    return u8_u8_to_word(cnum, coord);
}

// Return a pointer to the alien status for the current player
// given the row and column of the alien

/**翻译
 * 通过给定的行列返回一个指向当前玩家外星人状态的指针
 */

static uint8_t* GetAlienStatPtr(uint8_t row, uint8_t col)
{
    // xref 1581
    // row is 0 based
    // col is 1 based
    // 行从0开始
    // 列从1开始
    uint8_t idx = row * 11 + (col - 1);
    return u16_to_ptr(m.playerDataMSB << 8 | idx);
}

// Change alien deltaX and deltaY when alien rack bumps edges
// 当alien碰到屏幕edge的时候更改其移动方向
static void RackBump()
{
    // xref 1597
    // Change alien deltaX and deltaY when rack bumps edges
    // 当碰壁时更改alien移动方向
    uint8_t dx = 0;
    uint8_t dir = 0;

    if (m.rackDirection == 0)
    {
        // xref 159e check right edge
        // 如果还没有碰到右边界
        if (!RackBumpEdge(xytosc(213,32)))
            return;

        dx = -2;
        dir = 1;  // rack now moving left
    }
    else
    {
        // check left edge
        // 检查左边界
        if (!RackBumpEdge(xytosc(9,32)))
            return;

        // rack now moving right
        // inline 18f1
        // 如果只剩下一个alien加快移动速度
        dx = m.numAliens == 1 ? 3 : 2;  // go faster if only one alien remaining
        dir = 0;                        // rack now moving right
    }

    // 更新对应的标志位
    m.rackDirection = dir;
    m.refAlienDelta.x = dx;
    m.refAlienDelta.y = m.rackDownDelta;
}

// Check 23 bytes vertically up from sc for pixels.
// Used by RackBump to detect whether alien rack is hitting the edges of the play area.

/**翻译
 * 从 sc 垂直向上检查 23 字节像素
 * 被 RackBump 用于检查外星飞船是否碰到墙壁
 */

static uint8_t RackBumpEdge(Word sc)
{
    // xref 15c5
    uint8_t* screen = word_to_ptr(sc);

    for (int i = 0; i < 23; ++i)
    {
        timeslice();

        // found some pixels
        // 如果这一列上存在像素就判断有alien触界
        if (*screen++)
            return 1;
    }

    return 0;
}

// Count the number of live aliens for the current player
// 计算当前玩家存活的alien数目
static void CountAliens()
{
    // xref 15f3
    uint8_t* iter = GetPlayerDataPtr(); // Get active player descriptor
    uint8_t n = 0;

    for (int i = 0; i < 55; ++i)
    {
        timeslice();

        if (*iter++ != 0)
            ++n;
    }

    m.numAliens = n;

    if (n != 1)
        return;

    // Apparently unused
    *(u16_to_ptr(0x206b)) = 1;
}

// Return a pointer the the current player's RAM area
// 返回指向当前玩家RAM的指针
static uint8_t* GetPlayerDataPtr()
{
    // xref 1611
    return u16_to_ptr(m.playerDataMSB << 8);
}

// Handles player firing in game and demo mode.
// In both cases, nothing happens if there is a shot on the screen.
// In game mode, reads the fire button and debounces to detect press.
// In demo mode, initiates a shot always, and consumes the next movement command from the buffer.

/**翻译
 * 处理玩家在游戏模式中的开枪和演示模式中的开枪与移动
 * 在这两种模式中，如果已经开过枪且子弹还在则什么都不会发生
 * 在游戏模式中，读取开火按钮和防抖来检测按键
 * 在演示模式中，一直射击，并使用缓冲区中的下一个移动命令
 */

static void PlrFireOrDemo()
{
    // xref 1618
    if (m.playerAlive != 0xff)
        return;

    uint8_t timer_hi = m.playerHeader.TimerMSB;
    uint8_t timer_lo = m.playerHeader.TimerLSB;

    // Return if not ready yet.
    // 如果还没准备好则返回
    if (timer_lo | timer_hi)
        return;

    // Return if player has a shot still on screen
    // 如果上一发子弹还在屏幕上则返回
    if (m.plyrShotStatus)
        return;

    // xref 162b
    // 判断是否处于游戏模式
    if (m.gameMode)
    {
        // Handle fire button reading and debouncing
        // 处理开火按钮的读取和防抖
        uint8_t prev = m.fireBounce;
        // 当前开火按钮是否按下
        uint8_t cur  = (ReadInputs() & 0x10) != 0;

        if (prev == cur)
            return;


        // 修改开火状态
        if (cur)
            m.plyrShotStatus = 1;       // Flag shot active

        // 更改标志位
        m.fireBounce = cur;
        return;
    }
    else
    {
        // Demo player constantly fires
        // 演示模式下玩家一直开火

        m.plyrShotStatus = 1;

        // Consume demo command from circular buffer 0x1f74 <-> 0x1f7e
        //
        //   DemoCommands:
        //; (1=Right, 2=Left)
        //      74 75 76 77 78 79 7A 7B 7C 7D 7E
        //1F74: 01 01 00 00 01 00 02 01 00 02 01

        /**翻译
         * 从 0x1f74 - 0x1f7e 循环读取演示指令
         * 
         * 演示指令：
         * (1=向右走, 2=向左走)
         *       74 75 76 77 78 79 7A 7B 7C 7D 7E
         * 1F74: 01 01 00 00 01 00 02 01 00 02 01
         */


        Word iter = u16_to_word(m.demoCmdPtr.u16 + 1);


        // 循环读取移动指令
        if (iter.l >= 0x7e)
            iter.l = 0x74; // wrap

        m.demoCmdPtr = iter;
        m.nextDemoCmd = *(word_to_ptr(iter));
    }
}

// Called when all players are dead in game mode
// Prints "GAME OVER" and sets things up to reenter splash screens.

/**翻译
 * 在所有玩家死亡后调用
 * 打印 "GAME OVER" 然后 设置一些东西来重回初始屏幕
 */

static void GameOver()
{
    // xref 16c9

    PrintMessageDel(xytosc(72,192), m.MSG_GAME_OVER__PLAYER___, 10);
    TwoSecDelay();
    ClearPlayField();
    m.gameMode = 0;
    write_port(5, 0);      // all sound off
    EnableGameTasks();
    main_init(1);
    // 不会再返回
    assert(FALSE); // won't return
}

// Called when the player loses the game upon an alien reaching the bottom
// 在玩家因为alien到达屏幕底端而失败的时候被调用
static void on_invaded()
{
    // xref 16ea
    m.playerAlive = 0;

    do {
        PlayerShotHit();
        SoundBits3On(4);
    }
    while (!IsPlayerAlive());

    DsableGameTasks();
    DrawNumShipsSub(xytosc(24, 8)); // 19fa
    PrintNumShips(0);

    // xref 196b
    SoundBits3Off(0xfb);
    player_death(1);

    // won't return
    assert(FALSE);
}

// Increases the difficulty of the game as the player's score gets higher by
// decreasing time between alien shots.

/**翻译
 * 当玩家得分高了以后用减少外星人射击时间间隔来增大难度
 */

static void AShotReloadRate()
{
    // xref 170e
    uint8_t score_hi = *(GetScoreDescriptor() + 1);

    // Uses these tables, in decimal
    //   02 16 32 48       (AReloadScoreTab)
    //   48 16 11 08 07    (ShotReloadRate)

    int i = 0;

    // xref 171c
    for (i = 0; i < 4; ++i)
    {
        if (m.AReloadScoreTab[i] >= score_hi)
            break;
    }

    // xref 1727
    m.aShotReloadRate = m.ShotReloadRate[i];
}

// Turn player shot sound on/off depending on m.plyrShotStatus
// 根据 m.plyrShotStatus 开关玩家射击音乐
static void ShotSound()
{
    // xref 172c
    if (m.plyrShotStatus == 0)
    {
        SoundBits3Off(0xfd);
        return;
    }

    SoundBits3On(0x02);
}

// Ticks down and reset the timers that determines when the alien sound is changed.
// The sound is actually changed in FleetDelayExShip()

/**翻译
 * 当 外星人音频变化时候减少 tick，重置 timrers
 * 音频切换在 FleetDelayExShip() 中执行
 */

static void TimeFleetSound()
{
    // xref 1740
    if (--m.fleetSndHold == 0)
        DisableFleetSound();

    if (m.playerOK == 0)
    {
        DisableFleetSound();
        return;
    }

    if (--m.fleetSndCnt != 0)
        return;

    write_port(5, m.soundPort5);

    if (m.numAliens == 0)
    {
        DisableFleetSound();
        return;
    }

    m.fleetSndCnt = m.fleetSndReload;
    m.changeFleetSnd = 0x01;
    m.fleetSndHold = 0x04;
}

// Turn off the fleet movement sounds
// 关闭飞船移动声音
static void DisableFleetSound()
{
    // xref 176d
    SetSoundWithoutFleet(m.soundPort5);
}

// Mask fleet movement sound off from given byte, and use it to set sound
// 从给定字节屏蔽舰队移动声音，并使用它设置声音
static void SetSoundWithoutFleet(uint8_t v)
{
    // xref 1770
    write_port(5, v & 0x30);
}

// Handles rotating the fleet sounds if it is time to do so, and determines the
// delay between them using a table keyed by the number of live aliens.
// The bonus ship sound is also handled here.

/**翻译
 * 如果时间允许，处理舰队旋转的声音，并且它们之间的延迟根据外星人存活数目由一个表控制。
 * 奖励飞船的声音也在这里处理
 */

static uint8_t FleetDelayExShip()
{
    // xref 1775
    // The two sound tables (in decimal):
    //   [ 50 43 36 28 22 17 13 10 08 07 06 05 04 03 02 01 ] (soundDelayKey)
    //   [ 52 46 39 34 28 24 21 19 16 14 13 12 11 09 07 05 ] (soundDelayValue)
    // 上述为两张声音表 10进制

    uint8_t snd_b = 0;
    uint8_t snd_a = 0;

    if (m.changeFleetSnd)
    {
        uint8_t n = m.numAliens;

        // xref 1785
        int i = 0;

        for (i = 0; i < 16; ++i)
        {
            if (n >= m.soundDelayKey[i])
                break;
        }

        m.fleetSndReload = m.soundDelayValue[i];

        snd_b = m.soundPort5 & 0x30; // Mask off all fleet movement sounds
        snd_a = m.soundPort5 & 0x0f; // Mask off all except fleet sounds

        // Rotate to next sound and wrap if neccessary
        snd_a = (snd_a << 1) | (snd_a >> 7);

        if (snd_a == 0x10)
            snd_a = 0x01;

        m.soundPort5 = snd_a | snd_b;
        m.changeFleetSnd = 0;
    }

    // xref 17aa
    if (--m.extraHold == 0)
        SoundBits3Off(0xef);

    return snd_a;
}

// Read the input port corresponding to the current player.
// 读取当前玩家对应的操作
static uint8_t ReadInputs()
{
    // xref 17c0
    uint8_t port = m.playerDataMSB & 0x1 ? 1 : 2;
    return read_port(port);
}

// End the game if tilt switch is activated.
// 如果 tilt 按键被触发则结束游戏
static void CheckHandleTilt()
{
    // xref 17cd
    if (!(read_port(2) & TILT_BIT)) return;
    if (m.tilt) return;

    yield(YIELD_TILT);
    assert(FALSE);
}

// This routine is entered after the stack reset in CheckHandleTilt
// It prints the tilt message and ends the game, cycling back to the splash screen.

/**翻译
 * 这个函数在 CheckHandleTilt 中重置栈后进入
 * 打印 tilt 信息然后结束游戏，返回开始界面
 */

static void on_tilt()
{
    // xref 17dc
    for (size_t i = 0; i < 4; ++i)
        ClearPlayField();

    m.tilt = 1;
    DsableGameTasks();
    enable_interrupts();

    PrintMessageDel(xytosc(96,176), m.MSG_TILT, 4);
    OneSecDelay();

    m.tilt = 0;
    m.waitStartLoop = 0;

    GameOver(); // does not return.
    assert(FALSE);
}

// Play appropriate sounds based on saucer state.
// 根据飞碟状态播放合适的音乐
static void CtrlSaucerSound()
{
    // xref 1804
    if (m.saucerActive == 0)
    {
        // 协程切换
        SoundBits3Off(0xfe);
        return;
    }

    if (m.saucerHit)
        return;
    
    //协程切换
    SoundBits3On(0x01);
}

// Draws the text and sprites for the "SCORE ADVANCE TABLE" in the splash screens.
// 绘制开始屏幕的分数表信息
static void DrawAdvTable()
{
    // xref 1815
    // 绘制 SCORE ADVANCE TABLE 文字
    PrintMessage(xytosc(32,128), m.MSG__SCORE_ADVANCE_TABLE_, 0x15);

    // PrintMessageAdv uses this
    // 每个 "=xx POINTS" 文字都有 10 bytes
    m.temp206C = 10;

    PriCursor cursor;

    // Sprite display list for score advance table
    // 0x1dbe（m.SCORE_ADV_SPRITE_LIST）处 内容：
    // 用于绘制 "SCORE ADVANCE TABLE" 信息
    // 1DBE : 0E 2C 68 1D;  Flying Saucer
    // 1DC2 : 0C 2C 20 1C;  Alien C, sprite 0 
    // 1DC6 : 0A 2C 40 1C;  Alien B, sprite 1 
    // 1DCA : 08 2C 00 1C;  Alien A, sprite 0 
    // 1DCE : FF;           End of list
    cursor.src = m.SCORE_ADV_SPRITE_LIST;

    while (!ReadPriStruct(&cursor))
        Draw16ByteSprite(cursor.sc, cursor.obj);

    // Message display list for score advance table
    cursor.src = m.SCORE_ADV_MSG_LIST;

    while (!ReadPriStruct(&cursor))
        PrintMessageAdv(cursor.sc, cursor.obj);
}


// Used when drawing the Score Advance Table to draw the alien and saucer sprites
// 打印分表
static void Draw16ByteSprite(Word sc, uint8_t* sprite)
{
    // xref 1844
    SprDesc desc;
    desc.sc = sc;
    desc.spr = ptr_to_word(sprite);
    desc.n = 16;

    DrawSimpSprite(&desc);
}

// Used when drawing the Score Advance Table to draw the text
// 画图
static void PrintMessageAdv(Word sc, uint8_t* msg)
{
    // xref 184c
    size_t n = m.temp206C;
    PrintMessageDel(sc, msg, n);
}

// A PriStruct is a display list, containing a list of
// objects to display (either sprites or text), and their position.
// This routine is used to iterate through the list and read each member.

/**翻译
 * Pritruct 是一个播放列表，包含一系列需要播放的对象（精灵或文字) 以及它们的位置
 * 这个程序用于以迭代器的方式读取列表中的每个成员
 */

static int ReadPriStruct(PriCursor *pri)
{
    // xref 1856
    pri->sc.l = *pri->src++;

    if (pri->sc.l == 0xff)
        return TRUE; // hit end

    pri->sc.h = *pri->src++;

    Word obj;
    obj.l = *pri->src++;
    obj.h = *pri->src++;

    pri->obj = word_to_ptr(obj);

    return FALSE; // keep going
}

// Required for CCOIN and PLAy splash animations
// The animation structure used here is set up by IniSplashAni()
// Moves and animates a sprite until it reaches a target position

/**翻译
 * 用于播放 CCOIN 和 PLAy 动画
 * 所用的动画结构体被 IniSplashAni() 配置
 * 移动动画或者精灵直到指定位置
 * 图片每移动 4 下切换一次
 */

static void SplashSprite()
{
    // xref 1868
    ++m.splashAnForm;
    uint8_t* vptr = &m.splashDelta.y;

    // 获取 Y 移动步长
    uint8_t dy = *vptr;
    // 获取目标 X 坐标并更新起始坐标（下一次从这开始移动）
    uint8_t x = AddDelta(vptr, dy);

    // 判断是否完成移动
    if (m.splashTargetX != x)
    {
        
        uint8_t flip = m.splashAnForm & 0x04;
        Word spr = m.splashImRest;


        // 同一个人物的走路动画之间间隔 48 字节
        // 见 https://computerarcheology.com/Arcade/SpaceInvaders/Code.html#CnvtPixNumber
        // Alien Images 中的内存地址分布
        if (flip == 0) { spr.u16 += 48; }

        // 将当前动画显示的图片地址存到模拟内存中（结合 RAM 分布图看）
        // https://computerarcheology.com/Arcade/SpaceInvaders/RAMUse.html
        m.splashPic = spr;


        // 获取动画描述符，直接读取
        SprDesc desc = ReadDesc((SprDesc*) u16_to_ptr(SPLASH_DESC_ADDR));

        // splash desc is out of order, needs swapping.
        Word tmp = desc.pos;

        desc.pos = desc.spr;
        desc.spr = tmp;

        DrawSprite(&desc);
        return;
    }

    // xref 1898
    m.splashReached = 1;
}

// Handles the shooting part of the CCOIN splash animation
// Companion routine is SplashSquiggly()

/**翻译
 * 处理射击 CCOIN 动画中射击的部分
 * 伴随 SplashSquiggly() 函数运行
 */

static void AnimateShootingSplashAlien()
{
    // xref 189e
    // 设置ROM
    BlockCopy(u16_to_ptr(SQUIGGLY_SHOT_ADDR), m.SPLASH_SHOT_OBJDATA, 16);

    m.shotSync = 2;
    m.alienShotDelta = 0xff;
    m.isrSplashTask = 4;

    // spin until shot collides
    while ((m.squShotData.Status & SHOT_BLOWUP) == 0)
    {
        // xref 18b8
        timeslice(); // spin
    }

    // spin until shot explosion finishes
    // 等待子弹爆炸动画完成
    while ((m.squShotData.Status & SHOT_BLOWUP) != 0)
    {
        // xref 18c0
        timeslice(); // spin
    }

    // replace extra 'C' with blank space
    // 用空白覆盖多余的 'C'
    DrawChar(xytosc(120,136), 0x26);

    TwoSecDelay();
}

// Handle initialization and splash screens.
// Initially entered upon startup via reset(), with skip=0
// Is is terminated when the stack is reset upon the insertion of credits, and is
// replaced by WaitForStart()
// The routine is eventually re-entered with skip=0 via GameOver()

/**翻译
 * 处理初始化和开机界面
 * 在启动时伴随变量 skip=0 经由 reset() 函数进入（run_main_ctx 中的第一种情况）
 * 这个函数将在投币后被 WaitForStart() 取代
 * 主要是初始化游戏内存，按顺序设置并循环播放初始界面的动画
 */

static void main_init(int skip)
{
    // xref 18d4
    int init = !skip;
    // 判断是否需要初始化
    if (init)
    {
        // 从 ROM 中加载数据到 RAM
        CopyRomtoRam();
        DrawStatus();
    }

    // 18df
    while (TRUE)
    {
        // 判断是否是要初始化
        if (init)
        {
            m.aShotReloadRate = 8;

            write_port(3, 0); // turn off sound 1
            write_port(5, 0); // turn off sound 2

            // SetISRSplashTask
            m.isrSplashTask = 0;

            // xref 0af2
            enable_interrupts();

            OneSecDelay();

            // xref 0af6
            PrintMessageDel(xytosc(96, 184),
                            m.splashAnimate ? m.MSG_PLAY : m.MSG_PLAY2,
                            4);

            // xref 0b08
            // 打印 “SPACE INVADERS“ 动态消息
            MessageToCenterOfScreen(m.MSG_SPACE__INVADERS);

            // xref 0b0e
            OneSecDelay();
            DrawAdvTable();
            TwoSecDelay();

            // xref 0b17
            // 这个值默认不是 0，所以第一次来的时候会被跳过
            if (m.splashAnimate == 0)
            {
                // run script for alien twiddling with upside down 'Y'
                // 播放主界面外星人将反转的 Y 正过来的动画

                 /**
                 *  外星人来左边抓倒立的 Y
                 * ROM 中 0x1a95 内容（原版汇编代码注释）：
                 * 0x1A95: 00 00 FF B8 FE 20 1C 10 9E 00 20 1C（这里是小端模式）
                 * 其中解释：
                 * 00   -Image form（记录当前是第几帧，每次绘制自增）
                 * 00   -dx = 0（x 坐标不动）
                 * FF   -dy = -1（Y 坐标每次减少 1）（从右往左运动）
                 * BB   -X 起始坐标
                 * FE   -Y 起始坐标
                 * 1C20 -基础图形（小外星人）
                 * 10   -图片大小（16 bytes）
                 * 9E   -目标 Y 坐标
                 * 00   -到达 Y 时标志
                 * 1C20 -基础图形（小外星人）
                 * 
                 * 
                 * ！！！！！！！！注意！！！！！！
                 * 这里的 X 和 Y 是原版汇编代码使用的坐标系，实际上这里应该将
                 * dx - dy 和 x - y 调转一下即
                 * 00   -dy = 0
                 * FF   -dx = -1
                 * BB   -Y 起始坐标
                 * FE   -X 起始坐标
                 * 9E   -到达 X 坐标
                 * 00   -到达 X 时标志
                 * ！！！！！！！！！！！！！！！！！
                 * 
                 * 总结：将图片从 X=FE 移动到 X=9E，步长=-1
                 */


                IniSplashAni(u16_to_ptr(0x1a95));
                // 播放上一段设定的动画，然后再继续
                Animate();
                 /**
                 * 外星人带着倒立的 Y 从右边离开
                 * ROM 中 0x1BB0 内容：
                 * 0x1BB0: 00 00 01 B8 98 A0 1B 10 FF 00 A0 1B 00 00 00 00（这里是小端模式）
                 * 具体详解套上一个注释
                 * 
                 * 总结：将图形从 X=98 移动到 X=FF，步长=1
                 */
                IniSplashAni(u16_to_ptr(0x1bb0));
                // 播放上一段设定的动画，然后再继续
                Animate();

                OneSecDelay();

                /**
                 * 外星人带着正确的 Y 放回到 PLAy 上
                 * ROM 中 0x1fc9 内容：
                 * 0x1FC9: 00 00 FF B8 FF 80 1F 10 97 00 80 1F（这里是小端模式）
                 * 具体详解套上一个注释
                 * 
                 * 总结：将图片从 X=FF 移动到 X=97，步长=1
                 */

                IniSplashAni(u16_to_ptr(0x1fc9));
                // 播放上一段设定的动画，然后再继续
                Animate();

                OneSecDelay();
                // 清除这一部分像素
                ClearSmallSprite(xytosc(125, 184), 10, 0);
                TwoSecDelay();
            }

            // xref 0b4a
            // 清理屏幕
            ClearPlayField();

            if (m.p1ShipsRem == 0)
            {
                // xref 0b54
                m.p1ShipsRem = GetShipsPerCred();
                RemoveShip();
            }

            // xref 0b5d
            // 重置部分 RAM
            CopyRAMMirror();
            // 复活 P1 为alien
            InitAliensP1();
            // 初始化盾牌
            DrawShieldP1();
            RestoreShieldsP1();
            m.isrSplashTask = 1;

            DrawBottomLine();

            // xref 0b71

            /** 开始播放 demo 动画
             * 
             * 包括将 vram 显示到屏幕上、更新外星人对象指针
             * 都是在 run_int_ctx 中进行处理，协程交替运行不断更新每个对象
             * 每次都是处理一个sprite
             * 
             */

            do
            {
                PlrFireOrDemo();

                // xref 0b74
                // check player shot, and aliens bumping screen, also handle hidden message
                uint8_t a2 = CheckPlyrShotAndBump();

                // feed the watchdog
                write_port(6, a2);

            } while (IsPlayerAlive());

            // xref 0b7f
            m.plyrShotStatus = 0;

            // wait for demo player to finish exploding.
            while (! IsPlayerAlive() )
            {
                // xref 0b83
                timeslice(); // spin
            }
        }

        // xref 0b89
        m.isrSplashTask = 0;
        OneSecDelay();
        ClearPlayField();

        PrintMessage(xytosc(64,136), m.MSG_INSERT__COIN, 12);

        // draw extra 'C' for CCOIN
        if (m.splashAnimate == 0)
            DrawChar(xytosc(120,136), 2);

        PriCursor cursor;
        cursor.src = m.CREDIT_TABLE;

        ReadPriStruct(&cursor);
        PrintMessageAdv(cursor.sc, cursor.obj);

        // Only print coin info if DIP7 is set
        if ((read_port(2) & DIP7_COININFO) == 0)
        {
            cursor.src = m.CREDIT_TABLE_COINS;

            // xref 183a
            while (!ReadPriStruct(&cursor))
                PrintMessageAdv(cursor.sc, cursor.obj);
        }

        TwoSecDelay();

        // xref 0bc6
        if (m.splashAnimate == 0)
        {
            // xref 0bce
            // shoot C animation
            IniSplashAni(u16_to_ptr(0x1fd5));
            Animate();
            AnimateShootingSplashAlien();
        }

        // xref 0bda
        m.splashAnimate = !m.splashAnimate;
        ClearPlayField();
    }
}

// Return pointer to non current player's alive status
// 返回指向非当前玩家存活态的指针
static uint8_t* GetOtherPlayerAliveFlag()
{
    // xref 18e7
    return  (m.playerDataMSB & 0x1 ? &m.playerStates.h : &m.playerStates.l);
}

// Use a mask to enable sounds on port 3
// 使用掩码来使能 port 3 的音乐
static void SoundBits3On(uint8_t mask)
{
    // xref 18fa
    uint8_t snd = m.soundPort3;
    snd |= mask;
    m.soundPort3 = snd;
    write_port(3, snd);
}

// Init all the aliens for P2
// 初始化 P2 aliens
static void InitAliensP2()
{
    // xref 1904
    InitAliensSub(u16_to_ptr(P2_ADDR));
}

// Called from the main thread to do some miscellaneous tasks
// a) Player shot collision response
// b) Detecting and handling the alien rack bumping the screen edges

/**翻译
 * 在主协程中做一些杂活
 * a) 玩家子弹碰撞响应
 * b) 检测并处理外星飞船碰撞屏幕边缘
 */

static void PlyrShotAndBump()
{
    // xref 190a
    // 判定子弹状态
    PlayerShotHit();
    // 判定外星飞船是否碰壁
    RackBump();
}
/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 后边的代码未进行注释，其中的部件作用有所重复，
 * 本人所做的工作也只能机械的翻译
 * 故后边不再进行注释
 * 
 * 
 * 
 * 
 * 
 * 
 */
// Return pointer to current player's alive status
static uint8_t* CurPlyAlive()
{
    // xref 1910
    return  (m.playerDataMSB & 0x1 ? &m.playerStates.l : &m.playerStates.h);
}

// Draw the score text labels at the top of the screen
static void DrawScoreHead()
{
    // xref 191a
    PrintMessage(xytosc(0,240), m.MSG__SCORE_1__HI_SCORE_SCORE_2__, 28);
}

// Draw the score for P1
static void PrintP1Score()
{
    // xref 1925
    DrawScore(&m.P1Scor.l);
}

// Draw the score for P2
static void PrintP2Score()
{
    // xref 192b
    DrawScore(&m.P2Scor.l);
}

// Draw the score using the given descriptor in iter
static void DrawScore(uint8_t* iter)
{
    // xref 1931

    Word pos;
    Word val;

    val.l = *iter++;
    val.h = *iter++;
    pos.l = *iter++;
    pos.h = *iter++;

    Print4Digits(pos, val);
}

// Draw the credit text label at the bottom right
static void PrintCreditLabel()
{
    // xref 193c
    PrintMessage(xytosc(136,8), m.MSG_CREDIT_, 7);
}

// Draw the number of credits at the bottom right
static void DrawNumCredits()
{
    // xref 1947
    DrawHexByte(xytosc(192,8), m.numCoins);
}

// Draw the high score
static void PrintHiScore()
{
    // xref 1950
    DrawScore(&m.HiScor.l);
}

// Clear the screen and draw all the text surrounding the playfield
static void DrawStatus()
{
    // xref 1956
    ClearScreen();
    DrawScoreHead();

    PrintP1Score();
    PrintP2Score();

    PrintHiScore();

    PrintCreditLabel();
    DrawNumCredits();

    // Midway patched this out
    // PrintTaitoCorporation();
}

// Prints "* TAITO CORPORATION *" at the bottom of the screen
// This is a bit of dead code due to the patching out in DrawStatus()
static void PrintTaitoCorporation()
{
    // xref 198b
    PrintMessage(xytosc(32, 24), u16_to_ptr(0x19be), 0x13);
}

// Called during the game demo to check if player has entered the correct
// button combos to display the easter egg, "TAITO COP"
static void CheckHiddenMes()
{
    // xref 199a
    uint8_t a = 0;

    if (m.hidMessSeq == 0)
    {
        a = read_port(1);
        a &= 0x76;
        a -= 0x72;

        if (a)
            return;

        m.hidMessSeq = 1;
    }

    a = read_port(1);
    a &= 0x76;
    a -= 0x34;

    if (a)
        return;

    PrintMessage(xytosc(80,216), m.MSG_TAITO_COP, 9);
}

// Allow game related tasks in interrupt routines
static void EnableGameTasks()
{
    // xref 19d1
    m.gameTasksRunning = 1;
}

// Disallow game related tasks in interrupt routines
static void DsableGameTasks()
{
    // xref 19d7
    m.gameTasksRunning = 0;
}

// Use a mask to turn off sounds in port 3
static void SoundBits3Off(uint8_t mask)
{
    // xref 19dc
    uint8_t snd = m.soundPort3;
    snd &= mask;
    m.soundPort3 = snd;
    write_port(3, snd);
}

// Draw the sprites representing the number of lives (0 based) remaining at the bottom left
static void DrawNumShips(uint8_t n)
{
    // xref 19e6
    SprDesc desc;

    desc.sc = xytosc(24,8);
    desc.spr = ptr_to_word(m.PLAYER_SPRITES);
    desc.n = 16;

    for (int i = 0; i < n; ++i)
        desc.sc = DrawSimpSprite(&desc);

    DrawNumShipsSub(desc.sc);
}

// Clears the space to the right of the ship sprites (used to remove lives)
static void DrawNumShipsSub(Word pos)
{
    // xref 19fa
    // Clear up to x = 136 (start of credit label)

    do
    {
        pos = ClearSmallSprite(pos, 16, 0);
    } while ((pos.x != 0x35));
}

// Returns true if given obj is positioned on the half of screen that is not currently being drawn.
// Used to control which interrupt draws a particular game object.
// This prevents flicker.
static uint8_t CompXrToBeam(uint8_t* posaddr)
{
    // xref 1a06
    uint8_t b = m.vblankStatus;
    uint8_t a = *posaddr & XR_MID;

    return a == b;
}

// memcpy
static void BlockCopy(void *_dest, void *_src, size_t n)
{
    // xref 1a32

    uint8_t* dest = (uint8_t*) (_dest);
    uint8_t* src = (uint8_t*) (_src);

    for (size_t i = 0; i < n; ++i)
        *dest++ = *src++;
}

// Return a copy of a Sprite Descriptor from src
static SprDesc ReadDesc(SprDesc* src)
{
    // xref 1a3b
    SprDesc desc;
    desc.spr.l = src->spr.l;
    desc.spr.h = src->spr.h;

    desc.pos.l = src->pos.l;
    desc.pos.h = src->pos.h;

    desc.n = src->n;
    return desc;
}

// Convert a pixel pos to a screen address
// Pixel positions in memory are pre-offset by +32 pixels, meaning that when
// they are converted to a screen coordinate, 0x400 has already been added.
// Hence the or with 0x2000 below, instead of +0x2400
// See xpix()
static Word ConvToScr(Word pos)
{
    // xref 1a47
    return u16_to_word((pos.u16 >> 3) | 0x2000);
}

// bzero the 7168 bytes of vram
static void ClearScreen()
{
    // xref 1a5c
    uint8_t* screen = m.vram;
    size_t n = 7168;

    for (size_t i = 0; i < n; ++i)
        screen[i] = 0;
}

// CopyShields() subroutine.
//   cursor   - used to iterate through the screen and the player buffer
//   spr_size - contains the number of rows and bytes per row of the sprite
//   dir=0 - Copy a shield from the buffer to the screen
//   dir=1 - Copy a shield from the screen to the buffer
static void CopyShieldBuffer(ShieldBufferCursor *cursor, Word spr_size, uint8_t dir)
{
    // xref 1a69
    uint8_t nr = spr_size.h;
    uint8_t nb = spr_size.l;

    for (int i = 0; i < nr; ++i, cursor->sc.u16 += xysc(1,0))
    {
        uint8_t* screen = word_to_ptr(cursor->sc);

        for (int j = 0; j < nb; ++j)
        {
            if (dir == 0)
                *screen = *cursor->iter | *screen;
            else
                *cursor->iter = *screen;

            cursor->iter++;
            screen++;
        }
    }
}

// Take a life away from the player, and update the indicators
static void RemoveShip()
{
    // xref 1a7f
    uint8_t* nships_ptr;
    uint8_t num = GetNumberOfShips(&nships_ptr);

    if (num == 0)
        return;

    *nships_ptr = num-1;

    // The sprite indicator is 0 based.
    DrawNumShips(num-1);

    // The text indicator is 1 based.
    DrawHexByteSub(xytosc(8,8), num & 0xf);
}

// Print the numeric lives indicator at the bottom left
static void PrintNumShips(uint8_t num)
{
    // xref 1a8b
    DrawChar(xytosc(8,8), (num & 0x0f) + 0x1a);
}

// GAMECODE FINISH


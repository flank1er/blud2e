/* Convertor MAP files by Blood video game, Monolith 1997(c) to eduke32 map format
   Project: Blood CrossMatching
   Author: flanker
   It's fork by blud2b.c, HTTP://blood.sourceforge.net with additional features
   License GPL v.3
*/

#include <map>
#include <vector>
#include <set>
#include <glm/glm.hpp>
#include <typeinfo>
#include <functional>
#include <string>
#include <sstream>
#include <iostream>
#include <GL/gl.h>

#define SHOW(a) std::cout << #a << ": " << (int)(a) << std::endl
#define PN T.picnum
#define LT T.tag.r
#define HT T.tag.g
#define SC std::cout
#define SE std::endl
#define lastSprite spV.at(spV.size()-1)

struct WL {
    int num;
    glm::ivec2 pt;
};


bool fileExists(const char*);

const int size_signature=6;
const int size_firstHeader=18;
const int size_secondHeader=9;
const int size_thirdHeader=10;
const int size_extra=128;
const int size_unknownElts=2;
const int size_blood_header=size_signature+size_firstHeader+size_secondHeader+size_thirdHeader+size_extra;

template<class InputIterator, class Function>
Function for_loop(InputIterator first, Function fn)
{
    auto it=first;
    do{
        fn(*it);
        it=it->nextPoint;
    } while (it != first);

    return fn;
};

// header for Duke maps
struct dukeMap {
	int version;
	int X;
	int Y;
	int Z;
    unsigned short angle;
	short sector;
};

struct BloodHeader {
    unsigned int label=0x1A4D4C42;
    unsigned short ver=0x0700;
    int startX;
    int startY;
    int startZ;
    short startAngle;
    short sectorNum;
    short unknownElts;
    char author[size_secondHeader];
    int mapRevisions;
    short numSectors;
    short numWalls;
    short numSprites;
    char copyright[size_extra];
}__attribute__ ((__packed__));


struct OF
{
    char data[56];
};

struct Sector {
	short wallptr, wallnum;
	int ceilingz, floorz;
	short ceilingstat, floorstat;
	short ceilingpicnum, ceilingheinum;
	signed char  ceilingshade;
	unsigned char ceilingpal, ceilingxpanning, ceilingypanning;
	short floorpicnum, floorheinum;
	signed char  floorshade;
	unsigned char floorpal, floorxpanning, floorypanning;
	unsigned char visibility, filler; // Filler "should" == 0
	short lotag, hitag, extra;
    void print(std::stringstream&);
};

struct xSector {
    signed refer : 14;
    unsigned state : 1;

    // trigger data
    unsigned busy : 17;
    signed data : 16;
    unsigned txID : 10;
    unsigned rxID : 10;
    unsigned command : 8;
    unsigned triggerOn : 1;
    unsigned triggerOff : 1;
    unsigned busyTime : 12; // time to reach next state
    unsigned waitTime : 12; // delay before callback
    unsigned restState : 1; // state to return to on callback
    unsigned interruptable : 1;

    // lighting data
    signed amplitude : 8;
    unsigned freq : 8;
    unsigned phase : 8;
    unsigned wave : 4;
    unsigned shadeAlways : 1;
    unsigned shadeFloor : 1;
    unsigned shadeCeiling : 1;
    unsigned shadeWalls : 1;
    signed shade : 8;

    // panning data
    unsigned panAlways : 1;
    unsigned panFloor : 1;
    unsigned panCeiling : 1;
    unsigned drag : 1;

    // wind/water stuff
    unsigned underwater : 1;
    unsigned depth : 2;
    unsigned panVel : 8;
    unsigned panAngle : 11;
    unsigned wind : 1;

    // physical triggers
    unsigned decoupled : 1;
    unsigned triggerOnce : 1;
    unsigned isTriggered : 1;
    unsigned key : 3;
    unsigned triggerPush : 1;
    unsigned triggerImpact : 1;
    unsigned triggerReserved0 : 1;
    unsigned triggerEnter : 1;
    unsigned triggerExit : 1;
    unsigned triggerWPush : 1;
    unsigned triggerReserved1 : 1;
    unsigned triggerReserved2 : 1;

    // movement data
    signed offCeilZ : 32;
    signed onCeilZ : 32;
    signed offFloorZ : 32;
    signed onFloorZ : 32;
    signed marker0 : 16; // could be 13 bits
    signed marker1 : 16; // could be 13 bits
    unsigned crush : 1;

    unsigned ceilxpanFrac : 8;
    unsigned ceilypanFrac : 8;
    unsigned floorxpanFrac : 8;
    unsigned floorypanFrac : 8;

    unsigned locked : 1;
    unsigned pad : 31;

  //  unsigned upperLink : 11;
   // signed upperLinkZ : 32;
  //  unsigned lowerLink : 11;
//    signed lowerLinkZ : 32;
};

struct Sprite {
	int x, y, z;
	unsigned short cstat;
	short picnum;
	signed char shade;
	unsigned char pal, clipdist, filler;
	unsigned char xrepeat, yrepeat;
	signed char xoffset, yoffset;
	short sectnum, statnum;
	short ang, owner, xvel, yvel, zvel;
	short lotag, hitag, extra;
    void print(std::stringstream&);
};

struct xSprite {
    signed refer : 14;
    unsigned state : 1;

    // trigger data
    unsigned busy : 17;
    // 0 bits unused
    //unsigned unused6 : 4;
    unsigned txID : 10;                 // --->0x04
    unsigned rxID : 10;
    unsigned command : 8;
    unsigned triggerOn : 1;
    unsigned triggerOff : 1;
    // 2 bits unused
    unsigned wave : 2;

    unsigned busyTime : 12; // time to reach next state         <-- 0x08
    unsigned waitTime : 12; // delay before callback
    unsigned restState : 1; // state to return to on callback   <-- 0x0B
    unsigned interruptable : 1;

    unsigned difficulty : 2;
    // 4 bits unused
    unsigned unknow1 : 3;
    unsigned launchT : 1;
    unsigned dropId : 8;        //                              <-- 0x0C

    // physical triggers
    unsigned decoupled : 1;    //                                 <-- 0x0D
    unsigned triggerOnce : 1;
    unsigned isTriggered : 1; // used for triggerOnce objects
    unsigned key : 3;
    unsigned triggerPush : 1;
    unsigned triggerVector : 1; // action key                   <-- 0x0E
    unsigned triggerImpact : 1; // vector hits
    //unsigned triggerReserved0 : 1;
    unsigned triggerPickup : 1; // secrets
    unsigned triggerTouch : 1; // sawblades, spikes, zaps?
    unsigned triggerSight : 1; // dunno, yet.
    unsigned triggerProximity : 1; // proximity bombs
    unsigned triggerReserved1 : 1;
    unsigned triggerReserved2 : 1;
    unsigned launch1 : 1;

    unsigned launch2: 1;    //                                      <-- 0x0F
    unsigned launch3: 1;
    unsigned launch4: 1;
    unsigned launch5: 1;
    unsigned launchS: 1;
    unsigned launchB: 1;
    unsigned launchC: 1;
    unsigned triggerDudeLockout : 1;

    signed data1 : 16; // combo value?                      <-- 0x10
    signed data2 : 16; // combo key?                        <-- 0x12

    signed data3 : 16; // combo max?                        <-- 0x14
    unsigned goalAng : 11; // dudes                         <-- 0x16
    signed dodgeDir : 2; // dudes
    unsigned isLocked : 1;
    unsigned unknow3 : 2; //
    unsigned respawn : 2; // 0=optional never, 1=optional always, 2=always, 3=never
    unsigned data4 : 16;
    unsigned unknow4 : 6;

    unsigned lockMsg : 8; //                             <-- 0x1B
    unsigned health : 12;

    unsigned dudeDeaf : 1; // can't hear players                || 0x1D
    unsigned dudeAmbush : 1; // must be triggered manually
    unsigned dudeGuard : 1; // won't leave sector
    unsigned dudeFlag4 : 1;
    //
    signed target : 16; //                              <-- 0x1E
    signed targetX : 32;  //                              <-- 0x20
    signed targetY : 32; //                                 <-- 0x24
    signed targetZ : 32; //                                 <- 0x28
    unsigned burnTime : 16;  //                               <-- 0x2C
    signed burnSource : 16;  //                               <-- 0x2E

    unsigned unused2 : 16;  //                                 // <-- 0x30
    unsigned stateTimer : 16;  //                              // <-- 0x32

    unsigned aiState : 32;                                      // <-- 0x34
    // 0x38 <--> 56
};

struct Wall {
 	int x, y;
	short point2, nextwall, nextsector, cstat;
	short picnum, overpicnum;
	signed char  shade;
	unsigned char pal, xrepeat, yrepeat, xpanning, ypanning;
	short lotag, hitag, extra;
    void print(std::stringstream&);
};

struct xWall {
    signed refer : 14;//                    <-- 0x00
    unsigned state : 1;

    // trigger data
    unsigned busy : 17;
    signed data : 16; //                        <-- 0x04
    unsigned txID : 10;//                       <-- 0x06
    unsigned rxID : 10;//
    unsigned command : 8;
    unsigned triggerOn : 1;
    unsigned triggerOff : 1;
    unsigned busyTime : 12; // time to reach next state
    unsigned waitTime : 12; // delay before callback
    unsigned restState : 1; // state to return to on callback
    unsigned interruptable : 1;

    // panning data
    unsigned panAlways : 1;
    signed panXVel : 8;
    signed panYVel : 8;

    // physical triggers
    unsigned decoupled : 1;
    unsigned triggerOnce : 1;
    unsigned isTriggered : 1; // used for triggerOnce objects
    unsigned key : 3;
    unsigned triggerPush : 1;
    unsigned triggerImpact : 1;
    unsigned triggerReserved0 : 1;
    unsigned triggerReserved1 : 1;
    unsigned triggerReserved2 : 1;

    unsigned xpanFrac : 8;
    unsigned ypanFrac : 8;

    unsigned locked : 1;
    unsigned dudeLockout : 1;
    unsigned pad : 30;
};

struct FLOOR {
    float z;
    int texture_id;
    glm::ivec2 res;
    glm::vec2 delta; // sec_t::grad[2]  map2stl.c
};

class unionSector;

class  unionWall : public Wall, public xWall {
    public:
        void print(std::stringstream&);
        bool over=false;
        bool done=false;
        glm::vec3 pos;  //position
        glm::vec4 gis;  // gis.z = floorZ ; gis.w=ceilingZ
        glm::ivec3 tag; // lotag+hitag+extra_tag
        glm::ivec2 log;
        glm::ivec2 res;
        int texture_id;
        int outside_texture_id;
        int sector;
        std::vector<unionWall>::iterator nextPoint;
        std::vector<unionWall>::iterator prePoint;
        std::vector<unionWall>::iterator nextWall;
        std::vector<unionSector>::iterator nextSector;
        std::vector<unionSector>::iterator thisSector;
        glm::vec3 get_normal();
        int getTX() { return (over)? txID : 0;}
        int getRX() { return (over)? rxID : 0;}
        void setNum(int value) { num=value;}
        int getNum() { return num;}
        int getRefer() { return (over) ? refer: -1;}
    private:
        int num;
};

struct LOOP {
    std::vector<unionWall>::iterator marker;
    LOOP() {proper=inner=loop=rectangle=generic=false;};
    ~LOOP() {};
    bool property(std::string req)
    {
        if (req == "proper" && proper)
            return true;
        else if (req == "inner" && inner)
            return true;
        else if (req == "loop" && loop)
            return true;
        else if (req == "rectangle" && rectangle)
            return true;
        else if (req == "generic" && generic)
            return true;

        return false;
    };
    void set_property(std::string data)
    {
        if (data == "proper")
            proper=true;
        else if (data == "inner")
            inner=true;
        else if (data == "loop")
            loop=true;
        else if (data == "rectangle")
            rectangle=true;
        else if (data == "generic")
            generic=true;
    };

private:
    bool proper;
    bool inner;
    bool loop;
    bool rectangle;
    bool generic;
};

class  unionSector : public Sector, public xSector {
    public:
        FLOOR floor;
        FLOOR ceiling;
        void print(std::stringstream&);
        bool over=false;
        bool done=false;
        bool inner=false;
        bool ssd=false; // second slide door; for processing slide marker door;
        glm::ivec3 tag;
        glm::ivec2 log;
        glm::ivec2 dxdy;
        std::vector<unionSector>::iterator originNumber;
        std::vector<unionWall>::iterator firstWall;
        std::vector<LOOP> loops;
        std::vector<LOOP>::iterator generic;
        bool is(std::string title) { if ( over && to_num[title] == lotag) return true; return false;};
        float get_floorZ(float, float);
        float get_ceilingZ(float, float);
        int wall_count(){
            int ret=0;
            for(auto i: loops)
                for_loop(i.marker, [&ret](unionWall _s){ret++; _s=_s;}); // _s=_s REMOVE IT AFTER!!!
            return ret;
        };
        int getTX() { return (over)? txID : 0;}
        int getRX() { return (over)? rxID : 0;}
        void setNum(int value) { num=value;}
        int getNum() { return num;}
        int getRefer() { return (over) ? refer: -1;}
        bool isInside(glm::vec3);
        glm::vec3 get_rnd_pos(std::stringstream&);
        glm::vec3 getCenter();
        glm::vec2 getNNS(glm::vec2 pt, std::stringstream&);
    private:
        std::map<std::string, int> to_num={{"MotionZ", 600},{"Slide", 616}, {"Rotate", 617}, {"Slide Marked", 614},
                                          {"Z Motion SPRITE", 602}};
        int num;
};

class  unionSprite : public Sprite, public xSprite {
    private:
        int numberID;
        std::map<std::string, int> to_num={{"SectorSFX", 7640}, {"levelLocker", 5438},{"FootTraces", 7024},
        {"FireExt",5778}, {"Light switch",6198}, {"Lamp1", 7410},{"Lamp2", 5915}, {"Torch", 32}, {"Tube Lamp", 5916}};
        std::map<std::string, int> to_type={{"Ambient SFX", 710},{"BloodDrip Gen", 702}, {"WaterDrip Gen", 701},
        {"Player Start", 1}, {"Hidden Exploder", 459}, {"Toggle switch", 20}, {"1-Way switch", 21},
        {"Sector SFX", 709}, {"Player SFX", 711}, {"SFX Gen", 708}, {"Explode Object", 417}, {"Gib Object", 416},
        {"Dude Spawn", 18}, {"Wall Crack", 408}, {"Upper stack",11}, {"Lower stack", 12}};
    public:
        void print(std::stringstream&);
        bool over=false;
        bool done=false;
                glm::vec4 pos;
        glm::ivec3 tag;
        glm::ivec3 vel;
        glm::ivec2 log;
        glm::ivec2 res;
        int texture_id;
        std::vector<unionSector>::iterator inSector;
        bool is(std::string title) { if (to_num[title] == texture_id) return true; else return false;};
        bool isType(std::string title) { if (to_type[title] == lotag)return true; else return false;};
        void makeLevelLocker() {vel.z=1;};
        int makeAmbient(int, int);
        void makeBloodDripGen() {texture_id=6268;};
        void makeWaterDripGen() {texture_id=6382;};
        void makeFootTraces() {cstat &= ~(1<<9);};
        void makeHiddenExploder(){tag.r=waitTime*10;};
        //void makeCstat();
        int getTX() { return (over)? txID : 0;}
        int getRX() { return (over)? rxID : 0;}
        void setNum(int value) { numberID=value;}
        int getNum() { return numberID;}
        int getRefer() { return (over) ? refer: -1;}
};

class Resources {
private:
    const char* tex_con_file="defs.con";
    const char* sound_con_file="sounds.con";
    const char* original_sound="sounds_old.con";
    const char* pic_file="pic_table.con";

	std::map<int, std::string> source;
	std::map<int, std::string> texture;
	std::map<std::string, int> target;
    std::map<int, glm::ivec2>  pics_table;
public:
    template<class Function> Function get_all_target(Function fn) {
        for(auto T: target)
            fn(T.second);
        return fn;
    };

    int trans(const int sound_id) {
		std::string stri=source[sound_id];
        return target[stri];
    };

    glm::ivec2 get_resolution(int value) {return pics_table[value];};

    int load_tables(std::stringstream&);
};

class blud2e  {

protected:
    std::vector<unionSector> sV;
    std::vector<unionWall> wV;
    std::vector<unionSprite> spV;

private:
    BloodHeader BH;
    dukeMap dh;
    unsigned int uKelm;
    OF map_offset;
    int lengthMap, Revision=-1; //, posX,posY, posZ;
    float scale=1.f;
    std::map<int,std::string> map_descriptor={{-7, "Blood"}, {7, "Duke3D"}, {9, "EDuke32"}}; // ENUM ??
    int map_specification;
    void showInfo(std::stringstream&);
    std::string mapIs; // for dirty hacks

    Resources RS;

    int addSprite(int, int, int,std::string, std::stringstream&, glm::vec4);
    void Cstat();
    void makeEnemies();
    void makeSectorSFX(unionSprite& Sp);
    void makeRespawn(std::stringstream&);
    void makeExplodeAndGib();
    std::vector<int> findAllSprites(std::vector<unionSector>::iterator the_sector);    
    std::vector<int> find_owner_sprites(int owner);

    int makeExplosiveSector(std::stringstream&);
    int makeElevatorSector(std::stringstream&);
    int makeDoomDoors(std::stringstream&);
    int makeEnterSensor(std::stringstream&);
    int makeSlideSector(std::stringstream&);
    int makeController(std::stringstream&);
    int makeTROR(std::stringstream&);
    int makeRotateSector(std::stringstream&);
    int makeSlideDoors(std::stringstream&);
    int makeLighting(std::stringstream& refer);
    int makeQuotes(std::stringstream&);
    int remove_wall(int, std::stringstream&, bool);
    int makeSDSS(std::stringstream&);     // Slide Doors Sprite System = SDSS;
    int slideROR(std::stringstream& msg);
    int get_wall_maplevel(int, int, int, std::stringstream&);
    int division_wall_maplevel(int, int, int, bool);
    int wall_subdivision_ror(int, int, std::stringstream&);
    int prepareSlideSector(std::stringstream& refer);
    int makeZMotionSrite(std::stringstream& refer);
    int makeStarTrackDoors(std::stringstream&);
public:
    int read(char *filename, std::stringstream&);
    int read_text_file_to_string(const char*, std::string&, std::stringstream&);
    int write(char *filename, std::stringstream&);
    int write_v7B(char* filename, std::stringstream&);
    int write_obj(int, char*filename, std::stringstream&);
    int get_prv_point(int, int);
    int prepare(std::stringstream&);
	int finish();
    int check(std::stringstream&);
    void rm() {wV.erase(wV.begin(), wV.end());spV.erase(spV.begin(), spV.end()); sV.erase(sV.begin(), sV.end());}
    int processing(std::stringstream&, const float);

    void show(std::stringstream&);
    void printSector(int num, bool blood,std::stringstream&);

    bool isEmpty() { if ((int)sV.size() == 0) return true; else return false;};
    int getSectors() {return (int)sV.size();};
    int getWalls() {return (int)wV.size();}
    int getSprites() {return (int)spV.size();}
};

/*
AMMO lotag in Blood
60 - Spray can

62 - Bundle of TNT
63 - Case of TNT
64 - Proximity Detonator
65 - Remote Detonator
66 - Trapped Soul
67 - 4 shotgun shell
68 - Box of shotgun
69 - few bullets
70 - Voodoo Doll

72 - few tram of balle
73 - Tesla Charge
76 - Flares
79 - Casoline Can

41 - Sawed Off
42 - Tommy Gun
43 - Flare Pistol
45 - Tesla Cannon
46 - Napaln Lancher

ITEMS
100 - Skull key
103 - Dagger key
101 - Eye key
102 - Fire key
105 - Moon key
104 - Spider key
109 - Life Essence
107 - Doctor's Bag
110 - Life Seed
113 - Limited invisibili
114 - INVULNERABILITY
115 - Boots of Jumping
117 - Guns Akimbo
124 - Reflective shots
121 - Cristal Ball
118 - Diving Suit
125 - Beast Vision
127 - Rage Vision
128 - Dilirium Shroom
129 - Grow shroom
138 - Wooden Chest
137 - Black Chest
130 - Shrink shroom
136 - Tome
143 - Spirit Armor
144 - Super Armor
141 - Body Armor
140 - Basic Armor
142 - Fire Armor


ENEMIES
202 - Cultist w/Shotgun
201 - Cultist w/Tommy
230 - TCultist prone
203 - Axe Zombie
206 - Flash Gargoyle
244 - Sleep Zombie
205 - Earth Zombie
204 - Fat Zombie
208 - Flash Statue
210 - Phantoms
211 - Hound
212 - Hand
216 - Matter Spider
215 - Black Spider
213 - Brown Spider
214 - Red Spider
220 - Rat
219 - Bat
218 - Eel
217 - Gill Bears
227 - Cerberus
222 - Green Tentacle
229 - Tehernobog
221 - Green Pod
245 - Innocent
251 - Beast
250 - Tiny Galeb

Gib Objects

0	GlassT   <==> 10 бесцветное стекло
1	GlassS   <==> 10 бесцветное стекло
2	BurnShard  <==> 6 гоящая бумага
3	WoodShard <==> 5 деревянный осколок
4	MetalShard <==> 9 металический осколок
5	FireSpark <==> 1 искры
6	ShockSpark <==> 2 искры с дымом
7	BloodChunks <==> 18 мясо
8	BubblesS <==> 10 бесцветно стекло
9	BubblesM <==> 10 бесцветно стекло
10	BubblesL <==> 10 бесцветное стекло
11	Icicles  <==> 10 бесцветное стекло
12	GlassCombo1 <==> 17 разноцветное стекло
13	GlassCombo2 <==> 17 разноцветное стекло
14	WoodCombo  <==>  5 дерево
15	kGibHuman  <==> 18 мясо
16	MedicCombo  <==> 18 мясо
17	FlareSpark  <==> 3 грящее дерево
18	BloodBits   <==> 15 кровь
19	RockShards <==> 4 каменные осколки
20	PaperCombo1 <==> 7 бежевый осколок
21	PlantCombo1 <==> 7 бежевый осколок
22	ShockGibs   <==> 1 искры
23	ShockGibs2  <==> 1 искры
24	ShockGibs3  <==> 1 искры
25	Flames1 (PLASMA ONLY) <==> 3 горящее дерево
26	Flames2 (PLASM ONLY) <==> 3 горящее дерево
27	AxeZombieHead  <==> 18 мясо
28	Mime <==> 18 мясо
29	Hound <==> 18 мясо
30	FleshGargoyle <=> 18 мясо
*/

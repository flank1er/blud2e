/* Convertor MAP files by Blood video game, Monolith 1997(c) to eduke32 map format
   Project: Blood CrossMatching
   Author: flanker
   It's fork by blud2b.c, HTTP://blood.sourceforge.net with additional features
   License GPL v.3
*/

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <set>
#include <blud2e.h>
#include <glm/glm.hpp>

class CHANNEL
{
    std::set<int> the_list;
public:
    void add(int value) {the_list.insert(value);};
    int operator()(int ch = -1)
    {
        if (ch < 100 ||  the_list.count(ch))
        {
            ch=500;
            while (the_list.count(ch))
                ++ch;
        };
        the_list.insert(ch);
        return ch;
    };
};

class COLLECT
{
    std::set<int> the_list;
public:
    int get_size() {return (int)the_list.size();};
    void erase(){the_list.erase(the_list.begin(), the_list.end());};
    void print() { for(auto i: the_list) SC << " " << i;}
    void moveIt(std::vector<unionWall>& walls, std::vector<unionSector>& sectors,
        std::vector<unionSprite>& sprites, glm::vec4& d)
    {
        for(auto it=sectors.begin(); it != sectors.end(); it++ ) if (the_list.count(it-sectors.begin()))
        {
            it->floor.z   +=d.z;
            it->ceiling.z +=d.z;
            for(auto k=it->firstWall; k<(it->firstWall+it->wallnum); k++)
            {
                k->pos.x +=d.x;
                k->pos.y +=d.y;
            };
        };
        for(auto it=sprites.begin(); it != sprites.end(); it++ )
            if (the_list.count(it->inSector - sectors.begin()))
                it->pos +=d;
    };

    bool isBroken(std::vector<unionSprite>& sprites)
    {
        int cnt=0;
        for(auto i:sprites)
            if (the_list.count(i.sectnum) && (i.isType("Lower stack") || i.isType("Upper stack")))
                cnt++;
        return ((cnt != 1 ) ? true : false);
    };
    void operator()(std::vector<unionWall>& walls, std::vector<unionSector>& sectors, std::vector<unionSector>::iterator room)
    {
        the_list.insert(room-sectors.begin());
        for (auto it=room->firstWall; it <(room->firstWall+room->wallnum); it++)
            if (it->nextsector >= 0 && !the_list.count(it->nextsector))
                this->operator()(walls, sectors,(sectors.begin()+it->nextsector));
    };
    bool operator==(COLLECT& operand) {return ((this->get_size() == operand.get_size()) ? true : false);};
    bool operator!=(COLLECT& operand) {return ((this->get_size() != operand.get_size()) ? true : false);};
    bool operator<(COLLECT& operand) {return ((this->get_size() < operand.get_size()) ? true : false);};
    bool operator>(COLLECT& operand) {return ((this->get_size() > operand.get_size()) ? true : false);};
};

struct auxTROR{
    int up;
    int down;
    glm::vec4 diff;
};

CHANNEL channel;

std::map<int, int> type_to_pic{
	{201, 7940}, {202, 7945}, {203, 6290}, {204, 6490}, {205, 8174}, {206, 6590}, {207, 0}, {208, 6650},
	{209, 0}, {210, 8180}, {211, 6390}, {212, 7100}, {213, 7040}, {214, 7045}, {215, 7055}, {216, 7050},
	{217, 6690}, {218, 6990}, {219, 7068}, {220, 6865}, {221, 6912}, {222, 6917}, {223, 0}, {224, 0},
	{225, 0}, {227, 7800}, {228, 0}, {229, 8260}, {230, 8505}, {231, 0}, {239, 0}, {240, 0}, {241, 0},
	{242, 0}, {243, 0}, {244, 6329}, {245, 8918}, {246, 0}, {247, 0}, {248, 0}, {249, 0},  {250, 8990},
	{251, 8080}, {252, 0}, {253, 0}};

std::map<int, int> Gibs{{0,10},{1,10}, {2,6}, {3,5}, {4,9}, {5,1}, {6,2}, {7,18}, {8,10}, {9,10}, {10,10},
	{11,10}, {12,17}, {13,17}, {14,5}, {15,18}, {16,18}, {17,3}, {18,15}, {19,4}, {20,7}, {21,7}, {22,1},
	{23,1}, {24,1}, {25,3}, {26,3}, {27,18}, {28,18}, {29,18}, {30,18}};

// ammo, weapons, items
std::set<int> items{60, 62, 63, 64, 65, 66, 67, 68, 69, 70, 72, 73, 76, 79, 43,
	41, 42, 46, 50, 45, 109, 107, 110, 113, 114, 115, 117,
	124, 121, 118, 125, 127, 128, 129, 138, 137, 130, 136, 143, 144, 141, 140, 142};

// enemies
std::set<int> enemies{202, 201, 230, 203, 206, 207, 244, 205, 204, 208, 210, 211, 212,
	216, 215, 213, 214, 220, 219, 218, 217, 227, 222, 229, 221, 245, 251, 250};

// keys: skull, dagger, eye, fire,
std::set<int> keys{100,101, 102, 103,104, 105};

//////////////////////////////////////////////////////////////////////////////////////
// if points A, B and  C belong a one line
bool isLine(glm::vec3 A, glm::vec3 B, glm::vec3 C)
{
    // Equation of a Line from 2 Point
    float r;
    if (A.x != B.x)
    {
        r=((C.x-A.x)*(B.y-A.y))/(B.x-A.x)+A.y;
        return (r == C.y) ? true : false;
    };
    if (A.y != B.y)
    {
        r=((C.y-A.y)*(B.x-A.x))/(B.y-A.y)+A.x;
        return (r == C.x) ? true : false;
    };
    return true;
};

glm::vec3 unionWall::get_normal()
{
    glm::vec3 b=pos;
    glm::vec3 a=prePoint->pos;
    glm::vec3 c=nextPoint->pos;

    if (!isLine(prePoint->pos, pos, nextPoint->pos))
        return glm::normalize(glm::cross((b-a),(c-b)));
    return glm::vec3(0.f);
};

float unionSector::get_floorZ(float x, float y)
{
    float R=((float)floorz)*(1.f/(512.f*16.f));
    R +=(firstWall->gis.x - x)*floor.delta.x+(firstWall->gis.y - y)*floor.delta.y;
    return R;
};

float unionSector::get_ceilingZ(float x, float y)
{
    float R=((float)ceilingz)*(1.f/(512.f*16.f));
    R +=(firstWall->gis.x - x)*ceiling.delta.x+(firstWall->gis.y - y)*ceiling.delta.y;
    return R;
};

std::vector<unionWall>::iterator get_nearest_wall(std::vector<unionWall>&  walls, std::vector<unionWall>::iterator loop, glm::vec3 point)
{
    auto ret=loop;  auto it=loop;
    do{
        if (glm::distance(it->pos, point) < glm::distance(ret->pos, point))
            ret=it;
        it=it->nextPoint;
    } while(it != loop);
    return ret;
};

static std::vector<unionSprite>::iterator findTSprite(int request, std::vector<unionSprite>& V)
{
    auto it=V.begin();
    while (it != V.end() && it->lotag != request)
        it++;
	return it;
};

std::vector<int> Map::findAllSprites(std::vector<unionSector>::iterator the_sector)
{
    std::vector<int> the_vector;
    for(int i=0; i < (int)spV.size(); i++)
        if (spV.at(i).inSector == the_sector)
            the_vector.push_back(i);
    return the_vector;
};

glm::vec3 getCenter(std::vector<unionWall>::iterator it)
{
    glm::vec3 ret=glm::vec3(0.f);
    int counter=0;
    for_loop(it, [&counter, &ret](unionWall _s) {
        ret +=_s.pos;
        counter++;
    });
    ret /=counter;

    return ret;
};

float getArea(std::vector<unionWall>::iterator iter, glm::vec3 point)
{
    float ret=0.f;
    for_loop(iter, [&ret, &point](unionWall _s){
            // Heron's formula
            float a=glm::distance(_s.pos, _s.nextPoint->pos);
            float b=glm::distance(_s.pos, point);
            float c=glm::distance(_s.nextPoint->pos,point);
            float p=(a+b+c)/2.f;
            ret=sqrt(p*(p-a)*(p-b)*(p-c));
        });
    return ret;
};

glm::vec3 Map::getRndPosition(std::vector<unionSector>::iterator the_sector)
{
    glm::vec3 ret=glm::vec3(0.f);
    if (the_sector->generic->property("proper"))
    {
        ret=getCenter(the_sector->generic->marker);
        float S=getArea(the_sector->generic->marker, ret);
        float xMax=ret.x;
        float xMin=ret.x;
        float yMax=ret.y;
        float yMin=ret.y;
        for_loop(the_sector->generic->marker, [&xMax, &xMin, &yMax, &yMin](unionWall _s){
            if(xMax < _s.pos.x) xMax=_s.pos.x;
            if(xMin > _s.pos.x) xMin=_s.pos.x;
            if(yMax < _s.pos.y) yMax=_s.pos.y;
            if(yMin > _s.pos.y) yMin=_s.pos.y;
        });

        assert(xMax > xMin && yMax>yMin);
        float dx=xMax-xMin;
        float dy=yMax-yMin;
        float rndS;
        do {
            ret.x=(rand() % (int)dx)+xMin;
            ret.y=(rand() % (int)dy)+yMin;
            rndS=getArea(the_sector->generic->marker, ret);
        } while(rndS > S);
    } else
    {
        auto sprites=findAllSprites(the_sector);
        if ((int)sprites.size() > 0 && !the_sector->is("Rotate") )
        {
            ret.x=spV.at(*sprites.begin()).pos.x;
            ret.y=spV.at(*sprites.begin()).pos.y;
        } else
        {
            int n=0; for_loop(the_sector->generic->marker, [&n](unionWall _s) { n++;});
            auto m_begin=the_sector->generic->marker+(rand() % n);
            do {
                static auto mrk=m_begin;
                glm::vec3 nr=mrk->get_normal();
                if (nr.z > 0)
                    ret=(mrk->prePoint->pos+mrk->nextPoint->pos)/2.f;
                mrk=mrk->nextPoint;
            } while(ret == glm::vec3(0.f));

            glm::vec3 p0=the_sector->generic->marker->pos;
            glm::vec3 p1=the_sector->generic->marker->nextPoint->nextPoint->pos;
            ret=(p0+p1)/2.f;
        };
    };
    return ret;
};

template<typename T> int get_done(T& t)
{
    int ret=0;
    for(auto it : t)
        if (it.done)
            ret++;
    return ret;
};

template<typename T> std::vector<int> findTx(T& t, int TX, int RX = -1, int lotag = -1, bool for_all=false) {

    std::vector<int> ret;
	if (TX > 0)
	{
		for (auto it=t.begin(); it != t.end(); ++it)
		{
			if ( !for_all && it->over && it->txID == TX && !it->done &&
                (RX < 0 ||it->rxID == RX) &&	(lotag < 0 || it->lotag == lotag ))
                ret.push_back(it-t.begin());
            else if (for_all && it->over && it->txID == TX &&
                (RX < 0 ||it->rxID == RX) &&	(lotag < 0 || it->lotag == lotag ))
                ret.push_back(it-t.begin());
		};
	};
	return ret;
};

template<typename T> std::vector<int> findRx(T& t, int RX, int TX = -1, int lotag = -1, bool for_all=false) {

    std::vector<int> ret;
	if (RX > 0)
	{
		for (auto it=t.begin(); it != t.end(); ++it)
		{
			if ( !for_all && it->over && it->rxID == RX && !it->done &&
                (TX < 0 ||it->txID == TX) &&	(lotag < 0 || it->lotag == lotag ))
                ret.push_back(it-t.begin());
            else if (for_all && it->over && it->rxID == RX &&
                (TX < 0 ||it->txID == TX) &&	(lotag < 0 || it->lotag == lotag ))
                ret.push_back(it-t.begin());
		};
	};
	return ret;
};

int getKey(std::vector<unionSector>::iterator sec)
{
    int ret=sec->key;
    if (ret == 0){
        for_loop(sec->generic->marker, [&ret] (unionWall _s)
        {
            if (_s.nextWall->over && _s.nextWall->key >0 && ret>0 && _s.nextWall->key!= ret)
                SC << "ERROR: found collision in wall key! key: " << ret << " wall key: " << _s.nextWall->key <<SE;
            else if ((_s.nextWall->over && ret == 0 && _s.nextWall->key > 0)  || (_s.over && ret==0 && _s.key >0))
                ret=_s.nextWall->key;
        });
    };
    return ret;
};

int Map::addSprite(int room,  int rxChannel, int txChannel ,std::string name, glm::vec4 p=glm::vec4(0.f))
{
    assert(room < (int)sV.size());
    if (sV.at(room).over)
        assert (sV.at(room).refer == room);
    unionSprite nsp=spV.at(0);
    auto it=sV.begin()+room;
    if (room >= 0)
    {
        nsp.pos=glm::vec4(getRndPosition(sV.begin()+room), 0.f);
        //nsp.pos.z=(sV.at(room).floor.z+sV.at(room).ceiling.z)/2;
        nsp.pos.z=sV.at(room).floor.z;
    } else
        return -1;

    if (p != glm::vec4(0.f))
        {nsp.pos.x=p.x; nsp.pos.y=p.y;};

    nsp.inSector=sV.begin()+room;
    nsp.over=false;
    nsp.tag=glm::ivec3(0, 0,-1);
    nsp.vel=glm::ivec3(0);
    nsp.cstat=0x01;
    nsp.texture_id=1;
    nsp.owner=nsp.shade=nsp.xoffset=nsp.yoffset=nsp.ang=nsp.pal=nsp.statnum=0;
    nsp.xrepeat=nsp.yrepeat=64;

    if ( name == "SE13")
    {
        nsp.tag=glm::ivec3(13, rxChannel, -1);
           spV.push_back(nsp);
    } else if (name == "SE10")
    {
        nsp.tag=glm::ivec3(10, 128, -1);
        nsp.ang=512;
        nsp.vel.x=rxChannel;
        spV.push_back(nsp);
    } else if (name == "SE70")
    {
		nsp.tag=glm::ivec3(70,rxChannel, -1);
		nsp.ang=512;
		nsp.vel.x=rxChannel;
        nsp.pos.z=sV.at(room).offCeilZ;
        spV.push_back(nsp);
    } else if (name == "SE60")
    {
        nsp.tag=glm::ivec3(60, txChannel, -1);
        nsp.vel.x=rxChannel;
        spV.push_back(nsp);
	} else if (name == "SE69")
	{
        nsp.tag=glm::ivec3(69,rxChannel, -1);
        nsp.vel.x=rxChannel;
		nsp.ang=512;
		nsp.pos.z=sV.at(room).offFloorZ;
        spV.push_back(nsp);
    } else if (name == "SE64")
    {
        nsp.tag=glm::ivec3(64, rxChannel, -1);
        nsp.ang=txChannel;
        nsp.vel.x=rxChannel;
        spV.push_back(nsp);
    } else if (name == "GPSPEED")
    {
        nsp.texture_id=10;
        nsp.tag=glm::ivec3(p.x, rxChannel,-1);
        nsp.ang=txChannel;
        nsp.vel.x=rxChannel;
        spV.push_back(nsp);
    } else if (name == "TC_LOCKER")
    {
        nsp.tag=glm::ivec3(txChannel, 30, -1);
		nsp.vel=glm::ivec3(0, rxChannel, 1);
		nsp.texture_id=17;
		if (it->over && it->is("MotionZ") && !it->done && (it->onFloorZ != it->offFloorZ))
            nsp.pos.z=sV.at(room).onFloorZ;
        if (it->over && it->is("MotionZ") && !it->done && (it->onCeilZ != it->offCeilZ))
            nsp.pos.z=sV.at(room).onCeilZ;
        spV.push_back(nsp);
    } else if (name == "MovePoint")
    {
        nsp.texture_id=14;
        nsp.tag=glm::ivec3(txChannel, txChannel, -1);
        nsp.vel.x=rxChannel;
        spV.push_back(nsp);
    } else if (name == "SWITCH")
    {
        nsp.texture_id=6166;
        nsp.cstat=0;
        nsp.vel=glm::ivec3(rxChannel, txChannel, 7);
        spV.push_back(nsp);
    } else if (name == "PushTrigger")
    {
        nsp.texture_id=18;
        nsp.vel.z=nsp.owner=1;
        nsp.vel.y=rxChannel;
        spV.push_back(nsp);
    } else if (name == "Wall Crack")
    {
        nsp.texture_id=546;
        nsp.tag.g= rxChannel;
        nsp.cstat=0x202;
        spV.push_back(nsp);
    };

    return 0;
};

int Map::makeExplosiveSector() {
	int last=get_done(sV);
    std::set<int> boom_set;
    for(auto& T:sV)
    {
        if (T.is("MotionZ") && T.rxID >= 100 && !T.done)
        {
            auto the_list=findRx(spV, T.rxID);
            for(auto spr : the_list)
                if (spV.at(spr).isType("Hidden Exploder"))
                    boom_set.insert(T.refer);

            the_list=findTx(spV, T.rxID);
            for(auto spr: the_list)
                if(spV.at(spr).isType("Wall Crack") || spV.at(spr).isType("Gib Object"))
                    boom_set.insert(T.refer);
        };
    };

    std::cout << "Boom Sectors: "; for(auto T: boom_set) std::cout<< T << " "; std::cout << std::endl;

    for(auto T: boom_set)
    {
        assert(T == sV.at(T).refer);
    	sV.at(T).ceiling.z=sV.at(T).onCeilZ;
        sV.at(T).floor.z=sV.at(T).onFloorZ;
        int ch=sV.at(T).rxID;
        channel.add(ch);
        addSprite(T, ch, 0, "SE13");
        auto the_list=findRx(spV, sV.at(T).rxID);
        for (auto spr: the_list)
            if (spV.at(spr).isType("Hidden Exploder"))
            {
                spV.at(spr).tag.g=ch;
                spV.at(spr).texture_id=1247;
            };
        the_list=findTx(spV, sV.at(T).rxID);
        for (auto spr: the_list)
            if (spV.at(spr).isType("Wall Crack"))
            {
                spV.at(spr).tag.g=ch;
                spV.at(spr).texture_id=546;
            } else if (spV.at(spr).is("FireExt"))
            {
                spV.at(spr).texture_id=916;
                addSprite(T,ch, 0, "Wall Crack", spV.at(spr).pos);
            };
        sV.at(T).done=true;
    };

    int ret =get_done(sV)-last;
	std::cout << "was done: " << ret << " BOOM Sectors" << std::endl;
	return ret;
};

int Map::makeElevatorSector()
{
	int last=get_done(sV);
    std::set<int> elevator_set;

    for(auto& T:sV)
        if (T.is("MotionZ") && !T.done && (T.floor.z -T.ceiling.z) == (T.onFloorZ -T.onCeilZ) &&
                                          (T.floor.z -T.ceiling.z) == (T.offFloorZ -T.offCeilZ))
        {
            elevator_set.insert(T.refer);
            if ((!T.state && T.floor.z >= T.offFloorZ) || (T.state && T.floor.z >= T.onFloorZ))
                T.tag.r=19; else  T.tag.r=18;
            auto the_list=findAllSprites(sV.begin()+T.refer);
            for (auto k: the_list)
                if (spV.at(k).is("SectorSFX"))
                    spV.at(k).texture_id=5;
            T.done=true;
        };

    std::cout<< std::endl << "Elevator Sectors: "; for(auto T: elevator_set) std::cout<< T << " "; std::cout << std::endl;

    int ret =get_done(sV)-last;
	std::cout << "was done: " << ret << " Elevator Sectors" << std::endl;
	return ret;
};

int Map::makeDoomDoors()
{
	int last=get_done(sV);
    std::set<int> floor_set;

    for(auto T: sV)
    {
       if ((T.over && T.is("MotionZ") && !T.done && (T.onFloorZ != T.offFloorZ)) ||
             (T.over && T.is("MotionZ") && !T.done && (T.onCeilZ != T.offCeilZ)))
            floor_set.insert(T.refer);
    };

    std::cout<< std::endl << "FloorDoor Sectors: "; for(auto T: floor_set) std::cout<< T << " "; std::cout << std::endl;
    //std::cout<< "CeilingDoor Sectors: "; for(auto T: ceiling_set) std::cout<< T << " "; std::cout << std::endl;

    for(auto S: floor_set)
    {
        assert(S == sV.at(S).refer);
        bool is_floor=false;
        auto it=sV.begin()+S;
        int ch=channel(it->rxID);
        int chX=channel();

        it->done=true;
        it->log=glm::ivec2(ch,chX);

        auto pair_list=findTx(sV, it->rxID, -1, -1, true);
        for (auto iter=pair_list.begin(); iter != pair_list.end();)
            if (floor_set.count(*iter))
                iter++;
            else
                pair_list.erase(iter);

        auto ps=it;
        if (pair_list.size() >0)
        {
            std::cout<< "pair sector: " << S <<" <-- ";
            for(auto j: pair_list) std::cout<< j << " ";
            std::cout << std::endl;
            ps=sV.begin() + *pair_list.begin();
        };

        if (it->onFloorZ != it->offFloorZ)
            is_floor=true;

        addSprite(S,ps->log.x,chX, "SWITCH");
        addSprite(S,ps->log.x,it->key, "TC_LOCKER");
        auto the_list=findAllSprites(it);
        for (auto i: the_list)
            if(spV.at(i).is("SectorSFX"))
            {
                spV.at(i).vel.x=chX;
                spV.at(i).texture_id=5;
            };

       if (is_floor)
            addSprite(S, chX, 0, "SE69");
        else
            addSprite(S, chX, 0, "SE70");

        if (it->busy > 0)
            addSprite(S,chX,ps->busyTime, "MovePoint");
        else
            addSprite(S,chX,512, "MovePoint");

        if (it->waitTime > 0)
            addSprite(S, chX, ps->waitTime, "SE10");

        it->floor.z=it->onFloorZ;
        it->ceiling.z=it->onCeilZ;
    };

    int ret =get_done(sV)-last;
	std::cout << "was done: " << ret << " DoomDoor Sectors" << std::endl;
	return ret;
};

int Map::makeEnterSensor()
{
	int last=get_done(sV);
    std::set<int> sectors_set;

    for(auto T: sV)
        if (T.over && !T.done && !LT && !T.rxID && T.txID >= 100 && T.triggerEnter)
            sectors_set.insert(T.refer);

    std::cout<< std::endl << "EnterTrigger Sectors: ";
    for(auto T: sectors_set) std::cout<< T << " "; std::cout << std::endl;

    for(auto T: sectors_set)
    {
        assert(T == sV.at(T).refer);
        auto targets=findRx(sV, sV.at(T).txID, -1, -1, true);
        int ch=channel(sV.at(T).txID);

        if (targets.size() >0) std::cout << "found link: " << T << " <--> ";
        for(auto S: targets)
        {
            assert(sV.at(S).refer == S);
            static int x=sV.at(S).log.x;
            if (sV.at(S).log.x != x)  // WARNING
                std::cout << std::endl << " was found collision in logical level: " << T << " <--> " << S<< std::endl;
            if (sV.at(S).done)
                ch=sV.at(S).log.x;
            std::cout << S << " ";

        };
        if (targets.size()>0) std::cout << std::endl;
        addSprite(T, ch, 0, "PushTrigger");
        sV.at(T).done=true;
        sV.at(T).log=glm::ivec2(0, ch);
    };

    int ret =get_done(sV)-last;
	std::cout << "was done: " << ret << " Trigger Sectors" << std::endl;
	return ret;
};

int Map::makeSlideSector()
{
	int last=get_done(sV);
    std::set<int> sector_list;
    for(auto T: sV)
        if(T.over && !T.done && T.is("Slide"))
            sector_list.insert(T.refer);

    std::cout<< std::endl << "found Slide Sectors: ";
    for(auto T: sector_list) std::cout<< T << " "; std::cout << std::endl;

    for(auto T: sector_list)
    {
        assert(sV.at(T).refer == T);
        std::vector<unionSector>::iterator it=sV.begin()+T;
        if (sV.at(T).loops.size() ==1 && sV.at(T).generic->property("inner") )
        {
            assert(it->marker0 > 0 && it->marker1 >0 );
            assert(spV.at(it->marker0).xvel ==it->marker0 && spV.at(it->marker1).xvel == it->marker1);

            float dx=spV.at(it->marker0).pos.x -spV.at(it->marker1).pos.x;
            float dy=spV.at(it->marker0).pos.y -spV.at(it->marker1).pos.y;
            // Move to ON state
            if (!sV.at(T).state)
            {
                for_loop(sV.at(T).generic->marker, [&](unionWall & _s) mutable {
                    _s.pos +=glm::vec3(dx,dy, 0.f);
                    _s.nextWall->pos +=glm::vec3(dx,dy, 0.f);
                });
                for(auto& S: spV)
                    if (S.sectnum == T)
                        S.pos +=glm::vec4(dx,dy,0.f, 0.f);
            };

            int ch=channel(it->rxID);
            int angle=0;

            auto the_list=findAllSprites(it);
            for(auto S: the_list)
            {
                auto iter=spV.begin()+S;
                if (iter->over && iter->is("SectorSFX"))
                {
                    iter->texture_id=5;
                    iter->vel.x=ch;
                };
            };
            if (abs(spV.at(it->marker0).x-spV.at(it->marker1).x)<32 && spV.at(it->marker0).y >= spV.at(it->marker1).y)
					angle=512;
            else if (abs(spV.at(it->marker0).x-spV.at(it->marker1).x)<32 && spV.at(it->marker0).y <= spV.at(it->marker1).y)
					angle=1536;
            else if (spV.at(it->marker0).y == spV.at(it->marker1).y && spV.at(it->marker0).x >= spV.at(it->marker1).x)
					angle=1024;

            addSprite(T, ch, angle, "SE64");
            addSprite(T,ch,angle, "GPSPEED");
            lastSprite.tag.x=sqrt(dx*dx+dy*dy); // lotag
            addSprite(T,ch, 0, "TC_LOCKER");
            it->tag.g=ch;
            it->log=glm::ivec2(ch,0);
            it->done=true;
        };
    };

    int ret =get_done(sV)-last;
	std::cout << "was done: " << ret << " Slide Sectors" << std::endl;
	return ret;
};

int Map::makeController()
{
    int last=get_done(spV);
    std::set<int> the_list;
    std::cout << std::endl << "Controles was found: ";
    for(auto T:spV) if(T.over && !T.done && T.txID>=100 && T.rxID >=100)
    {
        std::cout << T.refer<< " ";
        the_list.insert(T.refer);
    }; std::cout<<std::endl;

    std::cout << "set controller: ";
    for(auto T:the_list)
    {
        auto it=spV.begin()+T;
        auto sensors_list=findTx(sV, it->rxID, -1, -1, true);
        auto activators_list=findRx(sV, it->txID, -1,-1,true);
        if (sensors_list.size() > 0 && activators_list.size() >0 )
        {
            std::set<int> rx, tx;
            for(auto i:sensors_list)  rx.insert(sV.at(i).log.y);
            for(auto i:activators_list) tx.insert(sV.at(i).log.x);
            rx.erase(0); tx.erase(0);
            if (rx.size() > 1 || tx.size()>1 )
            {
                std::cout << std::endl << "is collison: " << it->refer << std::endl;
            } else  if (rx.size() == 1 && tx.size() == 1)
            {
                std::cout << it->refer << "( ";
                for(auto i:sensors_list) std::cout << i << " "; std::cout << "-->";
                for(auto i:activators_list) std::cout << i << " "; std::cout << ") ";

                it->texture_id=9117;  // remove this sprite
                addSprite((it->inSector-sV.begin()), *rx.begin(), *tx.begin(), "SWITCH");
                // if sensor is eneterSector
                for (auto i : sensors_list)
                {
                    auto sprites=findAllSprites(sV.begin()+i);
                    auto isTrigger=sV.end();
                    for(auto k:sprites)
                        if (spV.at(k).texture_id == 18)
                            isTrigger=spV.at(k).inSector;
                    if (isTrigger !=sV.end())
                    {
                        int level=0; if (!it->launch1) level +=1;
                        if (!it->launch2) level +=2; if (!it->launch3) level +=4;
                        if (!it->launch4) level +=8; if (!it->launch5) level +=16;
                        addSprite((isTrigger-sV.begin()), isTrigger->log.y, level , "SE60");
                        std::cout << std::endl << "add sprite SE60 in: " << isTrigger -sV.begin() << " launch level: " <<level;
                    };
                };
                it->done=true;
            };
        };
    };

    int ret =get_done(spV)-last;
	std::cout << std:: endl << "was done: " << ret << " Controllers" << std::endl;
	return ret;
};

int Map::makeTROR()
{
    std::vector<auxTROR> aux;
    int ret=0;
    std::cout << std::endl << "ROR sectors was found:";
    for(auto T:spV) if(T.over && !T.done && T.isType("Upper stack"))
    {
        int s=T.inSector-sV.begin();
        //std::cout << " ("<< s<< "<-->";
        for(auto sp:spV) if(sp.over && !sp.done && sp.isType("Lower stack") && sp.data1 == T.data1)
        {
            auxTROR new_el;
            new_el.up=s;
            new_el.down=sp.inSector-sV.begin();
            new_el.diff=T.pos-sp.pos; // (Upper - Lower)
            new_el.diff.z=T.inSector->floor.z - sp.inSector->ceiling.z;
            aux.push_back(new_el);
            std::cout << " ("<< new_el.up<< "<-->" << new_el.down << ")";
        };
    }; std::cout<<std::endl;

    for (auto T: aux)
    {
        COLLECT up,down;
        up(wV, sV, (sV.begin()+T.up));
        down(wV, sV, (sV.begin()+T.down));
        if (up != down)
        {
            if (up < down && !up.isBroken(spV))
            {
                SC << "movement upper sectors:"; up.print(); SC<<SE;
                T.diff *=(-1);
            } else if (down <up &&  !down.isBroken(spV))
            {
                SC<<"movement lower sectors:"; down.print(); SC<<SE;
                down.moveIt(wV, sV, spV, T.diff);
            }
        } else if (up == down && T.diff.z == 0 && (sV.at(T.up).generic->property("inner") || sV.at(T.down).generic->property("inner")))
        {
            auto itS=sV.begin();
            if (sV.at(T.up).generic->property("inner"))
            {
                SC << "movement upper sectors: " << T.up<<SE;
                T.diff *=(-1);
                itS +=T.up;
            } else if (sV.at(T.down).generic->property("inner"))
            {
                SC << "movement down sectors: " << T.down<<SE;
                itS +=T.down;
            };
            assert(itS->loops.size()==1);
            for_loop(itS->generic->marker, [&](unionWall & _s) mutable {
                _s.pos +=glm::vec3(T.diff.x,T.diff.y, 0.f);
                _s.nextWall->pos +=glm::vec3(T.diff.x,T.diff.y, 0.f);
            });
            for(auto& S: spV)
                if (S.inSector == itS)
                    S.pos +=glm::vec4(T.diff.x,T.diff.y,0.f, 0.f);
        } else { SC << "ERROR: failure TROR processing!" <<SE; continue; };

        auto d=sV.begin()+T.down;
        auto u=sV.begin()+T.up;
        assert(d->loops.size() == 1);
        assert(u->loops.size() == 1);
        if (u->wallnum != d->wallnum){  SC << "ERROR: incorrect amount of wall!" <<SE;  continue;};

        auto it=u->firstWall;
        auto pair_wall=get_nearest_wall(wV, d->firstWall, it->pos);
        do {
            if (it == u->firstWall)
                it->cstat=pair_wall->cstat=16384;
            else
            {
                it->cstat +=2048;
                pair_wall->cstat +=1024;
            };
            it->tag.z=pair_wall-wV.begin(); // extra tag
            pair_wall->tag=glm::ivec3(it-wV.begin(), 0, -1); // lotag
            pair_wall=pair_wall->nextPoint;  it=it->nextPoint;
        } while (it != u->firstWall);

        d->ceilingstat +=1024;
        u->floorstat +=1024;
        dh.version=9;  // TROR MAP format;
        ret++;
    };
	std::cout  << "was done: " << ret << " TROR Sectors" << std::endl;
	return ret;
};

int Map::makeRotateSector()
{
	int last=get_done(sV);
    std::set<int> sector_list;
    for(auto it=sV.begin(); it != sV.end(); it++)
        if(it->over && !it->done && it->is("Rotate"))
            sector_list.insert(it-sV.begin());

    std::cout<< std::endl << "found Rotate Sectors: ";
    for(auto T: sector_list) std::cout<< T << " "; std::cout << std::endl;

    for(auto T: sector_list)
    {
        auto it=sV.begin()+T;
        if (it->loops.size() ==1 && it->generic->property("inner") )
        {
            assert(it->marker0 > 0);
            auto mrk=spV.begin()+it->marker0;
            assert(mrk->owner == T);
            int ch=channel(it->rxID);
            int chX=channel();
            int key=getKey(it);
            it->log=glm::ivec2(ch,chX);

            auto pair_list=findTx(sV, it->rxID, -1, -1, true);
            for (auto iter=pair_list.begin(); iter != pair_list.end();)
                if (sector_list.count(*iter))
                    iter++;
                else
                    pair_list.erase(iter);

            auto ps=it;
            if (pair_list.size() >0)
            {
                std::cout<< "pair sector: " << T <<" <-- ";
                for(auto j: pair_list) std::cout<< j << " ";
                std::cout << std::endl;
                ps=sV.begin() + *pair_list.begin();
            };

            mrk->tag=glm::ivec3(64,ch, -1);
            mrk->vel=glm::ivec3(ch,0, 1);
            mrk->owner=-1;
            mrk->texture_id=1;
            if (!it->state)
            {
                SC<<"Rotate Sector: "<< T <<SE;
                mrk->ang *=-1;
                float a=(M_PI*mrk->ang)/1024.0;
                for_loop(it->firstWall, [&](unionWall & _s) mutable {
               		float x1=_s.pos.x; float y1=_s.pos.y;
                    float x2=_s.nextWall->pos.x; float y2=_s.nextWall->pos.y;
                    _s.pos.x=           (mrk->pos.x + (x1 - mrk->pos.x) * cos(a) - (y1 - mrk->pos.y) * sin(a));
                    _s.nextWall->pos.x= (mrk->pos.x + (x2 - mrk->pos.x) * cos(a) - (y2 - mrk->pos.y) * sin(a));
                    _s.pos.y=           (mrk->pos.y + (y1 - mrk->pos.y) * cos(a) + (x1 - mrk->pos.x) * sin(a));
                    _s.nextWall->pos.y= (mrk->pos.y + (y2 - mrk->pos.y) * cos(a) + (x2 - mrk->pos.x) * sin(a));
                });
                for(auto& i:spV) if(i.inSector == it) if( i.texture_id != 1)
                {
                    float x1=i.pos.x; float y1=i.pos.y;
                    i.pos.x=(mrk->pos.x + (x1 - mrk->pos.x) * cos(a) - (y1 - mrk->pos.y) * sin(a));
                    i.pos.y=(mrk->pos.y + (y1 - mrk->pos.y) * cos(a) + (x1 - mrk->pos.x) * sin(a));
                };
            };
            auto the_list=findAllSprites(it);
            for(auto S: the_list) {
                auto iter=spV.begin()+S;
                if (iter->over && iter->is("SectorSFX")) {
                    iter->texture_id=5;
                    iter->vel.x=ch;
            };};
            addSprite(T,ps->log.x, key, "TC_LOCKER");
            addSprite(T,ch,1, "MovePoint");
            lastSprite.ang=0;
            lastSprite.tag.x=32;
            if (mrk->ang >0) lastSprite.vel.z=1;
            addSprite(T,ps->log.x,ch, "SWITCH");
            it->tag.g=ch;
            it->done=true;
        };
    };
    int ret =get_done(sV)-last;
	std::cout << "was done: " << ret << " Rotate Sectors" << std::endl;
	return ret;
};

int Map::makeSlideDoors()
{
	int last=get_done(sV);
    std::set<int> sector_list;
    for(auto it=sV.begin(); it != sV.end(); it++)
        if(it->over && !it->done && it->is("Slide Marked"))
            sector_list.insert(it-sV.begin());

    std::cout<< std::endl << "found Slide Doors: ";
    for(auto T: sector_list) std::cout<< T << " "; std::cout << std::endl;



    int ret =get_done(sV)-last;
	std::cout << "was done: " << ret << " Slide Doors" << std::endl;
	return ret;
};


int unionSprite::makeAmbient(int musicID, int radius)
{
		texture_id =5;
		tag=glm::ivec3(musicID, radius, -1);
		cstat=0;
        return 0;
};

void Map::Cstat()
{

    for (auto& T: spV)
    {
        std::set<int> bottle={
            5,
            5622,
            5690,
            5724,
            5725,
            5726,
            5879,
            5724,
            5737,
            5922,
            7640
        }; // disable Central Orientation
        std::set<int> getDown={
            5809,
            5810,
            6026,
            6035
        };

        if (bottle.count(T.texture_id))
        {
            T.cstat &=~(1<<7);
            continue;
        };

        glm::ivec2 size=targa[T.texture_id];
        int edge=size.y*2*T.yrepeat;

        if (getDown.count(T.texture_id))
        {
            T.pos.z+=edge;
            continue;
        };

        // if sprite is floor or wall aligned
        if ((T.cstat & (1<<5)) || ((T.cstat & (1<<4)) && T.z != T.inSector->ceilingz && T.z != T.inSector->floorz))
            continue;

        T.pos.z=T.inSector->floor.z;
        T.cstat &=~(1<<7);
    }

};

void Map::makeEnemies()
{
    for (auto& T : spV)
        if (T.over  && enemies.count(T.lotag))
        {
            if (T.launch1)
                LT=T.launch1+T.launch2+T.launch3+T.launch4+T.launch5+1; // skill of enemies

            if (T.launchS) T.texture_id=9117;

            if (T.key > 0)
            {
                int ch=channel();
                auto the_key=findTSprite(T.key+99, spV);
                if (the_key != spV.end())
                    T.vel.y=the_key->vel.x=ch;
            };
        };
};

void Map::makeSectorSFX(unionSprite& Sp)
{
    Sp.tag=glm::ivec3(sTable.trans(Sp.data1), sTable.trans(Sp.data2), -1);
    Sp.shade=Sp.pal=0;
    if (Sp.isType("SFX Gen"))  // make TriggerSound
    {
        Sp.texture_id=19;
        Sp.vel.z=1;
        Sp.tag=glm::ivec3(sTable.trans(Sp.data2), sTable.trans(Sp.data1), -1);
    };

    if ((Sp.data3 > 0 && Sp.data4 > 0) && (Sp.data2 != Sp.data4 || Sp.data1 != Sp.data3))
    {
        unionSprite nsp = Sp;
        nsp.tag=glm::ivec3(sTable.trans(Sp.data3), sTable.trans(Sp.data4), -1);
        nsp.vel.z=1;
        nsp.pos.z=Sp.inSector->floor.z;
        spV.push_back(nsp);
    };
};

void Map::makeRespawn(unionSprite& Sp)
{
    if (Sp.isType("Explode Object") || Sp.isType("Gib Object") || enemies.count(Sp.lotag) || Sp.isType("Dude Spawn"))
    {
        unionSprite reserved=Sp;
        Sp.texture_id=7197;
        Sp.cstat=0;
        Sp.tag.r=type_to_pic[Sp.data1];
        if ((Sp.launch1 + Sp.launch2 + Sp.launch3 + Sp.launch4 + Sp.launch5) > 0)
            Sp.tag.g=(1-Sp.launch1)+((1-Sp.launch2)<<1)+((1-Sp.launch3)<<2)+((1-Sp.launch4)<<3)+((1-Sp.launch5)<<4);
        if (reserved.texture_id !=7197)
        {
            Sp.tag.r=type_to_pic[Sp.dropId];
            spV.push_back(reserved);
        };
    };
};

void Map::makeExplodeAndGib()
{
    for(auto& Sp: spV)
        if (Sp.over && (Sp.isType("Explode Object")|| (Sp.isType("Gib Object")
            && !(Sp.cstat & 0x01) && Sp.texture_id != 5778 )))
        {
			Sp.tag=glm::ivec3(Gibs[Sp.data1], sTable.trans(Sp.data4), -1);
			Sp.statnum=15;
			Sp.clipdist=16;
			Sp.vel.z=2; // Only Explode
			if (Sp.data2 > 0 || Sp.data3 > 0)
			{
				Sp.vel.x=channel();
				unionSprite nsp=Sp;
				nsp.texture_id=5120;
				nsp.cstat |=0x202; // set transparent
				nsp.over=false;
				nsp.xrepeat=nsp.yrepeat=2; //
				nsp.tag.r=Gibs[Sp.data2];
				spV.push_back(nsp);
				if ( Sp.data3 > 0 )
				{
					nsp.tag.r=Gibs[Sp.data3];
					spV.push_back(nsp);
				};
			}
        };
};

int Map::prepare()
{
	// din't use music ID as channel
	for (auto it=sTable.target.begin(); it != sTable.target.end(); ++it)
		channel.add((*it).second); // reserved channels

	for (auto& T: wV)
	{
        // move data from Wall and xWall struct to unionWall
		T.texture_id =PN+5120;
		T.yrepeat =rint((float)T.yrepeat * 1.0/scale);

        T.pos=glm::vec3((float)T.x * scale, (float)T.y * scale, 1.0f);
        T.tag=glm::ivec3(T.lotag, T.hitag, T.extra);

		if (T.overpicnum > 0 )
			T.outside_texture_id = T.overpicnum + 5120;
        else
            T.outside_texture_id =0;

        if (T.nextwall < 0)
            T.nextWall=wV.end();
        else
            T.nextWall=wV.begin() + T.nextwall;

	};

    for (auto i=0; i<(int)wV.size(); i++)
	{
        wV.at(i).nextPoint=wV.begin()+wV.at(i).point2;
        wV.at(wV.at(i).point2).prePoint=wV.begin()+i;
	}


	for (auto it=sV.begin(); it != sV.end(); it++)
	{
        // move data from Sector and xSector struct to unionSector
		it->ceiling.texture_id = it->ceilingpicnum + 5120;
		it->floor.texture_id = it->floorpicnum + 5120;
		it->floor.z=(float)it->floorz * scale;
		it->ceiling.z=(float)it->ceilingz * scale;
        if (it->over)
        {
            it->offFloorZ=(float)it->offFloorZ*scale;
            it->onFloorZ=(float)it->onFloorZ*scale;
            it->offCeilZ=(float)it->offCeilZ*scale;
            it->onCeilZ=(float)it->onCeilZ*scale;
        };

        it->tag=glm::ivec3(0, 0, -1);
        it->firstWall=wV.begin()+it->wallptr;

        it->originNumber=it;
	};

    for (auto& T : spV)
    {
        T.texture_id=PN+5120;
        type_to_pic[T.lotag]=T.texture_id;
		T.tag=glm::ivec3(0, 0, -1);
		T.vel=glm::ivec3(0);
		T.pos=glm::vec4((float)T.x *scale, (float)T.y *scale, (float)T.z *scale, 1.0f);
		T.yrepeat =rint((float)T.yrepeat*scale);
        T.xrepeat =rint((float)T.xrepeat*scale);
        T.inSector=sV.begin()+T.sectnum;
        T.statnum=0;
    };

    for(int i=0; i<(int)sV.size();i++)
    {   //calculate Z coordinate
        //algorithm from map2stl.c by Ken Silverman
        // where gis.z  is floor's Z, gis.w is ceiling's Z
        sV.at(i).floor.delta=sV.at(i).ceiling.delta=glm::vec2(0.0f);
        if (sV.at(i).floorstat&2)
            sV.at(i).floor.delta.y=((float)sV.at(i).floorheinum)*(1.f/4096.f);
         if (sV.at(i).ceilingstat&2)
            sV.at(i).ceiling.delta.y=((float)sV.at(i).ceilingheinum)*(1.f/4096.f);

        int p0=sV.at(i).wallptr;
        int p1=wV.at(p0).point2;
        float fx =((float)(wV.at(p1).y)/512.f)-((float)(wV.at(p0).y)/512.f);
        float fy =((float)(wV.at(p0).x)/512.f)-((float)(wV.at(p1).x)/512.f);
        float f=fx*fx+fy*fy;
        if (f>0.f) f=1.f/(float)sqrt(f);
        fx *=f; fy *=f;

        sV.at(i).floor.delta.x=fx*sV.at(i).floor.delta.y;
        sV.at(i).floor.delta.y *=fy;
        sV.at(i).ceiling.delta.x=fx*sV.at(i).ceiling.delta.y;
        sV.at(i).ceiling.delta.y *=fy;
        for(int j=0, k=p0; j<sV.at(i).wallnum; j++, k++)
        {
                wV.at(k).gis.x=(float)(wV.at(k).x)/512.f;
                wV.at(k).gis.y=(float)(wV.at(k).y)/512.f;

                wV.at(k).gis.z=sV.at(i).get_floorZ(wV.at(k).gis.x, wV.at(k).gis.y);
                wV.at(k).gis.w=sV.at(i).get_ceilingZ(wV.at(k).gis.x, wV.at(k).gis.y);

        };
    };

    for (auto& T : sV)
    {
        LOOP new_loop;
        new_loop.marker=T.firstWall;
        for(int walls=0; walls < T.wallnum;)
        {
            T.loops.push_back(new_loop);
            for_loop(new_loop.marker, [&walls] (unionWall _s) { walls++;});
            new_loop.marker= T.firstWall+walls;
        };

        T.generic=T.loops.begin();

        if ((int)T.loops.size() < 2 ) continue;
        for (auto it=T.loops.begin(); it!=T.loops.end(); it++)
        {
            glm::vec3 n=glm::vec3(0.f);
            for_loop(it->marker, [&n] (unionWall _s) { n+=_s.get_normal();});
            if (n.z > 0) { T.generic=it; break; };
        };
    };

    for (auto& T: sV)
    {
        std::vector<unionWall>::iterator first, it;
        for(auto& K : T.loops)
        {
            if (K.marker == T.generic->marker)
                K.set_property("generic");

            std::set<int> ns, dots, normals;

            for_loop(K.marker, [&ns, &dots, &normals](unionWall _s) {
                glm::vec3 a,b;
                a=_s.pos - _s.nextPoint->pos;
                b=_s.pos - _s.prePoint->pos;
                dots.insert(glm::dot(a,b));

                a=_s.get_normal();
                normals.insert(a.z);

                ns.insert(_s.nextsector);
            });

            if ((int)ns.size() == 1 && ns.count(-1))
                K.set_property("loop");
            if ((int)ns.size() == 1 && !ns.count(-1) && K.property("generic"))
                K.set_property("inner");
            if ((int)dots.size() == 1 && T.wallnum == 4  && dots.count(0))
                K.set_property("rectangle");

            bool positive=false, negative=false;
            for(auto J : normals)
            {
                if (J < 0) negative=true;
                else if (J>0) positive=true;
            };

            if ((positive && !negative) || (!positive && negative))
                K.set_property("proper");
        };
    };


    return 0;
};

int Map::finish()
{
    for (auto& T: wV)
	{
        // move data from  unionWall  struct to Wall and xWall
        PN=T.texture_id;
        T.point2=T.nextPoint - wV.begin();
        T.x=rint(T.pos.x); T.y=rint(T.pos.y);
        T.lotag=T.tag.r; T.hitag=T.tag.g;  T.extra=T.tag.b;
        T.overpicnum=T.outside_texture_id;
        if (T.nextWall == wV.end())
            T.nextwall=-1;
        else
            T.nextwall=T.nextWall-wV.begin();
    };

	for (auto& T: sV)
	{
        // move data from unionSector struct to Sector and xSector
        T.ceilingpicnum=T.ceiling.texture_id;
        T.floorpicnum=T.floor.texture_id;
        T.floorz=rint(T.floor.z);
        T.ceilingz=rint(T.ceiling.z);
        T.lotag=T.tag.r; T.hitag=T.tag.g;  T.extra=T.tag.b;
        T.wallptr=T.firstWall-wV.begin();
        T.wallnum=T.wall_count();
    };

    for (auto& T : spV)
    {
        PN=T.texture_id;
        T.lotag=T.tag.r; T.hitag=T.tag.g; T.extra=T.tag.b;
        T.x=rint(T.pos.x); T.y=rint(T.pos.y); T.z=rint(T.pos.z);
        T.xvel=T.vel.x; T.yvel=T.vel.y; T.zvel=T.vel.z;
        T.sectnum=T.inSector-sV.begin();
    };
    return 0;
};

int Map::processing(const float scope=1.f) {
    scale=scope;
    prepare();

    std::cout << "Start processing..." << std::endl;

    for (auto& T : spV)
    {   // parsing picnumber
        if (T.is("levelLocker"))
            T.makeLevelLocker();
        else if (T.is("FootTraces"))
            T.makeFootTraces();
        // parsing type/lotag
        if (T.isType("Ambient SFX"))
            T.makeAmbient(sTable.trans(T.data3), T.data2*16);
        else if(T.isType("WaterDrip Gen"))
            T.makeWaterDripGen();
        else if(T.isType("BloodDrip Gen"))
            T.makeBloodDripGen();
        else if(T.isType("Toggle switch") || T.isType("1-Way switch"))
            T.tag.r=sTable.trans(T.data1);
        else if(items.count(T.lotag) && T.launchS) // mulipalyer
            T.pal=1;
        else if(items.count(T.lotag) && T.launchT && T.launchB)
            T.pal=2;
        else if(T.isType("Sector SFX") || T.isType("SFX Gen") || T.isType("Player SFX"))
            makeSectorSFX(T);
        //else if (T.isType("Explode Object")|| (T.isType("Gib Object") && !(T.cstat & 0x01) && T.texture_id != 5778 ))
        //    makeExplodeAndGib(T);

//        T.makeCstat();

        if (T.dropId > 0 || T.isType("Dude Spawn"))
            makeRespawn(T);
    };
    Cstat();
    makeExplodeAndGib();
    makeEnemies();

    // secret room
    for (auto& T: sV)
    {
        if ( T.over && T.txID == 2)
            LT=32767;
    };

    //spV.begin()->pos=spV.begin()->pos +glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

////////////////////////////////////////////////////////////////////////////
    for (auto& T: spV) // get start position
        if ( T.isType("Player Start") && T.texture_id == 7642 && !T.data1)
        {
			dh.X=T.pos.x; dh.Y=T.pos.y; dh.Z=T.pos.z;
			dh.angle=T.ang;
			dh.sector=(short)(T.inSector-sV.begin());
			std::cout << "Start position: " << "X: " << dh.X << " Y: " << dh.Y << " Z: " << dh.Z <<
                    " Sector: " << dh.sector<< std::endl;
        } else if(T.isType("Hidden Exploder"))
            T.makeHiddenExploder();

    makeExplosiveSector();
    makeElevatorSector();
    makeDoomDoors();
    makeSlideSector();
    makeEnterSensor();
    makeRotateSector();
    makeSlideDoors();

    makeController();
    makeTROR();

    check();

    std::set<int> the_list={};
    for (auto T: sV) if (T.over && !T.done && T.lotag) the_list.insert(T.refer);

    std::cout  <<std::endl << "undone sectors: ";
    for (auto T: the_list) std::cout << T << " "; std::cout << std::endl;

    // remove sprites
	std::set<int> robj{9117, 5873, 7202, 7451, 7452, 7642, 7643, 7644, 7645, 7646, 7647, 7648, 7649};
    auto it=spV.begin(); int last=spV.size();

    while (it != spV.end())
        if (robj.count(it->texture_id))
            spV.erase(it);
        else
			++it;

    std::cout << std::endl << "was removed: " << (last-spV.size()) << " sprites" << std::endl;

    finish();

	return 0;
};

int Map::check()
{
    bool flag=false; SC<<SE;
    for (auto i:sV)
    {
        if (i.wallnum != i.wall_count())
        {
            SC << "Have collision in wall counter: " << i.wallnum << " " << i.wall_count() << SE;
            flag=true;
        };
        if (i.wallptr != (i.firstWall - wV.begin()))
        {
            SC << "Have collision in wall pointer: " << i.wallptr << " " << (i.firstWall-wV.begin()) << SE;
            flag=true;
        };
    };
    if (!flag)
        SC << "check of the map structure was success." << SE;
    return 0;
};
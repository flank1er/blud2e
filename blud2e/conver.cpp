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
#include "blud2e.h"
#include <glm/glm.hpp>
//#include <cfloat>

class CHANNEL
{
private:
    std::set<int> the_list;
    std::set<int> reserved;
    std::set<int> sound_list;
public:
    void add(int value) {the_list.insert(value);}
    void addReserved(int value) {if (value >0) reserved.insert(value);}
    void addSound(int value) {sound_list.insert(value);}
    void printReserved(std::stringstream& msg){ for(auto T: reserved) msg << T<< " "; msg << std::endl;}
    void printChannals(std::stringstream& msg){ for(auto T: the_list) msg << T<< " "; msg << std::endl;}
    void printUsed(std::stringstream& msg) {for(auto T:the_list) if (reserved.count(T)) msg << T <<" "; msg << std::endl;}
    void printUnUsed(std::stringstream& msg) {for(auto T:reserved) if (!the_list.count(T)) msg << T <<" "; msg << std::endl;}
    int getSize() {return (int)the_list.size();}
    int getSizeReserved() { return (int)reserved.size();}
    bool isReserved(int value) { return (reserved.count(value)) ? true :false;}

    int operator()(int ch = -1) {
        if (reserved.count(ch) && ch >= 100)
        {
            the_list.insert(ch);
            return ch;
        }  else if (ch < 100 ||  the_list.count(ch))
        {
            ch=500;
            while (the_list.count(ch) || sound_list.count(ch))
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
    int get_size()
    {
        return (int)the_list.size();
    };

    void erase(){the_list.erase(the_list.begin(), the_list.end());};
    void print(std::stringstream& refer) { for(auto i: the_list) refer << " " << i;}
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

/// checkPoint in Polygon
/// from book "Polygonal models" author: A.Boreskov, E.Shikin. page 203, ISBN 5-86404-139-4
bool unionSector::isInside(glm::vec3 pt)
{
    int count=0;
    auto it=generic->marker;
    do{
        auto next=it->nextPoint;
        if ((it->pos.y == next->pos.y) || (it->pos.y > pt.y && next->pos.y > pt.y) || (it->pos.y < pt.y && next->pos.y < pt.y))
        {
            it=next;
            continue;
        }
        if (std::max(it->pos.y, next->pos.y) == pt.y)
        {
            count++;
        } else if (std::min(it->pos.y, next->pos.y) == pt.y)
        {
            it=next;
            continue;
        } else
        {
            float t=(pt.y - it->pos.y)/(next->pos.y-it->pos.y);
            if (it->pos.x+t*(next->pos.x-it->pos.x) >= pt.x)
                count++;
        }

        it=next;
    } while (it != generic->marker);

    return (count & 1) ? true : false;
}

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

std::vector<int> blud2e::findAllSprites(std::vector<unionSector>::iterator the_sector)
{
    std::vector<int> the_vector;
    for(int i=0; i < (int)spV.size(); i++)
        if (spV.at(i).inSector == the_sector)
            the_vector.push_back(i);
    return the_vector;
};

std::vector<int> blud2e::find_owner_sprites(int owner)
{
    std::vector<int> ret_list;
    for(auto T: spV)
        if (T.owner == owner)
            ret_list.push_back(T.getNum());
    return ret_list;
}

glm::vec3 unionSector::getCenter()
{
    glm::vec3 ret=glm::vec3(0.f);
    int counter=0;
    for_loop(generic->marker, [&](unionWall _s) {
        ret +=_s.pos;
        counter++;
    });

    return (ret/(float)counter);
};

glm::vec3 unionSector::get_rnd_pos(std::stringstream& msg)
{
    glm::vec3 ret=glm::vec3(0.f);
    auto orig=getCenter();
    float xMax, xMin, yMax, yMin;
    xMax=xMin=orig.x;  yMin=yMax=orig.y;
    for_loop(generic->marker, [&](unionWall _s)
    {
        if(xMax < _s.pos.x) xMax=_s.pos.x;
        if(xMin > _s.pos.x) xMin=_s.pos.x;
        if(yMax < _s.pos.y) yMax=_s.pos.y;
        if(yMin > _s.pos.y) yMin=_s.pos.y;
    });

    if (xMax < xMin || yMax < yMin)
    {
        msg << "ERROR: break in  get_rnd_pos\n";
        return ret;
    }
    float dx=xMax-xMin; float dy=yMax-yMin;
    do {
        ret.x=(rand() % (int)dx)+xMin;
        ret.y=(rand() % (int)dy)+yMin;
    } while(!isInside(ret));
    ret.z=floor.z;
    return ret;
};

glm::vec2 unionSector::getNNS(glm::vec2 pt, std::stringstream& msg)
{

    glm::vec2 ret=glm::vec2(0.f);

    for_loop(generic->marker, [&] (unionWall _s)
    {
        auto p=glm::vec2(0.f);
        p.x=_s.pos.x;
        p.y=_s.pos.y;
        if (glm::distance(pt, p) < glm::distance(ret, p))
            ret=p;

    });
    return ret;
    if (ret == glm::vec2(0.f))
        msg << "ERROR in  getNNS\n";

}

int blud2e::get_wall_maplevel(int num, int x, int y, std::stringstream& msg)
{
    for(int i=sV.at(num).wallptr; i < (sV.at(num).wallptr + sV.at(num).wallnum); i++)
    {
        if (wV.at(i).x == x && wV.at(i).y == y)
            return i;
    }
    return -1;
}

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
            if ( !for_all && it->over && it->txID == TX && !it->done && (RX < 0 ||it->rxID == RX) && (lotag < 0 || it->lotag == lotag))
                ret.push_back(it-t.begin());
            else if (for_all && it->over && it->txID == TX && (RX < 0 ||it->rxID == RX) &&	(lotag < 0 || it->lotag == lotag))
                ret.push_back(it-t.begin());
		};
	};
	return ret;
};

template<typename T> std::vector<int> findDoneTx(T& t, int TX)
{
    std::vector<int> ret;
    if (TX > 0)
    {
        for (auto it=t.begin(); it != t.end(); ++it)
        {
            if ( it->done && it->over && it->txID == TX)
                ret.push_back(it-t.begin());
        };
    };
    return ret;
};

template<typename T> std::vector<int> findDoneRx(T& t, int RX)
{
    std::vector<int> ret;
    if (RX > 0)
    {
        for (auto it=t.begin(); it != t.end(); ++it)
        {
            if ( it->done && it->over && it->rxID == RX)
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

int blud2e::addSprite(int room,  int rxChannel, int txChannel ,std::string name, std::stringstream& msg, glm::vec4 p=glm::vec4(0.f))
{
    if( room >= (int)sV.size())
    {
        msg << "ERROR: room number greatest than count of sectors\n";
        return EXIT_FAILURE;
    }

    if (room < 0 || room >= getSectors())
    {
        msg << "Error: out of range sectors. Method: addSprite, sprite: " << name <<  " sector: " << room << std::endl;
        return EXIT_FAILURE;
    }

    unionSprite nsp=spV.at(0);
    auto it=sV.begin()+room;

    if (it->getNum() != room)
    {
        msg << "Error in room number. Method: addSprite, sprite: " << name <<  " sector: " << room << std::endl;
        return EXIT_FAILURE;
    }

    glm::vec3 rnd_pos=it->get_rnd_pos(msg);
    if (!it->isInside(rnd_pos))
    {
        msg << "Error in position. Method: addSprite, sprite: " << name <<  " sector: " << room << std::endl;
        return EXIT_FAILURE;
    }

    nsp.pos=glm::vec4(rnd_pos, 0.f);

    if (p != glm::vec4(0.f))
        {nsp.pos.x=p.x; nsp.pos.y=p.y;};

    nsp.inSector=sV.begin()+room;
    nsp.over=false;
    nsp.tag=glm::ivec3(0, 0,-1);
    nsp.vel=glm::ivec3(0);
    nsp.cstat=0x01;
    nsp.texture_id=1;
    nsp.shade=nsp.xoffset=nsp.yoffset=nsp.ang=nsp.pal=nsp.statnum=0;
    nsp.owner=-1;
    nsp.xrepeat=nsp.yrepeat=64;
    nsp.setNum(getSprites());
    //nsp.sectnum=room;

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
        nsp.vel.z=1;
        nsp.vel.y=rxChannel;
        spV.push_back(nsp);
    } else if (name == "Wall Crack")
    {
        nsp.texture_id=546;
        nsp.tag.g= rxChannel;
        nsp.cstat=0x202;
        spV.push_back(nsp);
    } else if (name == "MASTERSWITCH")
    {
        nsp.texture_id=8;
        nsp.tag.r= rxChannel;
        //nsp.cstat=0x;
        spV.push_back(nsp);
    } else if (name == "TC_LIGHTING")
    {
        nsp.texture_id=32;
        //nsp.tag.g= rxChannel;
        nsp.cstat=0x0;
        nsp.shade=txChannel;
        nsp.owner=-1;
        nsp.vel.x=rxChannel;
        spV.push_back(nsp);
    };

    return EXIT_SUCCESS;
};

int blud2e::makeExplosiveSector(std::stringstream& refer) {
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

    refer << "Boom Sectors: "; for(auto T: boom_set) refer << T << " "; refer << std::endl;

    for(auto T: boom_set)
    {
        //assert(T == sV.at(T).refer);
        if (!sV.at(T).over || T != sV.at(T).refer)
        {
            refer << "ERROR: corrupt in sector:" << T << std::endl;
            return EXIT_FAILURE;
        }
    	sV.at(T).ceiling.z=sV.at(T).onCeilZ;
        sV.at(T).floor.z=sV.at(T).onFloorZ;
        int ch=channel(sV.at(T).rxID);
        //channel.add(ch);
        addSprite(T, ch, 0, "SE13", refer);

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
                spV.at(spr).cstat &=~(1<<7);
                addSprite(T,ch, 0, "Wall Crack", refer, spV.at(spr).pos);
                lastSprite.cstat=0x8090; // invisible
            };
        sV.at(T).done=true;
        sV.at(T).log=glm::ivec2(13,13); /// boom sector
    };

    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " BOOM Sectors" << std::endl;
    check(refer);
	return ret;
};

int blud2e::makeElevatorSector(std::stringstream& refer)
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

    refer<< std::endl << "Elevator Sectors: "; for(auto T: elevator_set) refer<< T << " "; refer << std::endl;

    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " Elevator Sectors" << std::endl;
	return ret;
};

int blud2e::makeDoomDoors(std::stringstream& refer)
{
	int last=get_done(sV);
    std::set<int> floor_set;

    for(auto T: sV)
    {
       if ((T.over && T.is("MotionZ") && !T.done && (T.onFloorZ != T.offFloorZ)) ||
             (T.over && T.is("MotionZ") && !T.done && (T.onCeilZ != T.offCeilZ)))
            floor_set.insert(T.refer);
    };

   refer<< std::endl << "FloorDoor Sectors: "; for(auto T: floor_set) refer<< T << " "; refer << std::endl;
    //std::cout<< "CeilingDoor Sectors: "; for(auto T: ceiling_set) std::cout<< T << " "; std::cout << std::endl;

    for(auto S: floor_set)
    {
        if (S != sV.at(S).getNum())
        {
            refer << "Error in sector number. Method: makeDoomDoors, sector: " << S << std::endl;
            return EXIT_FAILURE;
        }
        //assert(S == sV.at(S).refer);
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
            refer << "pair sector: " << S <<" <-- ";
            for(auto j: pair_list) refer << j << " ";
            refer << std::endl;
            ps=sV.begin() + *pair_list.begin();
        };

        if (it->onFloorZ != it->offFloorZ)
            is_floor=true;

        int range, speed;
        if (is_floor)
            range=abs(it->onFloorZ-it->offFloorZ);
        else
            range=abs(it->onCeilZ-it->offCeilZ);
        if (ps->busyTime > 0)
            speed=(rint)(range/(float)(ps->busyTime * 2.6f));
        else
            speed=512;

        addSprite(S,ps->log.x,chX, "SWITCH", refer);
        if (it->triggerWPush)
            addSprite(S,ps->log.x,it->key, "TC_LOCKER", refer);
        auto the_list=findAllSprites(it);
        for (auto i: the_list)
            if(spV.at(i).is("SectorSFX"))
            {
                spV.at(i).vel.x=chX;
                spV.at(i).texture_id=5;
            };

       if (is_floor)
            addSprite(S, chX, 0, "SE69", refer);
        else
            addSprite(S, chX, 0, "SE70", refer);

        if (it->busy > 0)
            addSprite(S,chX,ps->busyTime, "MovePoint", refer);
        else
            addSprite(S,chX,speed, "MovePoint",refer);

        if (it->waitTime > 0)
            addSprite(S, chX, ps->waitTime, "SE10",refer);

        it->floor.z=it->onFloorZ;
        it->ceiling.z=it->onCeilZ;
    };
    check(refer);
    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " DoomDoor Sectors" << std::endl;
	return ret;
};

int blud2e::makeEnterSensor(std::stringstream& refer)
{
	int last=get_done(sV);
    std::set<int> sectors_set;

    for(auto T: sV)
        if (T.over && !T.done && !LT && !T.rxID && T.txID >= 100 && T.triggerEnter)
            sectors_set.insert(T.refer);

    refer<< std::endl << "EnterTrigger Sectors: ";
    for(auto T: sectors_set) refer<< T << " "; refer << std::endl;

    for(auto T: sectors_set)
    {
        assert(T == sV.at(T).refer);
        auto targets=findRx(sV, sV.at(T).txID, -1, -1, true);
        int ch=channel(sV.at(T).txID);

        if (targets.size() >0) refer << "found link: " << T << " <--> ";
        for(auto S: targets)
        {
            assert(sV.at(S).refer == S);
            //static int x=sV.at(S).log.x;
            //if (sV.at(S).log.x != x)  // WARNING
            //    refer << std::endl << " was found collision in logical level: " << T << " <--> " << S<< std::endl;
            //if (sV.at(S).done)
            //    ch=sV.at(S).log.x;
            refer << S << " ";

        };
        if (targets.size()>0) refer << std::endl;
        addSprite(T, ch, 0, "PushTrigger",refer);
        if (sV.at(T).triggerOnce)
            lastSprite.owner=1;
        auto explosive=findRx(sV, ch,-1,-1,true);
        bool expl=false;
        for(auto Ex:explosive)
        {
            if (sV.at(Ex).log.x == 13 && sV.at(Ex).log.y == 13) // if Explosive sector;
            {
                lastSprite.tag.r=ch;
                expl=true;
            }
        }
        if (expl)
        {
            auto boom=findRx(spV, ch, -1, -1, true);
            std::set<int> exp_list;
            for(auto K: boom)
            {
                if (spV.at(K).isType("Hidden Exploder"))
                    exp_list.insert(spV.at(K).inSector->getNum());
            }
            for(auto Ex: exp_list)
            {
                addSprite(Ex, ch, 0, "MASTERSWITCH", refer);
            }
        }


        sV.at(T).done=true;
        sV.at(T).log=glm::ivec2(0, ch);
    };
    check(refer);
    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " Trigger Sectors" << std::endl;
	return ret;
};

int blud2e::makeSlideSector(std::stringstream& refer)
{
	int last=get_done(sV);
    std::set<int> sector_list;
    for(auto T: sV)
        if(T.over && !T.done && T.is("Slide"))
            sector_list.insert(T.refer);

    refer<< std::endl << "found Slide Sectors: ";
    for(auto T: sector_list) refer<< T << " "; refer << std::endl;

    for(auto T: sector_list)
    {
        assert(sV.at(T).refer == T);
        std::vector<unionSector>::iterator it=sV.begin()+T;
        if (sV.at(T).loops.size() ==1 )//&& sV.at(T).generic->property("inner") )
        {
            assert(it->marker0 > 0 && it->marker1 >0 );
            assert(spV.at(it->marker0).xvel ==it->marker0 && spV.at(it->marker1).xvel == it->marker1);
            int slaveSector=-1;
            float range=glm::distance(glm::ivec2(spV.at(it->marker0).pos.x, spV.at(it->marker0).pos.y),
                                      glm::ivec2(spV.at(it->marker1).pos.x, spV.at(it->marker1).pos.y));
            int speed=0;
            if (it->busyTime > 0)
                speed=(rint)(range/(float)(it->busyTime*2.6f));

            if (sV.at(T).log.x > 0 && sV.at(T).log.x < getSectors())
            {
                refer << "was found slave Sector: " << sV.at(T).log.x << " for master Sector: " << T << std::endl;
                slaveSector=sV.at(T).log.x;
            }

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

            addSprite(T, ch, angle, "SE64",refer);
            addSprite(T,ch,angle, "GPSPEED", refer);
            lastSprite.tag.x=sqrt(dx*dx+dy*dy); // lotag
            lastSprite.tag.y=speed;
            if (it->triggerWPush)
                addSprite(T,ch, 0, "TC_LOCKER", refer);
            if (it->waitTime > 0)
                addSprite(T, ch, it->waitTime, "SE10",refer);
            it->tag.g=ch;
            if (slaveSector >= 0)
                sV.at(slaveSector).tag.g=ch;
            it->log=glm::ivec2(ch,0);
            it->done=true;
        } //else if (sV.at(T).log.x > 0 && sV.at(T).log.x < getSectors())
        //{
        //    refer << "was found slave Sector: " << sV.at(T).log.x << " for master Sector: " << T << std::endl;
       // }

    };

    check(refer);
    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " Slide Sectors" << std::endl;
	return ret;
};


int blud2e::makeZMotionSrite(std::stringstream& refer)
{
    int last=get_done(sV);
    std::set<int> the_list;
    refer << std::endl << "Z Motion SPRITE sectors was found: ";
    for(auto T:sV)
        if(T.over && !T.done && T.is("Z Motion SPRITE") && T.txID>=100 && T.rxID >=100)
    {
        refer << T.getNum() << " ";
        the_list.insert(T.getNum());
    }; refer<<std::endl;

    for (auto T: the_list)
    {
        bool done=false;
        auto it=sV.begin()+T;
        auto sprites=findAllSprites(it);
        int ch=0;
        for(auto S: sprites)
        {
            auto sp=spV.begin()+S;
            if (sp->over && sp->rxID == 0 && sp->txID >= 100 && sp->txID==it->rxID)
            {
                ch=channel(sp->txID);
                int owner=channel();
                sp->owner=owner;
                sp->statnum=15;
                addSprite(T, ch, 0, "PushTrigger", refer, sp->pos);
                lastSprite.texture_id=6166;
                lastSprite.cstat=0x8090;
                lastSprite.vel.z=0;
                lastSprite.ang=sp->ang;
                lastSprite.owner=owner;
                lastSprite.pos=sp->pos;

                addSprite(T, it->txID, it->txID, "MovePoint", refer);
                lastSprite.owner=owner;
                lastSprite.tag=glm::ivec3(20,1,-1);
                glm::vec4 pt=lastSprite.pos;

                addSprite(T, it->txID, it->txID, "MovePoint", refer, sp->pos);
                lastSprite.owner=owner;
                lastSprite.texture_id=15;   // MOVER
                lastSprite.tag.x=lastSprite.tag.y=0;
                lastSprite.pos=pt;
                lastSprite.pos.z=lastSprite.pos.z-1024;

                done=true;
            }

        }

        if (done)
        {
            addSprite(T, channel(it->rxID), channel(it->txID), "SWITCH", refer);
            for(auto S: sprites)
            {
                auto sp=spV.begin()+S;
                if (sp->is("SectorSFX"))
                {
                    sp->texture_id=19;
                    sp->vel.x=it->txID;
                }
            }
        }
    }

    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " Z Motion SPRITE Sectors" << std::endl;
    return EXIT_SUCCESS;
}

int blud2e::makeStepRotate(std::stringstream& refer)
{
    int last=get_done(sV);
    std::set<int> the_list;
    refer << std::endl << "Step Rotate sectors was found: ";
    for(auto T:sV)
        if(T.over && !T.done && T.is("Step Rotate") && T.rxID >=100)
    {

        the_list.insert(T.getNum());
    };

    for (auto T: the_list)
    {
        auto it=sV.begin()+T;
        if (!it->done)
        {
            auto sat=findRx(sV, it->rxID);
            bool pivot_point=false;
            for (auto S: sat)
            {
                auto iter=sV.begin()+S;
                //bool pivot_point=false;
                if (iter->is("Step Rotate"))
                {
                    refer << iter->getNum() << " ";
                    //iter->tag.g=it->rxID;
                    iter->tag.r=30;
                    iter->done=true;
                    if (iter->marker0 >= 0 && !pivot_point)
                    {
                        spV.at(iter->marker0).texture_id=1;
                        spV.at(iter->marker0).tag.r=1;
                        spV.at(iter->marker0).cstat=0x1;
                        spV.at(iter->marker0).tag.g=it->rxID;
                        spV.at(iter->marker0).owner=0;
                        pivot_point=true;
                    }
                    auto sprites=findAllSprites(iter);
                    if ((int)sprites.size() > 0)
                    {
                        addSprite(S, 0, 0, "SE64", refer);
                        lastSprite.tag.r=0;
                        lastSprite.tag.g=it->rxID;
                        lastSprite.ang=512;
                        addSprite(S, 0, 0, "SE64", refer);
                        lastSprite.texture_id=2;
                        lastSprite.tag.r=it->rxID;
                        lastSprite.tag.g=0;
                        lastSprite.ang=1536;
                    }
                }
            }

            auto sprites=findTx(spV,it->rxID);
            for(auto S:sprites)
            {
                auto iter=spV.begin()+S;
                if (iter->over && (iter->isType("1-Way switch") || iter->isType("Toggle switch")))
                {
                    iter->tag.r=iter->txID;
                    iter->done=true;
                }
            }


        }
    }; refer<<std::endl;
    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " Step Rotate Sectors" << std::endl;
    return EXIT_SUCCESS;
}
int blud2e::makeController(std::stringstream& refer)
{
    std::set<int> switch_list;
    for(auto& T:spV)
        if(T.over && T.isType("1-Way switch") && T.texture_id == 6190 && T.rxID == 0 && T.txID >=100)
    {
        T.vel.y=T.txID;
        T.done=true;
    }

    int last=get_done(spV);
    std::set<int> the_list;
    refer << std::endl << "Controles was found: ";
    for(auto T:spV) if(T.over && !T.done && T.txID>=100 && T.rxID >=100)
    {
        refer << T.refer<< " ";
        the_list.insert(T.refer);
    }; refer<<std::endl;



    refer << "set controller: ";
    for(auto T:the_list)
    {
        auto it=spV.begin()+T;
        auto sensors_list=findDoneTx(sV, it->rxID);
        auto activators_list=findDoneRx(sV, it->txID);
        if (sensors_list.size() > 0 && activators_list.size() >0 )
        {
            it->texture_id=9117;  // mark for remove the sprite
            addSprite((it->inSector->getNum()), it->rxID, it->txID, "SWITCH", refer, it->pos);
            it->done=true;

            for(auto i: sensors_list)
            {
                auto sprites=findAllSprites(sV.begin()+i);

                auto iter=sprites.begin();
                while (iter != sprites.end())
                    if (spV.at(*iter).texture_id != 18)
                        sprites.erase(iter);
                    else
                        ++iter;

                for(auto k: sprites)
                {
                    int chx=channel();
                    int level=0; if (!it->launch1) level +=1;
                    if (!it->launch2) level +=2; if (!it->launch3) level +=4;
                    if (!it->launch4) level +=8; if (!it->launch5) level +=16;
                    addSprite((spV.at(k).inSector-sV.begin()), chx, level , "SE60", refer);
                    spV.at(k).vel.x=chx;
                    refer << std::endl << "add sprite SE60 in: " << (spV.at(k).inSector-sV.begin()) << " launch level: " <<level;
                };
           }
        }
    }

    for(auto T:the_list)
    {
        auto it=spV.begin()+T;
        auto sensors_list=findDoneTx(sV, it->rxID);
        auto activators_list=findDoneRx(spV, it->txID);
        if (sensors_list.size() > 0 && activators_list.size() >0 )
        {
            refer<< SE<< "sector: " << it->inSector->getNum()<< " sensors: ";
            for(auto S: sensors_list)
                refer << S << " type: "<<  sV.at(S).lotag << " ";
            refer << SE;
            refer << "activators: ";
            for(auto A: activators_list)
                refer << A << " type: " << spV.at(A).lotag << " ";
            refer << SE;

            it->texture_id=9117;  // mark for remove the sprite
            addSprite((it->inSector->getNum()), it->rxID, it->txID, "SWITCH", refer, it->pos);
            it->done=true;
        }


    }
    check(refer);
    int ret =get_done(spV)-last;
    refer << std:: endl << "was done: " << ret << " Controllers" << std::endl;
    return ret;
};

int blud2e::makeTROR(std::stringstream& refer)
{
    std::vector<auxTROR> aux;
    int ret=0;
    refer << std::endl << "ROR sectors was found:";
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
            refer << " ("<< new_el.up<< "<-->" << new_el.down << ")";
        };
    }; refer<<std::endl;

    for (auto T: aux)
    {
        COLLECT up,down;
        up(wV, sV, (sV.begin()+T.up));
        down(wV, sV, (sV.begin()+T.down));
        if (up != down)
        {
            if (up < down && !up.isBroken(spV))
            {
                refer << "movement upper sectors:"; up.print(refer); refer<<SE;
                T.diff *=(-1);
            } else if (down <up &&  !down.isBroken(spV))
            {
                refer<<"movement lower sectors:"; down.print(refer); refer<<SE;
                down.moveIt(wV, sV, spV, T.diff);
            }
        //} else if (up == down && T.diff.z == 0)
       } else if (up == down && T.diff.z == 0 &&
                  (sV.at(T.up).generic->property("inner") || sV.at(T.down).generic->property("inner")
                  || mapIs == "e1m1" ))
        {
            auto itS=sV.begin();
            if (sV.at(T.up).generic->property("inner"))
            {
                refer << "movement upper sectors: " << T.up<<SE;
                T.diff *=(-1);
                itS +=T.up;
            } else if (sV.at(T.down).generic->property("inner"))
            {
                refer << "movement down sectors: " << T.down<<SE;
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
        } else { refer << "ERROR: failure TROR processing!" <<SE; continue; };

        auto d=sV.begin()+T.down;
        auto u=sV.begin()+T.up;
        assert(d->loops.size() == 1);
        assert(u->loops.size() == 1);
        if (u->wallnum != d->wallnum){  refer << "ERROR: incorrect amount of wall!" <<SE;  continue;};

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
    check(refer);
    refer  << "was done: " << ret << " TROR Sectors" << std::endl;
	return ret;
};

int blud2e::makeRotateSector(std::stringstream& refer)
{
	int last=get_done(sV);
    std::set<int> sector_list;
    for(auto it=sV.begin(); it != sV.end(); it++)
        if(it->over && !it->done && it->is("Rotate"))
            sector_list.insert(it-sV.begin());

    refer<< std::endl << "found Rotate Sectors: ";
    for(auto T: sector_list) refer<< T << " "; refer << std::endl;

    for(auto T: sector_list)
    {
        auto it=sV.begin()+T;
        if (it->loops.size() ==1 && it->generic->property("inner") )
        {
            assert(it->marker0 > 0);
            auto mrk=spV.begin()+it->marker0;
            assert(mrk->owner == T);
            int ch=channel(it->rxID);
            int chx=channel();
            int key=getKey(it);
            it->log=glm::ivec2(ch,chx);

            auto pair_list=findTx(sV, it->rxID, -1, -1, true);
            for (auto iter=pair_list.begin(); iter != pair_list.end();)
                if (sector_list.count(*iter))
                    iter++;
                else
                    pair_list.erase(iter);

            auto ps=it;
            if (pair_list.size() >0)
            {
                refer<< "pair sector: " << T <<" <-- ";
                for(auto j: pair_list) refer<< j << " ";
                refer << std::endl;
                ps=sV.begin() + *pair_list.begin();
            };

            mrk->tag=glm::ivec3(64,ch, -1);
            mrk->vel=glm::ivec3(chx,0, 1);
            mrk->owner=-1;
            mrk->texture_id=1;
            if (!it->state)
            {
                refer<<"Rotate Sector: "<< T <<std::endl;
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

            glm::vec2 nns;
            nns=glm::vec2(mrk->pos.x, mrk->pos.y);
            nns=mrk->inSector->getNNS(nns, refer);
            mrk->pos.x=nns.x;
            mrk->pos.y=nns.y;

            auto the_list=findAllSprites(it);
            for(auto S: the_list) {
                auto iter=spV.begin()+S;
                if (iter->is("SectorSFX")) {
                    iter->texture_id=5;
                    iter->vel.x=ps->log.y;
            };};
            addSprite(T,ps->log.x, key, "TC_LOCKER", refer);

            addSprite(T,chx,1, "MovePoint", refer);
            lastSprite.ang=0;
            if (ps->busyTime > 0)
                lastSprite.tag.x=(rint)(512.f/(2.6f*(float)ps->busyTime));
            else
                lastSprite.tag.x=32;
            if (mrk->ang >0) lastSprite.vel.z=1;

            if (ch == ps->log.x)
                addSprite(T,ch,chx, "SWITCH", refer);
            else
                addSprite(T,ps->log.y,chx, "SWITCH", refer);
            it->tag.g=ch;
            it->done=true;

        };
    };
    check(refer);
    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " Rotate Sectors" << std::endl;
	return ret;
};

int blud2e::makeSlideDoors(std::stringstream& refer)
{
	int last=get_done(sV);
    std::set<int> sector_list;
    for(auto it=sV.begin(); it != sV.end(); it++)
        if(it->over && !it->done && it->is("Slide Marked"))
            sector_list.insert(it-sV.begin());

    refer<< std::endl << "found Slide Doors: ";
    for(auto T: sector_list) refer<< T << " "; refer << std::endl;
    int j=0;
    for(auto T: sector_list)
    {
        int p0=sV.at(T).marker0;
        int p1=sV.at(T).marker1;

        if (spV.at(p0).owner != T  || spV.at(p1).owner != T || sV.at(T).wallnum < 6 )
               continue;

        int x0=spV.at(p0).x;
        int y0=spV.at(p0).y;
        int x1=spV.at(p1).x;
        int y1=spV.at(p1).y;
        int dx=x0-x1;
        int dy=y0-y1;

        for (int i=sV.at(T).wallptr; i < (sV.at(T).wallptr+sV.at(T).wallnum); i++)
        {
            int p2=wV.at(i).point2;
            int xm=(wV.at(i).x+wV.at(p2).x)/2;
            int ym=(wV.at(i).y+wV.at(p2).y)/2;

            if (((xm == spV.at(p1).x && ym == spV.at(p1).y) || ( xm == (spV.at(p0).x+dx) && ym == (spV.at(p0).y+dy)))
                    && (x0 == x1 || y0 == y1))
            {
                int i2=wV.at(i).point2;
                int i3=wV.at(i2).point2;
                int i0=get_prv_point(T,i);
                int iabs=-1;
                if (i0>=0)
                    iabs=get_prv_point(T,i0);
                if (wV.at(i).x != wV.at(iabs).x && wV.at(i).y != wV.at(iabs).y)
                    continue;

                unionWall wa,wb,wc,wd;
                wa=wV.at(i0);
                wb=wV.at(i);
                wc=wV.at(i2);
                wd=wV.at(i3);

                wd.point2=sV.at(T).wallptr;
                wa.point2=sV.at(T).wallptr+1;
                wb.point2=sV.at(T).wallptr+2;
                wc.point2=sV.at(T).wallptr+3;

                wa.nextsector=wb.nextsector=wc.nextsector=wd.nextsector=sV.size();

                if (xm == spV.at(p1).x && ym == spV.at(p1).y)
                {
                    wa.x += dx;
                    wa.y += dy;
                    wb.x += dx;
                    wb.y += dy;
                    wc.x += dx;
                    wc.y += dy;
                    wd.x += dx;
                    wd.y += dy;

                    sV.at(T).log.x=getSectors();
                    sV.at(T).dxdy=glm::ivec2(dx,dy);
                } else
                {
                    wa.x -= dx;
                    wa.y -= dy;
                    wb.x -= dx;
                    wb.y -= dy;
                    wc.x -= dx;
                    wc.y -= dy;
                    wd.x -= dx;
                    wd.y -= dy;

                    sV.at(T).log.y=getSectors();
                    sV.at(T).ssd=true;

                    if ( x0 == x1 && y0 >= y1)
                        spV.at(p1).ang=1536;
                    else if (x0 == x1 && y0 <= y1)
                        spV.at(p1).ang=512;
                    else if (y0==y1 && x0 >= x1)
                        spV.at(p1).ang=1024;
                    else
                        spV.at(p1).ang=0;
                }

                wV.at(i).x=wV.at(i0).x;
                wV.at(i).y=wV.at(i0).y;
                wV.at(i2).x =wV.at(i3).x;
                wV.at(i2).y =wV.at(i3).y;

                if ( y0 == y1)
                {
                    float scale=1.05;
                    float scale2=1.1;

                    int Mx=(int)((wV.at(i0).x + wV.at(i).x + wV.at(i2).x + wV.at(i3).x)/4);
                    int My=(int)((wV.at(i0).y + wV.at(i).y + wV.at(i2).y + wV.at(i3).y)/4);

                    wV.at(i0).x=(int)((Mx-wV.at(i0).x)*scale2)+Mx;
                    wV.at(i0).y=(int)((wV.at(i0).y-My)*scale2)+My;
                    wV.at(i).x=(int)((Mx-wV.at(i).x)*scale2)+Mx;
                    wV.at(i).y=(int)((wV.at(i).y-My)*scale2)+My;
                    wV.at(i2).x=(int)((Mx-wV.at(i2).x)*scale2)+Mx;
                    wV.at(i2).y=(int)((wV.at(i2).y-My)*scale2)+My;
                    wV.at(i3).x=(int)((Mx-wV.at(i3).x)*scale2)+Mx;
                    wV.at(i3).y=(int)((wV.at(i3).y-My)*scale2)+My;

                    int i00=get_prv_point(T,i0);
                    int i4=wV.at(i3).point2;
                    Mx=(int)((wV.at(i00).x + wV.at(i4).x)/2);
                    My=(int)((wV.at(i00).y + wV.at(i4).y)/2);
                    wV.at(i4).x=(int)((Mx-wV.at(i4).x)*scale)+Mx;
                    wV.at(i4).y=(int)((wV.at(i4).y-My)*scale)+My;
                    wV.at(i00).x=(int)((Mx-wV.at(i00).x)*scale)+Mx;
                    wV.at(i00).y=(int)((wV.at(i00).y-My)*scale)+My;
                } else if ( x0 == x1)
                {
                    float scale=1.05;
                    float scale2=1.1;

                    int Mx=(int)((wV.at(i0).x + wV.at(i).x + wV.at(i2).x + wV.at(i3).x)/4);
                    int My=(int)((wV.at(i0).y + wV.at(i).y + wV.at(i2).y + wV.at(i3).y)/4);

                    wV.at(i0).x=(int)((wV.at(i0).x-Mx)*scale2)+Mx;
                    wV.at(i0).y=(int)((wV.at(i0).y-My)*scale2)+My;
                    wV.at(i).x=(int)((wV.at(i).x-Mx)*scale2)+Mx;
                    wV.at(i).y=(int)((wV.at(i).y-My)*scale2)+My;
                    wV.at(i2).x=(int)((wV.at(i2).x-Mx)*scale2)+Mx;
                    wV.at(i2).y=(int)((wV.at(i2).y-My)*scale2)+My;
                    wV.at(i3).x=(int)((wV.at(i3).x-Mx)*scale2)+Mx;
                    wV.at(i3).y=(int)((wV.at(i3).y-My)*scale2)+My;

                    int i00=get_prv_point(T,i0);
                    int i4=wV.at(i3).point2;
                    Mx=(int)((wV.at(i00).x + wV.at(i4).x)/2);
                    My=(int)((wV.at(i00).y + wV.at(i4).y)/2);
                    wV.at(i4).x=(int)((wV.at(i4).x-Mx)*scale)+Mx;
                    wV.at(i4).y=(int)((wV.at(i4).y-My)*scale)+My;
                    wV.at(i00).x=(int)((wV.at(i00).x-Mx)*scale)+Mx;
                    wV.at(i00).y=(int)((wV.at(i00).y-My)*scale)+My;
                }

                auto it=wV.begin() + sV.at(T).wallptr;
                wd.over=wa.over=wb.over=wc.over=false;
                wV.insert(it,wd);
                wV.insert(it,wc);
                wV.insert(it,wb);
                wV.insert(it,wa);

                sV.at(T).wallnum +=4;

                for (int k=T+1; k < (int)sV.size(); k++)
                    sV.at(k).wallptr+=4;

                for (int k=sV.at(T).wallptr+4; k < (int)wV.size(); k++)
                {
                    if (wV.at(k).point2 >= (sV.at(T).wallptr))
                        wV.at(k).point2+=4;
                };

                for (int k=0; k < (int)wV.size(); k++)
                {
                    if (wV.at(k).nextwall >= (sV.at(T).wallptr))
                        wV.at(k).nextwall+=4;
                    if (wV.at(k).extra >= 0 && wV.at(k).refer >= sV.at(T).wallptr)
                        wV.at(k).refer +=4;
                };

                int nptr=(int)wV.size();


                unionSector ns=sV.at(T);
                ns.wallptr=wV.size();
                ns.wallnum=4;
                int h=sV.at(T).ceilingz;
                ns.floorz=h;
                ns.over=false;
                sV.push_back(ns);

                wa.nextsector=wb.nextsector=wc.nextsector=wd.nextsector=T;
                wa.extra=wb.extra=wc.extra=wd.extra=-1;
                wa.point2=nptr+1; // w3
                wd.point2=nptr+2; // w2
                wc.point2=nptr+3; // w1
                wb.point2=nptr;   // w0

                wa.nextwall=sV.at(T).wallptr+3;
                wd.nextwall=sV.at(T).wallptr+2;
                wc.nextwall=sV.at(T).wallptr+1;
                wb.nextwall=sV.at(T).wallptr;

                wV.push_back(wa);
                wV.push_back(wd);
                wV.push_back(wc);
                wV.push_back(wb);

                wV.at(wV.at(nptr).nextwall).nextwall=nptr;
                wV.at(wV.at(nptr+1).nextwall).nextwall=nptr+1;
                wV.at(wV.at(nptr+2).nextwall).nextwall=nptr+2;
                wV.at(wV.at(nptr+3).nextwall).nextwall=nptr+3;
            }

            if ( x0 == x1 && y0 >= y1)
                spV.at(p0).ang=512;
            else if (x0 == x1 && y0 <= y1)
                spV.at(p0).ang=1536;
            else if (y0==y1 && x0 >= x1)
                spV.at(p0).ang=0;
            else
                spV.at(p0).ang=1024;

        }

        for (int i=sV.at(T).wallptr; i < (sV.at(T).wallptr+sV.at(T).wallnum); i++)
        {
            if (wV.at(i).x == wV.at(wV.at(i).point2).x && wV.at(i).y == wV.at(wV.at(i).point2).y)
            {
               remove_wall(i, refer, true);
               j++;

            };
        }
        auto itT=sV.begin()+T;
        auto sprites=findTx(spV, sV.at(T).rxID, -1, 0, true);
        auto iter=sprites.begin();
        while (iter != sprites.end())
            if (spV.at(*iter).sectnum != T )
                sprites.erase(iter);
            else
                ++iter;

        if ((int)sprites.size() == 2)
        {
            auto  mrk0=spV.begin()+sprites.at(0);
            auto  mrk1=spV.begin()+sprites.at(1);
            if (mrk1 == (mrk0+1))
            {
                mrk0->x -=dx;
                mrk0->y -=dy;
                mrk1->x +=dx;
                mrk1->y +=dy;
                sV.at(T).dxdy=glm::ivec2(dx,dy);
                sV.at(T).res=glm::ivec2(sprites.at(0),sprites.at(1));
            }

        }


    };
    refer << "was removed " << j << " walls\n";
    refer << std::endl;
    //check(refer);

    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " Slide Doors" << std::endl;
	return ret;
};

int blud2e::makeSDSS(std::stringstream& refer) // Slide Doors Sprite System(SDSS)
{

    int last=get_done(sV);
    std::set<int> sector_list;
    for(auto it=sV.begin(); it != sV.end(); it++)
        if(it->over && !it->done && it->is("Slide Marked"))
            sector_list.insert(it-sV.begin());

    refer<< std::endl << "found Slide Doors: ";
    for(auto T: sector_list) refer<< T << " "; refer << std::endl;
    for(auto T: sector_list)
    {
        if (sV.at(T).log.x > 0 && !sV.at(T).startrack)
        {
            refer << T << " ";
            int ch=channel(sV.at(T).rxID);
            int chx=channel();
            int key=sV.at(T).key;

            auto sprites=findAllSprites((sV.begin()+T));
            int p0=sV.at(T).marker0;
            int p1=sV.at(T).marker1;
            int d0=sV.at(T).log.x;
            int d1=sV.at(T).log.y;
            int dx=sV.at(T).dxdy.x;
            int dy=sV.at(T).dxdy.y;

            auto it=sV.begin()+T;
            float range=glm::distance(glm::ivec2(spV.at(it->marker0).pos.x, spV.at(it->marker0).pos.y),
                                      glm::ivec2(spV.at(it->marker1).pos.x, spV.at(it->marker1).pos.y));
            int speed=0;
            if (it->busyTime > 0)
                speed=(rint)(range/(float)(it->busyTime*2.6f));

            if (key == 0)
            {
                for_loop(sV.at(d0).firstWall, [&] (unionWall _w)
                {
                   key =(_w.key > key) ? _w.key : key;
                });
            }

            glm::vec3 rnd_pos=sV.at(d0).get_rnd_pos(refer);
            for(auto S: sprites)
            {
                spV.at(S).sectnum=d0;
                spV.at(S).owner=0;
                spV.at(S).inSector=(sV.begin()+d0);

                if (spV.at(S).isType("Sector SFX"))
                {
                    spV.at(S).texture_id=5;
                    spV.at(S).vel.x=chx;
                    spV.at(S).pos.x=rnd_pos.x;
                    spV.at(S).pos.y=rnd_pos.y;
                }
            }

            addSprite(d0,ch, chx, "SWITCH", refer);
            spV.at(p0).texture_id=1;
            spV.at(p0).vel.x=spV.at(p0).tag.g=chx;
            spV.at(p0).tag.r=64;

            spV.at(p1).texture_id=17;
            spV.at(p1).tag.r=key;
            spV.at(p1).tag.g=30;
            spV.at(p1).vel.z=1;

            spV.at(p1).vel.y=sV.at(d0).tag.g=chx;
            addSprite(d0, chx, 0,"GPSPEED", refer);
            lastSprite.tag.r=sqrt(dx*dx+dy*dy)*scale-32;
            lastSprite.tag.g=speed;

            if (d1 > 0 && d1 < getSectors())
            {
                int chX=channel();
                sV.at(d1).tag.g=chX;
                addSprite(d1, chx, ch, "SE64", refer,spV.at(p0).pos);
                lastSprite.tag.g=chX;
                lastSprite.ang=spV.at(p1).ang;
                addSprite(d1, chx, ch, "TC_LOCKER", refer,spV.at(p0).pos);
                lastSprite.pos.x+=dx*scale;
                lastSprite.pos.y+=dy*scale;
                lastSprite.ang=spV.at(p1).ang;
                lastSprite.tag.r=key;
                addSprite(d1, chX, 0,"GPSPEED", refer);
                lastSprite.tag.r=sqrt(dx*dx+dy*dy)*scale-32;
                lastSprite.tag.g=speed;

            }
            spV.at(p1).ang=spV.at(p0).ang;            
            //sV.at(T).over=false;
        } else if (sV.at(T).res.x > 0 && sV.at(T).res.y > 0 && !sV.at(T).startrack) // for two doors
        {
            refer << T << " ";
            int ch=channel(sV.at(T).rxID);
            int chx=channel();
            int chx2=channel();
            int owner=channel();
            int key=sV.at(T).key;

            auto sprites=findAllSprites((sV.begin()+T));
            int p0=sV.at(T).marker0;
            int p1=sV.at(T).marker1;
            int m0=sV.at(T).res.x;
            int m1=sV.at(T).res.y;
            int dx=sV.at(T).dxdy.x;
            int dy=sV.at(T).dxdy.y;

            addSprite(T, chx, chx, "MovePoint", refer,spV.at(m0).pos);
            lastSprite.owner=owner;
            lastSprite.texture_id=15;
            lastSprite.tag=glm::ivec3(0,0,-1);
            lastSprite.ang=0;
            spV.at(m0).owner=owner;
            addSprite(T, chx, chx, "MovePoint", refer,spV.at(p0).pos);
            lastSprite.pos.x +=dx;
            lastSprite.pos.y +=dy;
            lastSprite.ang=0;
            lastSprite.tag=glm::ivec3(50, 1, -1);

            addSprite(T, 0, ch, "SWITCH", refer,spV.at(m0).pos);
            lastSprite.owner=owner;
            lastSprite.cstat=0x8090;
            lastSprite.vel.z=0;
            lastSprite.pos.z=spV.at(m0).pos.z;
            lastSprite.yrepeat=spV.at(m0).yrepeat*(spV.at(m0).res.y/lastSprite.res.y);
            lastSprite.xrepeat=spV.at(m0).xrepeat*(spV.at(m0).res.x/lastSprite.res.x);
            //lastSprite.tag=glm::ivec3(0,0,-1);
            lastSprite.ang=(1024+spV.at(m0).ang)%2048;

            addSprite(T, ch, chx, "SWITCH", refer);
            addSprite(T, chx, chx2, "SWITCH", refer);


            owner=channel();
            //int chx2=channel();
            addSprite(T, chx2, chx2, "MovePoint", refer,spV.at(m1).pos);
            lastSprite.owner=owner;
            lastSprite.texture_id=15;
            lastSprite.tag=glm::ivec3(0,0,-1);
            lastSprite.ang=0;
            spV.at(m1).owner=owner;
            addSprite(T, chx2, chx2, "MovePoint", refer,spV.at(p0).pos);
            lastSprite.pos.x -=dx;
            lastSprite.pos.y -=dy;
            lastSprite.ang=0;
            lastSprite.tag=glm::ivec3(50, 1, -1);

            addSprite(T, 0, ch, "SWITCH", refer,spV.at(m1).pos);
            lastSprite.owner=owner;
            lastSprite.cstat=0x8090;
            lastSprite.vel.z=0;
            lastSprite.pos.z=spV.at(m0).pos.z;
            lastSprite.yrepeat=spV.at(m0).yrepeat*(spV.at(m0).res.y/lastSprite.res.y);
            lastSprite.xrepeat=spV.at(m0).xrepeat*(spV.at(m0).res.x/lastSprite.res.x);
            lastSprite.ang=(1024+spV.at(m1).ang)%2048;
        }

        if (sV.at(T).startrack)
        {
            int K=sV.at(T).st_num.x;
            if (K <0 || K >= getSectors())
                continue;
            int p0=sV.at(T).marker0;
            int p1=sV.at(T).marker1;
            int speed=0;
            float range=glm::distance(glm::ivec2(spV.at(p0).pos.x, spV.at(p0).pos.y),
                                      glm::ivec2(spV.at(p1).pos.x, spV.at(p1).pos.y));
            if (sV.at(T).busyTime > 0)
                speed=(rint)(range/(float)(sV.at(T).busyTime*2.6f));

            refer << T << " ";
            int ch=channel(sV.at(T).rxID);
            int chx=channel();
            //int chx2=channel();
            int key=sV.at(T).key;

            auto sprites=findAllSprites((sV.begin()+T));
            for (auto S: sprites)
            {
                if (spV.at(S).is("SectorSFX"))
                {
                    spV.at(S).texture_id=5;
                    spV.at(S).vel.x=chx;
                }
            }
            //int p0=sV.at(T).marker0;
            //int p1=sV.at(T).marker1;
            int dx=sV.at(T).dxdy.x;
            int dy=sV.at(T).dxdy.y;

            spV.at(p0).texture_id=1;
            spV.at(p1).texture_id=1;
            spV.at(p0).ang=(spV.at(p1).ang+1024)%2048;
            spV.at(p1).pos=spV.at(p0).pos;

            spV.at(p0).vel.x=chx;
            addSprite(K, ch, ch, "TC_LOCKER", refer);
            //lastSprite.pos.x-=dx*scale;
            //lastSprite.pos.y-=dy*scale;
            //lastSprite.ang=spV.at(p1).ang;
            lastSprite.tag.r=key;
            lastSprite.tag.g=chx;

            addSprite(K, chx, 0,"GPSPEED", refer);
            lastSprite.tag.r=sqrt(dx*dx+dy*dy)*scale-32;
            lastSprite.tag.g=speed;
            lastSprite.ang=spV.at(p0).ang;

            addSprite(T, ch, chx, "SWITCH", refer);

            spV.at(p0).tag.r=64;
            spV.at(p0).tag.g=chx;
            if (sV.at(T).ssd)
            {
                int Z=sV.at(T).st_num.y;
                if (Z <0 || Z >= getSectors())
                    continue;
                int chx2=channel();
                spV.at(p1).tag.r=64;
                spV.at(p1).tag.g=chx2;
                spV.at(p1).vel.x=chx2;
                addSprite(Z, ch, ch, "TC_LOCKER", refer);
                lastSprite.tag.r=key;
                lastSprite.tag.g=chx2;

                addSprite(Z, chx2, 0,"GPSPEED", refer);
                lastSprite.tag.r=sqrt(dx*dx+dy*dy)*scale-32;
                lastSprite.tag.g=speed;
                lastSprite.ang=spV.at(p1).ang;

                addSprite(Z, chx, chx2, "SWITCH", refer);







            }





        }

    }
    refer << std::endl;


    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " Slide Doors Sprite System" << std::endl;
    return ret;
}


int blud2e::prepareSlideSector(std::stringstream& refer)
{
    int doneSlide=0;
    std::set<int> sector_list;
    for(auto T: sV)
        if(T.over && T.is("Slide"))
            sector_list.insert(T.refer);

    refer<< std::endl << "found Slide Sectors: ";
    for(auto T: sector_list) refer<< T << " "; refer << std::endl;
    int number=0;
    for (auto& W: wV) W.setNum(number++);
    for (int i=0; i < (int)sV.size(); i++)
        for (int j=sV.at(i).wallptr; j<(sV.at(i).wallptr+sV.at(i).wallnum); j++)
            wV.at(j).sector=i;


    for(auto T: sector_list)
    {
        int count=0;
        std::set<int> sectors;
        auto first=sV.at(T).wallptr;
        auto it=first;
        do{
            count++;
            sectors.insert(wV.at(it).nextsector);
            it=wV.at(it).point2;
        } while (it != first);

        if (count != sV.at(T).wallnum)
        {
             refer << "count: " << count << " wallnum: " << sV.at(T).wallnum <<std::endl;
            continue;
        }
        if ( sectors.count(-1) == 0 )
            continue;

        refer << "slide sector: " << T <<std::endl;
        sectors.erase(-1);
        for (auto& K: sectors)
        {
            auto sec=sV.begin()+K;

            int j=0;

            for (int i=sec->wallptr; i<(sec->wallptr+sec->wallnum); i++)
                if (wV.at(i).point2 < i)
                    j++;

            if (j == 1)
                sectors.erase(K);
        }

        if ((int)sectors.size()  == 1 )
        {
            auto sec=sV.begin()+ *sectors.begin();

            int marker0=sec->wallptr;
            int marker1=marker0;
            int mrk=marker0;
            for (int i=sec->wallptr; i<(sec->wallptr+sec->wallnum); i++)
            {
                static bool flag=false;

                if (wV.at(i).nextsector == T && mrk != sec->wallptr)
                {
                    marker0=mrk;
                    flag=true;
                }

                if (wV.at(i).point2 < i)
                {
                    if (flag)
                        marker1=i;

                    flag=false;
                    mrk=i+1;

                }
            }
            refer<< " found loop sector: " <<*sectors.begin() << std::endl;
            refer<< "marker0: " << marker0 << " marker1: " << marker1 <<  std::endl;

            if (marker0 == sec->wallptr || marker1 == sec->wallptr)
                continue;

            if (wV.at(marker0).nextsector >= 0)
                continue;

            bool done=false;
            int idx=marker0;
            int countWall=0;
            do
            {
                static bool generic=true;
                if (wV.at(idx).nextsector == -1 && generic)
                    countWall++;
                else if (wV.at(idx).nextsector == -1 && !generic)
                    countWall++;
                else
                {
                    idx=wV.at(wV.at(idx).nextwall).point2;
                    generic=false;
                    if (wV.at(idx).nextsector < 0)
                        countWall++;
                    else
                        done=true;
                }

                idx=wV.at(idx).point2;

                if (wV.at(idx).x == wV.at(marker0).x && wV.at(idx).y == wV.at(marker0).y)
                    done=true;

                if (countWall > (int)wV.size()) // protect
                {
                    refer << "ERROR in prepare Slide Sector! Endlessly loop!";
                    return EXIT_FAILURE;
                }
            } while(!done);
            refer << "count of wall: " << countWall << std::endl;

            ////////////////////////////////
            done=false;
            idx=marker0;
            int WC=wV.size();
            std::vector<unionWall> myWalls;
            do
            {
                static int countWall=0;
                static bool generic=true;
                if (wV.at(idx).nextsector == -1 && generic)
                {
                    unionWall nw=wV.at(idx);
                    nw.nextsector=nw.sector;
                    nw.nextwall=nw.getNum();
                    myWalls.push_back(nw);

                    wV.at(idx).nextwall=WC+countWall;
                    wV.at(idx).nextsector=getSectors();
                    countWall--;
                } else if (wV.at(idx).nextsector == -1 && !generic)
                {
                    unionWall nw=wV.at(idx);
                    nw.nextsector=nw.sector;
                    nw.nextwall=nw.getNum();
                    myWalls.push_back(nw);

                    wV.at(idx).nextsector=getSectors();
                    wV.at(idx).nextwall=WC+countWall;
                    countWall--;
                } else
                {
                    idx=wV.at(wV.at(idx).nextwall).point2;
                    generic=false;
                    if (wV.at(idx).nextsector < 0)
                    {
                        unionWall nw=wV.at(idx);
                        nw.nextsector=nw.sector;
                        nw.nextwall=nw.getNum();
                        myWalls.push_back(nw);

                        wV.at(idx).nextsector=getSectors();
                        countWall--;

                    } else
                        done=true;
                }

                idx=wV.at(idx).point2;

                if (wV.at(idx).x == wV.at(marker0).x && wV.at(idx).y == wV.at(marker0).y)
                    done=true;

            } while(!done);

            refer << "size of the new sector: " << myWalls.size() << std::endl;
            unionSector ns=sV.at(T);
            ns.wallptr=wV.size();
            ns.wallnum=myWalls.size();
            int h=sV.at(*sectors.begin()).ceilingz;
            ns.floorz=h;
            ns.over=false;
            sV.push_back(ns);

            int sizeWalls=wV.size();
            std::reverse(myWalls.begin(), myWalls.end());

            for (auto it=myWalls.begin(); it < myWalls.end(); it++)
            {
                wV.push_back(*it);
            }
            int x=wV.at(wV.size()-1).x;
            int y=wV.at(wV.size()-1).y;
            for (int i=sizeWalls; i < (int)wV.size(); i++)
            {
                if (i != ((int)wV.size()-1))
                    wV.at(i).point2=i+1;
                else
                    wV.at(i).point2=sizeWalls;
            }

            for (int i=((int)wV.size()-1); i > sizeWalls; i--)
            {
                    wV.at(i).x=wV.at(i-1).x;
                    wV.at(i).y=wV.at(i-1).y;
            }

            wV.at(sizeWalls).x=x;
            wV.at(sizeWalls).y=y;

            for (int i=sizeWalls; i < (int)wV.size(); i++)
            {
                wV.at(wV.at(i).nextwall).nextwall=i;
                wV.at(wV.at(i).nextwall).nextsector=sV.size()-1;

            }
            doneSlide+=sectors.size();
            //for(auto S: sectors) sV.at(S).log.x=getSectors()-1;
            sV.at(T).log.x=getSectors()-1;
        }
    };


    refer << "was preapare: " << doneSlide << " Slide Doors Sprite System\n" << std::endl;
    return EXIT_SUCCESS;
}

int blud2e::makeStarTrackDoors(std::stringstream& refer)
{
    int last=get_done(sV);
    std::set<int> the_list;
    refer << std::endl << "Slide Marked sectors was found: ";
    for(auto T:sV)
        if(T.over && !T.done && T.is("Slide Marked") && T.log.x == 0)
    {
        refer << T.refer << " ";
        the_list.insert(T.refer);
    }; refer<<std::endl;

    for (auto T: the_list)
    {
        auto itT=sV.begin()+T;
        int p0=sV.at(T).marker0;
        int p1=sV.at(T).marker1;

        if (spV.at(p0).owner != T  || spV.at(p1).owner != T || sV.at(T).wallnum < 6 )
               continue;

        int x0=spV.at(p0).x;
        int y0=spV.at(p0).y;
        int x1=spV.at(p1).x;
        int y1=spV.at(p1).y;
        int dx=x0-x1;
        int dy=y0-y1;

        auto sprites=findRx(spV, itT->txID);

        if ((int)sprites.size() > 0)
        {
            refer << SE<< "was found SSD" << T << SE;
        }

        for (int i=sV.at(T).wallptr; i < (sV.at(T).wallptr+sV.at(T).wallnum); i++)
        {
            auto it=wV.begin()+i;
            int p2=wV.at(i).point2;
            int xm=(wV.at(i).x+wV.at(p2).x)/2;
            int ym=(wV.at(i).y+wV.at(p2).y)/2;

            if ((xm == spV.at(p1).x && ym == spV.at(p1).y))
            {
                refer << " found Star Track Door!" << " sector: "  << T << std::endl;
                //int i2=wV.at(i).point2;
                glm::ivec2 i1=glm::ivec2(wV.at(i).x, wV.at(i).y);     ///    ^i3-->
                glm::ivec2 i2=glm::ivec2(wV.at(i+1).x, wV.at(i+1).y);////    |     i4
                glm::ivec2 D=glm::ivec2(dx,dy);                       ///    |     |
                glm::ivec2 i3=i2+D;                                   ///    i2    |
                glm::ivec2 i4=i1+D;                                   ///    <--i1 *


                unionWall wa,wb,wc,wd;
                wa=wb=wc=wd=wV.at(i);
                wa.x=i1.x; wa.y=i1.y;
                wb.x=i4.x; wb.y=i4.y;
                wc.x=i3.x; wc.y=i3.y;
                wd.x=i2.x; wd.y=i2.y;

                wd.point2=sV.at(T).wallptr;
                wa.point2=sV.at(T).wallptr+1;
                wb.point2=sV.at(T).wallptr+2;
                wc.point2=sV.at(T).wallptr+3;

                wa.nextsector=wb.nextsector=wc.nextsector=wd.nextsector=sV.size();
                wa.xrepeat=wb.xrepeat=wc.xrepeat=wd.xrepeat=wV.at(i+1).xrepeat;

                auto it=wV.begin() + sV.at(T).wallptr;
                wd.over=wa.over=wb.over=wc.over=false;

                wV.insert(it,wd);
                wV.insert(it,wc);
                wV.insert(it,wb);
                wV.insert(it,wa);

                sV.at(T).wallnum +=4;

                for (int k=T+1; k < (int)sV.size(); k++)
                    sV.at(k).wallptr+=4;

                for (int k=sV.at(T).wallptr+4; k < (int)wV.size(); k++)
                {
                    if (wV.at(k).point2 >= (sV.at(T).wallptr))
                        wV.at(k).point2+=4;
                };

                for (int k=0; k < (int)wV.size(); k++)
                {
                    if (wV.at(k).nextwall >= (sV.at(T).wallptr))
                        wV.at(k).nextwall+=4;
                    if (wV.at(k).extra >= 0 && wV.at(k).refer >= sV.at(T).wallptr)
                        wV.at(k).refer +=4;
                };
                sV.at(T).st_num.x=getSectors();
                sV.at(T).dxdy=glm::ivec2(dx,dy);
                sV.at(T).startrack=true;


                int nptr=(int)wV.size();

                unionSector ns=sV.at(T);
                ns.wallptr=wV.size();
                ns.wallnum=4;
                int h=sV.at(T).ceilingz;
                ns.floorz=h;
                ns.over=false;
                sV.push_back(ns);

                wa.nextsector=wb.nextsector=wc.nextsector=wd.nextsector=T;
                wa.extra=wb.extra=wc.extra=wd.extra=-1;

                wa.point2=nptr+1; // w3
                wd.point2=nptr+2; // w2
                wc.point2=nptr+3; // w1
                wb.point2=nptr;   // w0

                wa.nextwall=sV.at(T).wallptr+3;
                wd.nextwall=sV.at(T).wallptr+2;
                wc.nextwall=sV.at(T).wallptr+1;
                wb.nextwall=sV.at(T).wallptr;

                wV.push_back(wa);
                wV.push_back(wd);
                wV.push_back(wc);
                wV.push_back(wb);

                wV.at(wV.at(nptr).nextwall).nextwall=nptr;
                wV.at(wV.at(nptr+1).nextwall).nextwall=nptr+1;
                wV.at(wV.at(nptr+2).nextwall).nextwall=nptr+2;
                wV.at(wV.at(nptr+3).nextwall).nextwall=nptr+3;

                if ( x0 == x1 && y0 >= y1)
                    spV.at(p0).ang=512;
                else if (x0 == x1 && y0 <= y1)
                    spV.at(p0).ang=1536;
                else if (y0==y1 && x0 >= x1)
                    spV.at(p0).ang=0;
                else
                    spV.at(p0).ang=1024;
                 spV.at(p0).ang=1024;


                i+=4;

            } else if (xm == (spV.at(p0).x+dx) && ym == (spV.at(p0).y+dy))
            {
                refer << " found Star Track Door!" << " sector: "  << T << std::endl;
                //int i2=wV.at(i).point2;
                glm::ivec2 i1=glm::ivec2(wV.at(i).x, wV.at(i).y);     ///    ^i3-->
                glm::ivec2 i2=glm::ivec2(wV.at(wV.at(i).point2).x, wV.at(wV.at(i).point2).y);////    |     i4
                glm::ivec2 D=glm::ivec2(dx,dy);                       ///    |     |
                glm::ivec2 i3=i2-D;                                   ///    i2    |
                glm::ivec2 i4=i1-D;                                   ///    <--i1 *


                unionWall wa,wb,wc,wd;
                wa=wb=wc=wd=wV.at(i);
                wa.x=i1.x; wa.y=i1.y;
                wb.x=i4.x; wb.y=i4.y;
                wc.x=i3.x; wc.y=i3.y;
                wd.x=i2.x; wd.y=i2.y;

                wd.point2=sV.at(T).wallptr;
                wa.point2=sV.at(T).wallptr+1;
                wb.point2=sV.at(T).wallptr+2;
                wc.point2=sV.at(T).wallptr+3;

                wa.nextsector=wb.nextsector=wc.nextsector=wd.nextsector=sV.size();
                wa.xrepeat=wb.xrepeat=wc.xrepeat=wd.xrepeat=wV.at(i+1).xrepeat;

                auto it=wV.begin() + sV.at(T).wallptr;
                wd.over=wa.over=wb.over=wc.over=false;

                wV.insert(it,wd);
                wV.insert(it,wc);
                wV.insert(it,wb);
                wV.insert(it,wa);

                sV.at(T).wallnum +=4;

                for (int k=T+1; k < (int)sV.size(); k++)
                    sV.at(k).wallptr+=4;

                for (int k=sV.at(T).wallptr+4; k < (int)wV.size(); k++)
                {
                    if (wV.at(k).point2 >= (sV.at(T).wallptr))
                        wV.at(k).point2+=4;
                };

                for (int k=0; k < (int)wV.size(); k++)
                {
                    if (wV.at(k).nextwall >= (sV.at(T).wallptr))
                        wV.at(k).nextwall+=4;
                    if (wV.at(k).extra >= 0 && wV.at(k).refer >= sV.at(T).wallptr)
                        wV.at(k).refer +=4;
                };

                int nptr=(int)wV.size();

                sV.at(T).st_num.y=getSectors();
                sV.at(T).ssd=true;

                unionSector ns=sV.at(T);
                ns.wallptr=wV.size();
                ns.wallnum=4;
                int h=sV.at(T).ceilingz;
                ns.floorz=h;
                ns.over=false;
                sV.push_back(ns);

                wa.nextsector=wb.nextsector=wc.nextsector=wd.nextsector=T;
                wa.extra=wb.extra=wc.extra=wd.extra=-1;

                wa.point2=nptr+1; // w3
                wd.point2=nptr+2; // w2
                wc.point2=nptr+3; // w1
                wb.point2=nptr;   // w0

                wa.nextwall=sV.at(T).wallptr+3;
                wd.nextwall=sV.at(T).wallptr+2;
                wc.nextwall=sV.at(T).wallptr+1;
                wb.nextwall=sV.at(T).wallptr;

                wV.push_back(wa);
                wV.push_back(wd);
                wV.push_back(wc);
                wV.push_back(wb);

                wV.at(wV.at(nptr).nextwall).nextwall=nptr;
                wV.at(wV.at(nptr+1).nextwall).nextwall=nptr+1;
                wV.at(wV.at(nptr+2).nextwall).nextwall=nptr+2;
                wV.at(wV.at(nptr+3).nextwall).nextwall=nptr+3;

                if ( x0 == x1 && y0 >= y1)
                    spV.at(p1).ang=1536;
                else if (x0 == x1 && y0 <= y1)
                    spV.at(p1).ang=512;
                else if (y0==y1 && x0 >= x1)
                    spV.at(p1).ang=1024;
                else
                    spV.at(p1).ang=0;

                i+=4;
            }
        }
    }

    int ret =get_done(sV)-last;
    refer << "was done: " << ret << " StarTrack Doors Sectors" << std::endl;
    return EXIT_SUCCESS;
}




int blud2e::makeLighting(std::stringstream& refer)
{
    int last=get_done(spV);
    std::set<int> switch_list;
    for(auto it=spV.begin(); it != spV.end(); it++)
        if(it->over && it->isType("Toggle switch") && it->is("Light switch"))
            switch_list.insert(it-spV.begin());

    refer<< std::endl << "found Light Switch: ";
    for(auto T: switch_list)

    {
        refer << std::endl;
        refer<< T << " <-->  ";
        int ch=channel(spV.at(T).txID);
        auto sectors_list=findRx(sV, ch, -1, -1);
        std::vector<int> lamp_list;
        for (auto S: sectors_list)
        {
            auto it=sV.begin()+S;
            auto sprites=findAllSprites(it);
            for (auto L: sprites)
                if (spV.at(L).is("Lamp1"))
                    lamp_list.push_back(L);

            refer << S << " ";
            int amp=0;
            if (sV.at(S).over)
                //amp=sV.at(S).amplitude;
                amp=sV.at(S).floorshade+sV.at(S).amplitude;
            addSprite(S, ch, amp, "TC_LIGHTING", refer);
            if(sV.at(S).over && sV.at(S).wave > 5)
                lastSprite.vel.z=1;
            else
                lastSprite.vel.z=2;
            sV.at(S).done=true;
        }
        spV.at(T).done=true;
        spV.at(T).vel.y=ch;
        if ((int)lamp_list.size() > 0)
        {
            //refer << "found lamps: " << lamp_list.size() << std::endl;
            int chX=channel();
            int ch1=channel();
            for (auto& L: lamp_list)
            {
                spV.at(L).vel.y=chX;
            }
            addSprite(sectors_list.at(0), ch1, ch, "SWITCH", refer);
            lastSprite.vel.z=9;
            lastSprite.owner=1;
            addSprite(sectors_list.at(0), ch1, chX, "PushTrigger", refer);
            lastSprite.vel.z=3;
            lastSprite.tag.g=lamp_list.size();
            lastSprite.cstat=0;
            lastSprite.vel.x=chX;
            lastSprite.owner=1;

        }
    }
    std::set<int> lamp_sectors;
    for(auto it=spV.begin(); it != spV.end(); it++)
        if(it->is("Lamp1") || it->is("Lamp2") && (it->triggerVector || it->triggerImpact))
        {
            auto sector=it->inSector-sV.begin();
            auto sprites=findAllSprites(it->inSector);
            bool torch=false;
            for(auto S:sprites) if (spV.at(S).is("Torch"))
            {
                torch =true;
                break;
            }

            if (!torch && it->inSector->over)
                lamp_sectors.insert(it->inSector -sV.begin());
        };

    refer << std::endl << "found sectors witch breakables lamps: ";
    for (auto T: lamp_sectors)
    {
        //refer << T << " ";

        int ch1=channel();
        int ch2=channel();
        int ch3=channel();
        auto sec=sV.begin()+T;
        auto sList=findRx(sV, sec->rxID, -1, -1,true);
        if (sList.size() >0 )
        {
            refer << " (";
            for (auto Z:sList)
            {
                if(sV.at(Z).over && sV.at(Z).amplitude != 0)
                {
                    refer << Z << " ";
                    int amp=sV.at(Z).floorshade+sV.at(Z).amplitude;
                    addSprite(Z, ch1, amp , "TC_LIGHTING", refer);
                    //lastSprite.tag.g=1;
                    //if (sV.at(Z).wave >= 4)
                    lastSprite.tag.g=2;
                }
            }
            refer << "), ";
        }
        auto lamps_here=findAllSprites(sec);
        int l=0;
        for(auto L: lamps_here)
            if ((spV.at(L).is("Lamp1") || spV.at(L).is("Lamp2")) &&
                 (spV.at(L).triggerVector || spV.at(L).triggerImpact))
            {
                l++;
                 spV.at(L).vel.y=ch3;
            }
        addSprite(T, ch2, ch1, "SWITCH", refer);
        lastSprite.vel.z=9;
        lastSprite.owner=1;
        addSprite(T, ch2, ch3, "PushTrigger", refer);
        lastSprite.vel.z=3;
        lastSprite.tag.g=l;
        lastSprite.cstat=0;
        lastSprite.vel.x=ch3;
        lastSprite.owner=1;
    }

    int j=0;
    for(auto& T: sV) if (T.over && T.amplitude != 0)
    {
        auto sector=sV.begin()+T.getNum();
        auto sprites=findAllSprites(sector);
        bool torch=false;
        for(auto S:sprites) if (spV.at(S).is("Torch"))
        {
            torch =true;
            break;
        }
        if (!torch)
        {
            int amp=T.floorshade+T.amplitude;
            addSprite(T.getNum(), 0, amp, "TC_LIGHTING", refer);
            lastSprite.tag.g=1;
            if ( T.wave > 5)
            {
                lastSprite.vel.z=4;
                //lastSprite.owner=1;
                int ch=channel();
                lastSprite.tag.r=ch;
                T.tag.g=ch;
            } else
            {
                lastSprite.vel.z=2;

            }

            j++;
        }

    }
    refer << std::endl << "add " << j << " static light.";

    refer << std::endl;


    check(refer);
    int ret =get_done(spV)-last;
    refer << "was done: " << ret << " Light Switches" << std::endl;
    return ret;
};

int unionSprite::makeAmbient(int musicID, int radius)
{
		texture_id =5;
		tag=glm::ivec3(musicID, radius, -1);
		cstat=0;
        return 0;
};

void blud2e::Cstat()
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
            5778, // fire extinguisher
            5809,
            5810,
            6026,            
            6035,
            6212
        };

        if (bottle.count(T.texture_id))
        {
            T.cstat &=~(1<<7);
            continue;
        };

        glm::ivec2 size=RS.get_resolution(T.texture_id);
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

void blud2e::makeEnemies()
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
                    T.vel.x=the_key->vel.y=ch;
            };
        };
};

void blud2e::makeSectorSFX(unionSprite& Sp)
{
    Sp.tag=glm::ivec3(RS.trans(Sp.data1), RS.trans(Sp.data2), -1);
    Sp.shade=Sp.pal=0;


    if (Sp.isType("SFX Gen"))  // make TriggerSound
    {
        Sp.texture_id=19;
        if (Sp.rxID >=100)
        {
            Sp.vel=glm::ivec3(channel(Sp.rxID), Sp.txID, 0);
        } else
            Sp.vel.z=1;

        Sp.done=true;
        Sp.tag=glm::ivec3(RS.trans(Sp.data2), RS.trans(Sp.data1), -1);
    } else if (Sp.isType("Sector SFX"))
    {
        if (!Sp.inSector->over || Sp.inSector->is("Normal"))
        {
            Sp.texture_id=19;
            if (Sp.rxID >=100)
            {
                Sp.vel=glm::ivec3(channel(Sp.rxID), channel(Sp.txID), 0);
            } else
                Sp.vel.z=1;
            Sp.done=true;

        }
    }
    if ((Sp.data3 > 0 && Sp.data4 > 0) && (Sp.data2 != Sp.data4 || Sp.data1 != Sp.data3))
    {
        unionSprite nsp = Sp;
        nsp.tag=glm::ivec3(RS.trans(Sp.data3), RS.trans(Sp.data4), -1);
        nsp.vel.z=1;
        nsp.pos.z=Sp.inSector->floor.z;
        nsp.over=false;
        nsp.setNum(getSprites());
        spV.push_back(nsp);
    };

    if (Sp.isType("SFX Gen") && Sp.tag.r == 957)  // !!!!! fix initial sound for E1M1
        Sp.tag.r=1010;



};

int blud2e::makePlayerSFX(std::stringstream& refer)
{
    int count=0;
    for(auto& S: spV)
    {
        if ( S.over && S.isType("Decoration") && S.txID >= 100 & S.rxID >=100)
        {
            auto sensors=findTx(spV, S.rxID);
            auto activators=findRx(spV, S.txID);
            if ((int)sensors.size() > 0 && (int)activators.size() > 0)
            {
                bool done=false;
                for(auto A: activators)
                {
                    if (spV.at(A).isType("Player SFX"))
                    {
                        done=true;
                        spV.at(A).texture_id=19;
                        spV.at(A).vel=glm::ivec3(spV.at(A).rxID, spV.at(A).txID, 0);
                    }

                }
                if (done)
                {
                    refer << "was found Player SFX: " << S.getNum() << SE;
                    S.vel.y=S.txID;
                    count++;
                }
            }


        }

    }

    refer << "make Player SFX: " << count << std::endl;
    return EXIT_SUCCESS;
}

void blud2e::makeRespawn(std::stringstream& msg)
{
    int r_count, g_count;
    r_count=g_count=0;
    std::vector<unionSprite> stack;
    for(auto& Sp: spV)
    {

        if ((Sp.isType("Explode Object") || Sp.isType("Gib Object") || enemies.count(Sp.lotag) || Sp.isType("Dude Spawn")) &&
                (Sp.dropId > 0 || Sp.isType("Dude Spawn")))
        {
            unionSprite reserved=Sp;
            Sp.texture_id=7197;
            Sp.cstat=0x81d0;
            Sp.tag.r=type_to_pic[Sp.data1];
            if ((Sp.launch1 + Sp.launch2 + Sp.launch3 + Sp.launch4 + Sp.launch5) > 0)
                Sp.tag.g=(1-Sp.launch1)+((1-Sp.launch2)<<1)+((1-Sp.launch3)<<2)+((1-Sp.launch4)<<3)+((1-Sp.launch5)<<4);
            if (reserved.texture_id !=7197)
            {
                Sp.tag.r=type_to_pic[Sp.dropId];
                reserved.over=false;
                stack.push_back(reserved);
                g_count++;
            };
            r_count++;
        };

    };
    for(auto& T: stack)
    {
        T.setNum(getSprites());
        spV.push_back(T);
    }
    msg << "Respawn sprites was generated: " << g_count << std::endl;
    msg << "Respawn sprites was processed: " << r_count << std::endl;
};

void blud2e::makeExplodeAndGib()
{
    for(auto& Sp: spV)
        if (Sp.over && (Sp.isType("Explode Object")|| (Sp.isType("Gib Object")
            && !(Sp.cstat & 0x01) && Sp.texture_id != 5778 )))
        {
            Sp.tag=glm::ivec3(Gibs[Sp.data1], RS.trans(Sp.data4), -1);
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
                nsp.setNum(getSprites());
				spV.push_back(nsp);
				if ( Sp.data3 > 0 )
				{
                    //nsp.tag.r=Gibs[Sp.data3];
                    //spV.push_back(nsp);
				};
			}
        };
};

int blud2e::prepare(std::stringstream& msg)
{
    if (RS.load_tables(msg) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }

    RS.get_all_target([&](int _v) { channel.addSound(_v);});

	for (auto& T: wV)
	{
        static int n=0; T.setNum(n);
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
        if (PN ==  2342)
        {
            T.texture_id=T.outside_texture_id=9739;
        }
        n++;
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
        static int n=0; T.setNum(n++);
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
        static int n=0; T.setNum(n++);
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
    std::set<int> erTex;
    for (auto& T: wV) {
        T.res=RS.get_resolution(T.texture_id);
        if (T.res.x == 0 && T.res.y == 0)
        {
            if (!erTex.count(T.texture_id))
                msg << "ERROR: not found resolution for wall texture_id: " << T.texture_id << std::endl;
            erTex.insert(T.texture_id);
            T.res=glm::ivec2(1.f);
        }
        channel.addReserved(T.getTX());
        channel.addReserved(T.getRX());

    }
    for (auto& T: spV) {
        T.res=RS.get_resolution(T.texture_id);
        if (T.res.x == 0 && T.res.y == 0)
        {
            if (!erTex.count(T.texture_id))
                msg << "ERROR: not found resolution for sprite texture_id: " << T.texture_id
                    << " in sector: " << T.sectnum << std::endl;
            erTex.insert(T.texture_id);
            T.res=glm::ivec2(1.f);
        }
        channel.addReserved(T.getTX());
        channel.addReserved(T.getRX());
    }
    for (auto& T: sV) {
        T.floor.res=RS.get_resolution(T.floor.texture_id);
        if (T.floor.res.x == 0 && T.floor.res.y == 0)
        {
            if (!erTex.count(T.floor.texture_id))
                msg << "ERROR: not found resolution for floor texture_id: " << T.floor.texture_id << std::endl;
            erTex.insert(T.floor.texture_id);
            T.floor.res=glm::ivec2(1.f);
        }
        T.ceiling.res=RS.get_resolution(T.ceiling.texture_id);
        if (T.ceiling.res.x == 0 && T.ceiling.res.y == 0)
        {
            if (!erTex.count(T.ceiling.texture_id))
                msg << "ERROR: not found resolution for ceiling texture_id: " << T.ceiling.texture_id << std::endl;
            erTex.insert(T.ceiling.texture_id);
            T.ceiling.res=glm::ivec2(1.f);
        }
        channel.addReserved(T.getTX());
        channel.addReserved(T.getRX());
    };
    msg << "count reserved channals is: " << channel.getSizeReserved() << std::endl;
    channel.printReserved(msg);
    check(msg);
    return EXIT_SUCCESS;
};

int blud2e::get_prv_point(int num, int point)
{
    for (int i=0; i < (int)wV.size(); i++)
    {
        if (wV.at(i).point2 == point)
            return i;
    };
    return -1;
};

int blud2e::remove_wall(int num, std::stringstream& msg, bool nextwall=true)
{
   if ((int)wV.size() <=  num)
        return EXIT_FAILURE;

    auto it=wV.begin() + num;

    int sCount=sV.size()-1;
    while (sV.at(sCount).wallptr > num)
        sCount--;

    if (wV.at(num).nextwall != -1 && nextwall)
        remove_wall(wV.at(num).nextwall, msg , false);

    int pp=get_prv_point(sCount, num);
    wV.at(pp).point2=wV.at(num).point2;
    wV.erase(it);
    sV.at(sCount).wallnum--;

    for (int i=sCount+1; i < (int)sV.size(); i++)
            sV.at(i).wallptr--;

    for (int i=0; i < (int)wV.size(); i++)
    {
        if (wV.at(i).point2 >= num)
            wV.at(i).point2--;
    };

    for (auto it=wV.begin();it != wV.end(); ++it)
    {
        if (it->nextwall >= num)
            it->nextwall--;
        if (it->extra >= 0 && it->refer >= num)
            it->refer--;
    };

    return EXIT_SUCCESS;
};

int blud2e::slideROR(std::stringstream& refer)
{
    std::set<int> e1m1ROR={65,146,145,55, 51,50,44,45,56};
    std::set<int> joint;
    bool map_size=true;
    for (auto T: e1m1ROR) if ((int)sV.size() <= T)
        map_size=false;

    if (map_size && sV.at(65).marker0 == 138 && sV.at(65).marker1 == 139)
    {
        refer <<std::endl<<  "map E1M1.MAP detect" << std::endl;
        mapIs="e1m1";
        auto m0=spV.begin()+140; // upper stack
        auto m1=spV.begin()+0;   // lower stack
        int dx=m1->x - m0->x;
        int dy=m1->y - m0->y;
        refer << "dx: " << dx << " dy: " << dy<< std::endl;
        sV.at(65).ceilingz=sV.at(90).floorz;

        for(auto T: e1m1ROR)
            for(int i=0, j=sV.at(T).wallptr; i < sV.at(T).wallnum; i++, j++ )
            {
                if (wV.at(j).nextsector >= 0 && !e1m1ROR.count(wV.at(j).nextsector))
                    joint.insert(wV.at(j).nextsector);
            }

        if (joint.size() > 0)
        {   refer << "joint sectors: ";
            for(auto T: joint)
                refer << T << " ";
            refer << std::endl;
        }

        for (auto T: joint)
        {
            for(int i=sV.at(T).wallptr; i < (sV.at(T).wallptr + sV.at(T).wallnum); i++)
            {
                int ns=wV.at(i).nextsector;
                int nw=wV.at(i).nextwall;
                int p2=wV.at(i).point2;
                int nsp2=wV.at(p2).nextsector;

               if (ns >= 0 && e1m1ROR.count(ns))
               {
                   wV.at(i).x -=dx;
                   wV.at(i).y -=dy;
               }
               if (nsp2 < 0 && e1m1ROR.count(ns))
               {
                   wV.at(p2).x -=dx;
                   wV.at(p2).y -=dy;
               };
            }
        }

        for(auto T: e1m1ROR)
        {
            for(int i=sV.at(T).wallptr; i < (sV.at(T).wallptr + sV.at(T).wallnum); i++)
            {
               wV.at(i).x -=dx;
               wV.at(i).y -=dy;
            }
        }

        for (auto& T: spV)  if (e1m1ROR.count(T.sectnum))
        {
            T.x -=dx;
            T.y -=dy;
        }

    }
    wall_subdivision_ror(65, 90, refer);
    return EXIT_SUCCESS;
}

int blud2e::wall_subdivision_ror(int lower, int upper, std::stringstream& refer)
{

    int first_lower=get_wall_maplevel(upper, wV.at(sV.at(lower).wallptr).x, wV.at(sV.at(lower).wallptr).y, refer);
    int first_upper=get_wall_maplevel(lower, wV.at(sV.at(upper).wallptr).x, wV.at(sV.at(upper).wallptr).y, refer);

    if (first_lower < 0 || first_upper < 0)
    {
        refer << "ERROR: can't find first walls\n";
        return EXIT_FAILURE;
    }

   refer << SE <<"wall count "
          << lower << ": "
          << sV.at(lower).wallnum
          << " first: "
          <<  sV.at(lower).wallptr
          << " analog: "
          <<  first_lower
          << std::endl;

   refer << "wall count "
          << upper << ": "
          << sV.at(upper).wallnum
          << " first: "
          << sV.at(upper).wallptr
          <<  " analog: "
          <<  first_upper
          <<  std::endl;

    std::vector<WL> st;
    for(int i=sV.at(lower).wallptr; i < (sV.at(lower).wallptr + sV.at(lower).wallnum); i++)
    {
        int u=get_wall_maplevel(upper, wV.at(i).x, wV.at(i).y, refer);
        if (u >= 0)
        {
            glm::ivec2 l0=glm::ivec2(wV.at(i).x, wV.at(i).y);
            glm::ivec2 l1=glm::ivec2(wV.at(wV.at(i).point2).x, wV.at(wV.at(i).point2).y);
            glm::ivec2 u0=glm::ivec2(wV.at(u).x, wV.at(u).y);
            glm::ivec2 u1=glm::ivec2(wV.at(wV.at(u).point2).x, wV.at(wV.at(u).point2).y);
            assert(l0 == u0);
            int dL=glm::distance(l0,l1);
            int dU=glm::distance(u0,u1);

            if (dL > dU)
            {
                //division_wall_maplevel(i,refer, wV.at(wV.at(u).point2).x, wV.at(wV.at(u).point2).y, true);
                //wall_subdivision_ror(lower, upper, refer);
                //return EXIT_SUCCESS;
                refer << "lower wall: " << i << std::endl;
                WL wl;
                wl.num=i;
                wl.pt=u1;
                st.push_back(wl);
            } else if (dL < dU)
            {
                //division_wall_maplevel(u,refer, wV.at(wV.at(i).point2).x, wV.at(wV.at(i).point2).y, true);
                //wall_subdivision_ror(lower, upper, refer);
                //return EXIT_SUCCESS;
                refer << "upper wall: " << u << std::endl;
                WL wl;
                wl.num=u;
                wl.pt=l1;
                st.push_back(wl);
                break; //// !!! remove it
            }
        }
    }
    if ((int)st.size() > 0)
    {
        auto it=st.begin();
        refer << "found wall for division: " << st.size() << SE;
        refer << "wall division: " << it->num << " x: " << it->pt.x << " y: " << it->pt.y <<SE;
        division_wall_maplevel(it->num, it->pt.x, it->pt.y, true);
        wall_subdivision_ror(lower, upper, refer);
    }
    return EXIT_SUCCESS;
}

int blud2e::division_wall_maplevel(int wall, int x=-1, int y=-1, bool nextwall=true)
{
    int x1,y1, x2, y2;
    if (wall < 0 || wall >= (int)wV.size())
        return EXIT_FAILURE;

    if (x == -1 && y == -1) {
        x=(wV.at(wall).x+wV.at(wV.at(wall).point2).x)/2;
        y=(wV.at(wall).y+wV.at(wV.at(wall).point2).y)/2;
    };

    if ( nextwall && wV.at(wall).nextwall >= 0 && wV.at(wall).nextwall < wall)
    {
        wall= wV.at(wall).nextwall;
    }

    unionWall nw=wV.at(wall);
    int nwall=wV.at(wall).nextwall;

    if (wV.at(wall).nextwall >= 0 && nextwall )
    {
        division_wall_maplevel(wV.at(wall).nextwall,x,y,false);
        nw.nextwall++;
    }
    // X_REPEAT - wall texturing
    glm::ivec2 a=glm::ivec2(wV.at(wall).x, wV.at(wall).y);
    glm::ivec2 b=glm::ivec2(x, y);
    glm::ivec2 c=glm::ivec2(wV.at(wV.at(wall).point2).x, wV.at(wV.at(wall).point2).y);
    float dist_full=glm::distance(a,c);
    float dist_old=glm::distance(b,c);
    float dist_new=glm::distance(a,b);
    wV.at(wall).xrepeat *=dist_old/dist_full;
    nw.xrepeat *=dist_new/dist_full;

    wV.at(wall).x=x;
    wV.at(wall).y=y;

    nw.point2=wall+1;

    int sCount=sV.size()-1;
    while (sV.at(sCount).wallptr > wall)
        sCount--;

    SC << "Division Sector: " << sCount << std::endl;
    SC << "Division Wall: " << wall << std::endl;
    sV.at(sCount).wallnum++;

    auto it=wV.begin()+wall;
    nw.over=false;

    wV.insert(it,nw);

    for (int i=sCount+1; i < (int)sV.size(); i++)
            sV.at(i).wallptr++;

    for (int i=wall+1; i < (int)wV.size(); i++)
    {
        if (wV.at(i).point2 > wall)
            wV.at(i).point2++;
    };


    for (auto it=wV.begin();it != wV.end(); ++it)
    {
        if (it->nextwall > wall)
            it->nextwall++;
        if (it->extra >= 0 && it->refer >= wall)
            it->refer++;
    };

    if (nextwall && wV.at(nwall+1).nextsector >0)
        wV.at(++nwall).nextwall++;

    return EXIT_SUCCESS;
}

int blud2e::finish()
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

int blud2e::makeQuotes(std::stringstream& msg)
{
    msg << std::endl << "Quote Sector: "<< " sprite: ";
    for(auto T: spV)
    {
        if(T.over && T.rxID >= 100 && T.txID == 3)
        {
            auto the_list=findDoneTx(sV,T.rxID);
            for(auto& S: the_list)
            {
                auto it=sV.begin()+S;
                auto sprites=findAllSprites(it);
                for (auto& F: sprites)
                    if(spV.at(F).texture_id == 18)
                    {
                        spV.at(F).texture_id=20;
                        T.texture_id=9117;
                        msg << T.getNum() << " ";
                    }
            }
        }
    }

    msg <<std::endl;
    return EXIT_SUCCESS;
};

int blud2e::processing(std::stringstream& msg, const float scope=1.f) {
    scale=scope;
    prepareSlideSector(msg);
    slideROR(msg);

    makeSlideDoors(msg);

    makeStarTrackDoors(msg);


    if (prepare(msg) == EXIT_FAILURE)
    {
        msg << "ERROR: missing files: sounds.con or sounds_old.con or defs.con or pic_table.con" << std::endl;
        return EXIT_FAILURE;
    }

    msg << std::endl << "Start processing..." << std::endl;

    for (auto& T : spV)
    {   // parsing picnumber
        if (T.is("levelLocker"))
            T.makeLevelLocker();
        else if (T.is("FootTraces"))
            T.makeFootTraces();
        if (T.isType("Ambient SFX"))
           T.makeAmbient(RS.trans(T.data3), T.data2*12);
        else if(T.isType("WaterDrip Gen"))
           T.makeWaterDripGen();
        else if(T.isType("BloodDrip Gen"))
            T.makeBloodDripGen();
        else if(T.isType("Toggle switch") || T.isType("1-Way switch"))
            T.tag.g=RS.trans(T.data1);
        else if(items.count(T.lotag) && T.launchS) // mulipalyer
            T.pal=1;
        else if(items.count(T.lotag) && T.launchT && T.launchB)
            T.pal=2;
        else if(T.isType("Sector SFX") || T.isType("SFX Gen") || T.isType("Player SFX"))
            makeSectorSFX(T);

        if (T.is("Lamp1") || T.is("Lamp2"))
            T.cstat=0x121;

    };
    makeRespawn(msg);
    Cstat();
    makeExplodeAndGib();
    makeEnemies();
    makeLighting(msg);

    // secret room
    for (auto& T: sV)
    {
        if ( T.over && T.txID == 2)  LT=32767;
        if (T.over && T.panFloor)
        {
            addSprite(T.getNum(), 0, 0, "SE10", msg);
            lastSprite.tag.r=24;
            lastSprite.ang=T.panAngle;
            T.floorstat=0x30;
        }
    }

    for (auto& T: spV)  if(T.isType("Hidden Exploder")) T.makeHiddenExploder();

    check(msg);
    makeExplosiveSector(msg);

    makeElevatorSector(msg);
    makeDoomDoors(msg);
    makeSlideSector(msg);
    makeEnterSensor(msg);
    makeRotateSector(msg);
    makeSDSS(msg);
    makeZMotionSrite(msg);
    makeStepRotate(msg);

    makePlayerSFX(msg);
    makeController(msg);
    makeQuotes(msg);

    makeTROR(msg);

    for (auto& T: spV) // start position
    {
        if ( T.isType("Player Start") && T.texture_id == 7642 && !T.data1)
        {
            dh.X=T.pos.x; dh.Y=T.pos.y; dh.Z=T.pos.z;
            dh.angle=T.ang;
            dh.sector=(short)(T.inSector-sV.begin());
            msg << "Start position: " << "X: " << dh.X << " Y: " << dh.Y << " Z: " << dh.Z
                << " Sector: " << dh.sector<< std::endl;
            auto the_list=findRx(spV, 7, -1, -1, true);
            if ((int)the_list.size() > 0)
            {
                bool flag=false;
                for(auto S: the_list )
                {
                    if (spV.at(S).txID >= 100 && spV.at(S).rxID == 7)
                    {
                        addSprite((spV.at(S).inSector-sV.begin()), 7, spV.at(S).txID, "SWITCH", msg, spV.at(S).pos);
                        spV.at(S).texture_id=9117; // for remove
                        spV.at(S).lotag=0; // throw out from the post-processing
                        flag=true;
                    }
                }

                if (flag)
                {
                    addSprite(dh.sector, 0, 0, "PushTrigger", msg);
                    lastSprite.owner=1;
                    lastSprite.vel.y=7;
                };
            }
        }
    }

    std::set<int> the_list={};
    for (auto T: sV) if (T.over && !T.done && T.lotag) the_list.insert(T.refer);

    msg  <<std::endl << "undone sectors: ";
    for (auto T: the_list) msg << T << " ";msg << std::endl;

    // remove sprites
	std::set<int> robj{9117, 5873, 7202, 7451, 7452, 7642, 7643, 7644, 7645, 7646, 7647, 7648, 7649};
    auto it=spV.begin(); int last=spV.size();

    while (it != spV.end())
        if (robj.count(it->texture_id))
            spV.erase(it);
        else
			++it;

    msg << std::endl << "was removed: " << (last-spV.size()) << " sprites" << std::endl;
    msg << "count used channals is: " << channel.getSize() << std::endl;
    channel.printUsed(msg);
    msg << "count UNused channals is: ";
    channel.printUnUsed(msg);
    //remove_wall(1, msg);
    finish();
    return EXIT_SUCCESS;
};

int blud2e::check(std::stringstream& refer)
{
    bool flag=false; refer<<SE;
    for (auto i:sV)
    {
        if (i.wallnum != i.wall_count())
        {
            refer << "Have collision in wall counter: " << i.wallnum << " " << i.wall_count() << SE;
            flag=true;
        };
        if (i.wallptr != (i.firstWall - wV.begin()))
        {
            refer << "Have collision in wall pointer: " << i.wallptr << " " << (i.firstWall-wV.begin()) << SE;
            flag=true;
        };
    };
    if (!flag)
        refer << "check of the map structure was success.\n";

    int n=0;
    for (auto T: sV)
    {

        if (T.getNum() != n)
            refer << " corrupt in Sector number: " << n<< " inside number: " << T.getNum() << " Refer: "<< T.getRefer() << std::endl;
        if (T.getRefer() >= 0  && T.getRefer() != n)
            refer << " corrupt in Sector refer: " << n << std::endl;
        n++;
    }

    n=0;
    for (auto T: spV)
    {
        if (T.getNum() != n)
            refer << " corrupt in Sprite number: " << n<< " inside number: " << T.getNum() << " Refer: "<< T.getRefer() << std::endl;
        if (T.getRefer() >= 0  && T.getRefer() != n)
            refer << " corrupt in Sprite refer: " << n << std::endl;
        n++;
    }
    n=0;
    for (auto T: wV)
    {
        if (T.getNum() != n)
           refer << " corrupt in Wall number: " << n<< " inside number: " << T.getNum() << " Refer: "<< T.getRefer() << std::endl;
        if (T.getRefer() >= 0  && T.getRefer() != n)
            refer << " corrupt in Wall  refer: " << n << std::endl;
        n++;
    }
    return EXIT_SUCCESS;
};




/* Convertor MAP files by Blood video game, Monolith 1997(c) to eduke32 map format
   Project: Blood CrossMatching
   Author: flanker
   It's fork by blud2b.c, HTTP://blood.sourceforge.net with additional features
   License: GPL v.3
*/

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <iostream>
#include <cassert>
#include <string>
#include <map>
#include <blud2e.h>
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

const int size_signature=6;
const int size_firstHeader=18;
const int size_secondHeader=9;
const int size_thirdHeader=10;
const int size_extra=128;
const int size_unknownElts=2;
const int size_sector=40;
const int size_extra_sector=60;
const int size_wall=32;
const int size_extra_wall=24;
const int size_sprite=44;
const int size_extra_sprite=56;

bool isEncrypted=false;

struct firstHeader {
	int startX;
	int startY;
	int startZ;
	short startAngle;
	short sectorNum;
	short unknownElts;
};

struct thirdHeader {
	int mapRevisions;
	short numSectors;
	short numWalls;
	short numSprites;
};

int DecryptBuffer (unsigned char* Buffer, const size_t DataSize, unsigned char DecryptKey) {

	assert (Buffer != NULL);
	// If the map isn't encrypted
	if (! isEncrypted)
		return 1;
	// Decryption
	for (unsigned short i = 0; i < DataSize; i++)
        	Buffer[i] ^= (unsigned char)(DecryptKey + i);
	return 0;
};

template<typename T, typename T1> void writeVector(T &the_vector, T1 &chuck, std::ofstream &file)
{
    unsigned short lenght=(unsigned short)the_vector.size();
	file.write((char*)(&lenght), sizeof(lenght));
    for ( auto it: the_vector)
    {
        chuck=dynamic_cast<T1&>(it);
        file.write((char*)(&chuck), sizeof(T1));
    };
};

void getw(std::string& t, std::ifstream& in) {
	in>>t;
};

int read_string(std::vector<std::string> &words, std::ifstream& in) {
	int i=0;
	while (!in.eof())
		getw(words[i++],in);
	return (i-1);
};

///////----- M A P    C L A S S ////////////////////////////
void Map::showInfo() {
    std::cerr << "===== MAP INFO ============" << std::endl
        << "Initial position: "  << std::endl
        << "startX: " << dh.X  << std::endl
        << "startY: " << dh.Y  << std::endl
        << "startZ: " << dh.Z  << std::endl
        << "startAngle: " << dh.angle  << std::endl
        << "sectorNum: " << dh.sector  << std::endl;
        if (isEncrypted)
        {
            std::cerr << std::endl << "Encrypted: yes" << std::endl;
            std::cerr << "Map revision: " << Revision  << std::endl;
        } else
            std::cerr << std::endl << "Encrypted: no" << std::endl;

    std::cerr << "map version: " << dh.version << std::endl;
    std::cerr << "amount Sectors: " << sV.size()  << std::endl;
    std::cerr << "amount Walls: " << wV.size()  << std::endl;
    std::cerr << "amount Sprites: " << spV.size()  << std::endl;


};

////////////////////// W R I T E ////////////////////////////////////
int  Map::write(char *filename) {

    std::ofstream out(filename, std::ofstream::binary);
    if (out.is_open())
    {
    assert(sizeof(Sector) == size_sector && sizeof(Sprite)  == size_sprite &&
        sizeof(Wall) == size_wall && sizeof(dh) == 20);

    out.write((char*)(&dh), sizeof(dh));

    Sector newSec; writeVector(sV, newSec, out);
    Wall newWall; writeVector(wV, newWall, out);
    Sprite newSpr; writeVector(spV, newSpr, out);

	out.close();
	} else {
        std::cerr << "ERROR: can't open file " << *filename << " for write!" <<SE;
        return -1;
	}

    wV.erase(wV.begin(), wV.end());
    spV.erase(spV.begin(), spV.end());
    sV.erase(sV.begin(), sV.end());
    return 0;
};

////////////////// R E A D /////////////////////////////////////////////
int Map::read(char *filename)
{
	// open file
	std::ifstream in (filename, std::ifstream::binary);

	if (in.is_open())
	{
		unsigned short amountSectors, amountWalls, amountSprites;
		bool blood_format=true;
        // calculate lenght of file
        in.seekg (0, in.end);
		lengthMap = in.tellg();
		in.seekg (0, in.beg);

		assert(lengthMap >= 26) ; // minimal size for empty map of duke format

        // get work buffer
		char Buffer[128];
		char * ptr= (char*)Buffer;
		unsigned char sign[6];

		in.read((char*)&sign, sizeof(sign));

		// if Blood map format
		if ( (sign[0] ^ sign[1] ^ sign[2] ^ sign[3]) == 0x59 )
            {
                std::cerr << "Blood MAP format detection..." <<std::endl;
                if (sign[4] == 3 && sign[5] == 6 )
                    dh.version=63;
                else if (sign[4] == 0 && sign[5] == 7)
                {
                    dh.version=7;
                    isEncrypted=true;
                } else
                {
                    std::cerr << "\nDecryptor can only handle map versions 6.3 and 7.0.\n" <<
                    "Try bringing the map up in the latest version of mapedit, then save it.\n" <<
                    "This should produce a 7.0 version map, which Decryptor CAN convert to build." << std::endl;
                    in.close();
                    return -1;
                };

                // read magic+version
                in.read(ptr,size_firstHeader);

                DecryptBuffer ((unsigned char*)ptr, size_firstHeader, 0x4D);
                firstHeader &hd1=*(firstHeader*)Buffer;

                dh.X=hd1.startX;
                dh.Y=hd1.startY;
                dh.Z=hd1.startZ;
                dh.angle=hd1.startAngle;
                dh.sector=hd1.sectorNum;
                unsigned int NbUnknownElements = (1 << hd1.unknownElts);

                std::cerr << "count unknownElts: " << NbUnknownElements << std::endl;

                // seek second header
                in.seekg(size_secondHeader, in.cur);

                // read third header
                in.read(ptr,size_thirdHeader);
                DecryptBuffer ((unsigned char*)ptr, size_thirdHeader, 0x68);
                thirdHeader &h3=*(thirdHeader*)Buffer;
                amountSectors=h3.numSectors;
                amountWalls=h3.numWalls;
                amountSprites=h3.numSprites;
                Revision=h3.mapRevisions;

                // seek to position to begin of first sector
                long offset = (isEncrypted) ? size_extra : 0;
                offset += (NbUnknownElements * size_unknownElts);
                in.seekg(offset, in.cur);
           } else
           {
                in.seekg(0, in.beg);
                in.read((char*)&dh, sizeof(dh));
                if ( dh.version < 6 && dh.version > 10 )
                {
                     std::cerr << "Incorrect format file!" << std::endl;
                     in.close();
                     return -1;
                };
                std::cerr << "Duke MAP format detection..." <<std::endl;
                in.read((char*)&amountSectors, sizeof(amountSectors));
                blood_format=false;
           };

            unsigned char  decryptKey = ((Revision * size_sector) & 0xFF);
            // read sector's array
            for ( int i=0; i< amountSectors; i++)
            {
                unionSector S;
                Sector& z=dynamic_cast<Sector&>(S);
                in.read(ptr,size_sector);
                DecryptBuffer ((unsigned char*)ptr, size_sector, decryptKey);
                z=*(unionSector*)Buffer;
                if (blood_format && S.extra > 0)
                {
                    in.read(ptr, size_extra_sector);
                    S.over=true;
                    xSector& xz=dynamic_cast<xSector&>(S);
                    xz=*(xSector*)Buffer;
                };
                sV.push_back(S);
            };

            // WALLS---------------------------------
            if (!blood_format)
                in.read((char*)&amountWalls,2);

            decryptKey = (((Revision * size_sector) | 0x4d) & 0xFF);
            for ( int i=0; i< amountWalls; i++)
            {
                unionWall W;
                Wall& z=dynamic_cast<Wall&>(W);
                in.read(ptr,size_wall);
                DecryptBuffer ((unsigned char*)ptr, size_wall, decryptKey);
                z=*(unionWall*)Buffer;
                if (blood_format && W.extra > 0)
                {
                    in.read(ptr, size_extra_wall);
                    W.over=true;
                    xWall& xz=dynamic_cast<xWall&>(W);
                    xz=*(xWall*)Buffer;
                };
                wV.push_back(W);
            };

            // SPRITES
            if (!blood_format)
                in.read((char*)&amountSprites,2);

            decryptKey = (((Revision * size_sprite) | 0x4d) & 0xFF);
            for ( int i=0; i< amountSprites; i++)
            {
                unionSprite SP;
                Sprite& z=dynamic_cast<Sprite&>(SP);
                in.read(ptr,size_sprite);
                DecryptBuffer ((unsigned char*)ptr, size_sprite, decryptKey);
                z=*(unionSprite*)Buffer;
                if (blood_format && SP.extra > 0)
                {
                    in.read(ptr, size_extra_sprite);
                    SP.over=true;
                    xSprite& xz=dynamic_cast<xSprite&>(SP);
                    xz=*(xSprite*)Buffer;
                };
                spV.push_back(SP);
            };

            showInfo();
        } else{
            std::cerr << "ERROR: can't open file: " << filename << SE;
            return -1;
        };

	in.close();
    return 0;
};

///////////////////  S H O W ///////////////////////////////////////////
void Map::printSector(int  num, bool blood)
{
    /// prepare
    std::map<int, std::string> secType = {{600, "Z Motion"}, {0, "Normal"}};
	std::map<int, std::string> wallType = {{ 0, "Normal"} ,{20, "Toggle switch"}, {21, "1-way switch"},
		{500, "Wall Link"}, {501, "Wall stack"}, {511, "Gib Wall"}};
	std::map<int, std::string> spriteType = {{0, "Decoration"}, {1, "Player start" }};
	// check range of the nummber
	if ( (unsigned int)num >= sV.size())
	{
		std::cerr << "ERROR: number of sector greater than you number!" << std::endl;

		return;
    };

	std::cout << "----------------------------------" << std::endl;
	std::cout << "Sector# " << num << std::endl;

	// main body /////////////////////////////////////////////////////////////////
	sV.at(num).print();

    for(auto it=wV.begin()+sV.at(num).wallptr; it < wV.begin()+sV.at(num).wallptr+sV.at(num).wallnum; it++)
    {

        it->print();
       // std::cout << "nextPoint: " << it->nextPoint->refer << std::endl;
        //std::cout << "previousPoint: " <<  it->prePoint->refer << std::endl;
    };

    for( auto T: spV)
//        if ( T.inSector == (sV.begin()+num))
        if ( T.sectnum == num)
			T.print();
///////////////// CUT HERE ////////////////////////////////////////////
  prepare();

    std::cout << "loops: " << sV.at(num).loops.size() <<  std::endl;
    std::cout << "generic: " << sV.at(num).generic->marker-wV.begin() <<  std::endl;

    if (sV.at(num).generic->property("proper"))
        std::cout << "Sector is proper" << std::endl;
    if (sV.at(num).generic->property("generic"))
        std::cout << "Sector is generic" << std::endl;
    if (sV.at(num).generic->property("rectangle"))
        std::cout << "Sector is rectangle" << std::endl;
    if (sV.at(num).generic->property("inner"))
        std::cout << "Sector is inner" << std::endl;
    if (sV.at(num).generic->property("loop"))
        std::cout << "Sector is loop" << std::endl;

    std::vector<int> sprites=findAllSprites(sV.begin()+num);
    std::cout << "sprites: " << sprites.size() << std::endl;
    for(auto I: sprites) std::cout << I << " "; std::cout<<std::endl;

    glm::vec3 orig=getRndPosition(sV.begin()+num);
    std::cout << "Origin x: " << orig.x << " Origin y: " << orig.y << std::endl;

    SC << "wall amount: "<< sV.at(num).wall_count() << SE;
    //UNIT el(wV, sV, spV);
    //el.is_set(spV.begin());
    //std::cout << "type: " << typeid(*spV.begin()).name() << std::endl;

    //set_it(a, wV.begin()+10);

    //std::cout << "name: " << a.name << std::endl;
    //void* ptr=get_it(a);
    //td::vector<unionWall>::iterator ptr1=reinterpret_cast<std::vector<unionWall>::iterator&>(ptr);


/*
    std::vector<unionWall>::iterator& ptr=dynamic_cast<std::vector<unionWall>::iterator&>(a);
    ptr=wV.begin()+10;

    std::vector<unionWall>::iterator& ptr1=dynamic_cast<std::vector<unionWall>::iterator&>(a);
    void* ptr2=dynamic_cast<std::vector<unionWall>::iterator*>(&a);
    ptr2=dynamic_cast<std::vector<unionSector>::iterator*>(&a);
    std::vector<unionSector>::iterator& ptr3=reinterpret_cast<std::vector<unionSector>::iterator&>(ptr2);
*/

//    std::vector<unionWall>::iterator& x0=get_it(a);

    //std::cout << "ptr: " << ptr1-wV.begin() << std::endl;
   finish();

};

void Sprite::print() {
	std::cout  << "-----SPRITE----------------------" << std::endl;
    SHOW(x); SHOW(y); SHOW(z);
    SHOW(cstat);
	//std::cout << "picnum: " << picnum << " " << sprite_name << std::endl;
	SHOW(picnum);
    SHOW(shade);
    SHOW(pal); SHOW(clipdist); SHOW(filler);
    SHOW(xrepeat); SHOW(yrepeat);
    SHOW(xoffset); SHOW(yoffset);
    SHOW(sectnum); SHOW(statnum);
    SHOW(ang); SHOW(owner); SHOW(xvel); SHOW(yvel); SHOW(zvel);
    SHOW(lotag); SHOW(hitag); SHOW(extra);
};

void Sector::print() {
	std::cout  << "-----SECTOR----------------------" << std::endl;
    SHOW(wallptr); SHOW(wallnum);
    SHOW(ceilingz); SHOW(floorz);
    SHOW(ceilingstat); SHOW(floorstat);
    SHOW(ceilingpicnum); SHOW(ceilingheinum);
    SHOW(ceilingshade);
    SHOW(ceilingpal); SHOW(ceilingxpanning); SHOW(ceilingypanning);
    SHOW(floorpicnum); SHOW(floorheinum);
    SHOW(floorshade);
    SHOW(floorpal); SHOW(floorxpanning); SHOW(floorypanning);
    SHOW(visibility); SHOW(filler); // Filler "should" == 0
    SHOW(lotag); SHOW(hitag); SHOW(extra);
};

void Wall::print() {
	std::cout  << "-----WALL----------------------" << std::endl;
    SHOW(x); SHOW(y);
    SHOW(point2); SHOW(nextwall); SHOW(nextsector); SHOW(cstat);
    SHOW(picnum); SHOW(overpicnum);
    SHOW(shade);
    SHOW(pal); SHOW(xrepeat); SHOW(yrepeat); SHOW(xpanning); SHOW(ypanning);
    SHOW(lotag); SHOW(hitag); SHOW(extra);
};

void unionSector::print()
{
    Sector::print();
    if (!over)
        return;

	SHOW(refer);
    SHOW(state);
    // trigger data
    SHOW(busy);
    SHOW(data);
    SHOW(txID);
    SHOW(rxID);
    SHOW(command);
    SHOW(triggerOn);
    SHOW(triggerOff);
    SHOW(busyTime);
    SHOW(waitTime);
    SHOW(restState);
    SHOW(interruptable);
    // lighting data
    SHOW( amplitude);
    SHOW(freq);
    SHOW(phase);
    SHOW(wave);
    SHOW(shadeAlways);
    SHOW(shadeFloor);
    SHOW(shadeCeiling);
    SHOW(shadeWalls);
    SHOW(shade);

    // panning data
    SHOW(panAlways);
    SHOW(panFloor);
    SHOW(panCeiling);
    SHOW(drag);

    // wind/water stuff
    SHOW(underwater);
    SHOW(depth);
    SHOW(panVel);
    SHOW(panAngle);
    SHOW(wind);

    // physical triggers
    SHOW(decoupled);
    SHOW(triggerOnce);
    SHOW(isTriggered);
    SHOW(key);
    SHOW(triggerPush);
    SHOW(triggerImpact);
    SHOW(triggerReserved0);
    SHOW(triggerEnter);
    SHOW(triggerExit);
    SHOW(triggerWPush);
    SHOW(triggerReserved1);
    SHOW(triggerReserved2);

    // movement data
    SHOW(offCeilZ);
    SHOW(onCeilZ);
    SHOW(offFloorZ);
    SHOW(onFloorZ);
    SHOW(marker0); // could be 13 bits
    SHOW(marker1); // could be 13 bits
    SHOW(crush);

    SHOW(ceilxpanFrac);
    SHOW(ceilypanFrac);
    SHOW(floorxpanFrac);
    SHOW(floorypanFrac);

    SHOW(locked);
    SHOW(pad);

    SHOW(upperLink);
    SHOW( upperLinkZ);
    SHOW(lowerLink);
    SHOW( lowerLinkZ);
};


void unionSprite::print()
{
    Sprite::print();
    if (!over)
        return;

	std::cout  << "-----xSPRITE----------------------" << std::endl;
	std::map<int, std::string> respawn_str={{0, "Option"}, {1,"Never"}, {2, "Always"} , {3, "Perman"}};
	std::map<int, std::string>  wave_str={{0, "Sine"},{1,"Linear"}, {2,"SlowOff"}, {3,"SlowOn"}};
	std::map<bool, std::string> toggle={{true, "ON"}, {false, "OFF"}};
	std::map<int, std::string> cmd={{0, "OFF"}, {1,"ON"}, {2,"State"}, {3,"Toggle"},{4,"!State"},
		{5,"Link"},{6,"Lock"},{7,"Unlock"},{8,"Toggle Lock"},{9,"Stop OFF"}, {10,"Stop ON"}, {11, "Stop NEXT"}};
	std::cout << "Number: " << refer << std::endl;
	std::cout << "busy: " << busy << std::endl;
	std::cout << "RX: " << rxID << std::endl;
	std::cout << "TX: " << txID << std::endl;
	std::cout << "State: " << toggle[state] << std::endl;

	if (command < 64)
		std::cout << "Cmd: " << command << " " << cmd[command] << std::endl;
	else
		std::cout << "Cmd: " << command << " " << (command -64) << std::endl;

    std::cout << "==========Send when==========" << std::endl;
	std::cout << "going ON: " << toggle[triggerOn] << std::endl;
	std::cout << "going OFF: " << toggle[triggerOff] << std::endl;
	std::cout << "busyTime: " << busyTime << std::endl;
	std::cout << "waitTime: " << waitTime << std::endl;
	std::cout << "restState: " << toggle[restState] << std::endl;
	std::cout << "==========Trigger On==========" << std::endl;
	std::cout << "Push: " << toggle[triggerPush] << std::endl;
	std::cout << "Vector: " << toggle[triggerVector] << std::endl;
	std::cout << "Impact: " << toggle[triggerImpact] << std::endl;
	std::cout << "Pickup: " << toggle[triggerPickup] << std::endl;
	std::cout << "Touch: " << toggle[triggerTouch] << std::endl;
	std::cout << "Signt: " << toggle[triggerSight] << std::endl;
	std::cout << "Proximity: " << toggle[triggerProximity] << std::endl;
	std::cout << "DudeLockout: " << toggle[triggerDudeLockout] << std::endl;
	std::cout << "==========Launch==========" << std::endl;
	std::cout << "1: " << toggle[!launch1] << std::endl;
	std::cout << "2: " << toggle[!launch2] << std::endl;
	std::cout << "3: " << toggle[!launch3] << std::endl;
	std::cout << "4: " << toggle[!launch4] << std::endl;
	std::cout << "5: " << toggle[!launch5] << std::endl;
	std::cout << "S: " << toggle[!launchS] << std::endl;
	std::cout << "B: " << toggle[!launchB] << std::endl;
	std::cout << "C: " << toggle[!launchC] << std::endl;
	std::cout << "T: " << toggle[!launchT] << std::endl;
	std::cout << "=========Trigger Flags===========" << std::endl;
	std::cout << "Decoupled: " << toggle[decoupled] << std::endl;
	std::cout << "1-shot: " << toggle[triggerOnce] << std::endl;
	std::cout << "Locked: " << toggle[isLocked] << std::endl;
	std::cout << "Interruptable: " << toggle[interruptable] << std::endl;
	std::cout << "Data1: " << data1 << std::endl;
	std::cout << "Data2: " << data2 << std::endl;
	std::cout << "Data3: " << data3 << std::endl;
	std::cout << "Data4: " << data4 << std::endl;
    std::cout << "Key: " << key << std::endl;
	std::cout << "wave: " << wave << " " << wave_str[wave] << std::endl;
	std::cout << "Respawn: " << std::endl;
	std::cout << "When: " << respawn << ": " << respawn_str[respawn]  <<  std::endl;
	std::cout << "Dude Flags: " << std::endl;
	std::cout << "dudeDeaf: " << toggle[dudeDeaf] << std::endl;
	std::cout << "dudeAmbush: " << toggle[dudeAmbush] << std::endl;
	std::cout << "dudeGuard: " << toggle[dudeGuard] << std::endl;
	std::cout << "reserved: " << toggle[dudeFlag4] << std::endl;
	std::cout << "Lock msg: " << lockMsg << std::endl;
	std::cout << "Drop item: " << dropId << std::endl;
	SHOW(busy);
    SHOW(target);
    SHOW(targetX);
    SHOW(targetY);
    SHOW(targetZ);
    SHOW(burnTime);
    SHOW(burnSource);
    SHOW(unused2);
    SHOW(stateTimer);
    SHOW(aiState);
};


void unionWall::print()
{
    Wall::print();

    if (!over)
        return;

	std::map<bool, std::string> toggle={{true, "ON"}, {false, "OFF"}};
	std::map<int, std::string> cmd={{0, "OFF"}, {1,"ON"}, {2,"State"}, {3,"Toggle"},{4,"!State"},
		{5,"Link"},{6,"Lock"},{7,"Unlock"},{8,"Toggle Lock"},{9,"Stop OFF"}, {10,"Stop ON"}, {11, "Stop NEXT"}};

//	std::cout  << "-----xWall----------------------" << std::endl;

	std::cout << "Wall number: " << refer << std::endl;
	std::cout << "TX: " << txID << std::endl;
	std::cout << "RX: " << rxID << std::endl;
	std::cout << "State: " << toggle[state] << std::endl;
	if (command < 64)
		std::cout << "Cmd: " << command << " " << cmd[command] << std::endl;
	else
		std::cout << "Cmd: " << command << " " << (command -64) << std::endl;
	std::cout << "Send when:" << std::endl;
	std::cout << "going ON: " << toggle[triggerOn] << std::endl;
	std::cout << "going OFF: " << toggle[triggerOff] << std::endl;
	std::cout << "busyTime: " << busyTime << std::endl;
	std::cout << "waitTime: " << waitTime << std::endl;
	std::cout << "restState: " << toggle[restState] << std::endl;

	std::cout << "Trigger On:" << std::endl;
	std::cout << "Push: " << toggle[triggerPush] << std::endl;
	std::cout << "Vector: " << toggle[triggerImpact] << std::endl;
	std::cout << "Reserved: " << toggle[triggerReserved0] << std::endl;
	std::cout << "DudeLockout: " << toggle[dudeLockout] << std::endl;
	std::cout << "Trigger Flags: " << std::endl;
	std::cout << "Decoupled: " << toggle[decoupled] << std::endl;
	std::cout << "1-shot: " << toggle[triggerOnce] << std::endl;
	std::cout << "Locked: " << toggle[locked] << std::endl;
	std::cout << "Interruptable: " << toggle[interruptable] << std::endl;

	std::cout << std::endl;
	std::cout << "Data: " << data << std::endl;
	std::cout << "Key: " << key << std::endl;
	std::cout << "panX: " << panXVel << std::endl;
	std::cout << "panY: " << panYVel << std::endl;
	std::cout << "panAlways: " << toggle[panAlways] << std::endl;

    SHOW(busy);
    SHOW(triggerReserved1);
    SHOW(triggerReserved2);
    SHOW(xpanFrac);
    SHOW(ypanFrac);
    SHOW(pad);
};

int soundTable::open(std::string source_file, std::string target_file, std::string textures_file) {
	for (int i=0; i<3; i++) {
		std::string filename;
		if (i == 0 )
			filename = source_file;
		else if (i == 1)
			filename = target_file;
		else filename=textures_file;

///		COUNT OF LINES ////////////////////

		std::ifstream f(filename, std::ifstream::binary);
		if (f.is_open()) {
		f.seekg(0,f.end);
		int l=f.tellg();
		f.seekg(0,f.beg);
		int lines=0;
		char * buf = new char;
		bool if_space=false;

		for (int i=0;i<l-1; i++) {
			f.read(buf,1);
			if ( *buf == '\n' || *buf == ' ' || *buf == '\t' )
				if_space=true;
			else if (if_space) {
				if_space=false;
				lines++;
			};
		};

		delete buf;
		f.close();
		lines+=10; // overflow protect

		std::ifstream in(filename);
		std::vector<std::string> w(lines);
		int num=read_string(w,in);
		w.resize(num);
		in.close();

		auto it=w.begin();
		while(it != w.end()) {
			if ( *it == "define" ) {
				++it;
				if (i == 1) {
					std::string str=*it;
					std::string str1=*(++it);
					int d=atoi(str1.c_str());
					target[str]=d;
				} else {
					std::string str=*it;
					std::string str1=*(++it);
					int d=atoi(str1.c_str());
					if (i == 0)
						source[d]=str;
					else
						texture[d]=str;
				};
			};
			++it;
		};
		w.erase(w.begin(), w.end());
    } else
    {
        std::cerr << "ERROR: can't open file: " << filename << std::endl;
        return -1;
    };
	};
	return 0;
};

int Map::openPicsTable(std::string filename, std::map<int, glm::ivec2> &table){
///		COUNT OF LINES ////////////////////

		std::ifstream f(filename, std::ifstream::binary);
		if (f.is_open())
		{
            f.seekg(0,f.end);
            int l=f.tellg();
            f.seekg(0,f.beg);
            int lines=0;
            char * buf = new char;
            bool if_space=false;

            for (int i=0;i<l-1; i++) {
                f.read(buf,1);
                if ( *buf == '\n' || *buf == ' ' || *buf == '\t' ) if_space=true;
                else if (if_space) {
                    if_space=false;
                    lines++;
                };
            };

            delete buf;
            f.close();
            lines+=10; // overflow protect

            std::ifstream in(filename);
            std::vector<std::string> w(lines);
            int num=read_string(w,in);
            w.resize(num);
            in.close();

            auto it=w.begin();
            while(it != w.end())
            {
                int d0, d1, d2;
                std::string str;
                if ( *it == "define" && (it+3)<w.end())
                {
                    str=*(++it); d0=atoi(str.c_str());
                    str=*(++it); d1=atoi(str.c_str());
                    str=*(++it); d2=atoi(str.c_str());
                    glm::ivec2 v=glm::ivec2(d1,d2);
                    table[d0]=v;
                };
                ++it;
            };
            w.erase(w.begin(), w.end());
        } else
        {
            std::cerr << "ERROR: can't open file: " << filename << std::endl;
            return -1;
        };
    return 0;
};

void Map::show()
{
    for (auto T: sV)
        T.print();

    for (auto T: wV)
        T.print();

    for (auto T: spV)
        T.print();

};

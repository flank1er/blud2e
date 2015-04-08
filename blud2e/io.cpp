/* Convertor MAP files by Blood video game, Monolith 1997(c) to eduke32 map format
   Project: Blood CrossMatching
   Author: flanker
   It's fork by blud2b.c, HTTP://blood.sourceforge.net with additional features
   License: GPL v.3
*/
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <map>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>

#include "blud2e.h"

#include <glm/gtc/matrix_transform.hpp>
#include <sys/stat.h> // check that file exists
#include <zlib.h>

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
    unsigned short startAngle;
	short sectorNum;
	short unknownElts;
};

struct thirdHeader {
	int mapRevisions;
	short numSectors;
	short numWalls;
	short numSprites;
};

struct signature {
    unsigned int label=0x1A4D4C42;
    unsigned short ver=0x0700;
};


std::string textFileRead (const char * filename)
{
    std::string str, ret = "" ;
    std::ifstream in;
    in.open(filename) ;
    if (in.is_open())
    {
        getline (in, str) ;
        while (in)
        {
            ret += str + "\n" ;
            getline (in, str) ;
        }
        //    cout << "Shader below\n" << ret << "\n" ;
        in.close();
        return ret ;
    }
    else
    {
        //qDebug() << "Unable to Open File " << filename ;
        return "";
    }
}

bool fileExists(const char* filename)
{
    struct stat buf;
    if (stat(filename, &buf) != -1)
    {
        return true;
    }
    return false;
};

int DecryptBuffer (unsigned char* Buffer, const size_t DataSize, unsigned char DecryptKey)
{
    if (!isEncrypted || Buffer == NULL)
        return EXIT_FAILURE;

	for (unsigned short i = 0; i < DataSize; i++)
        	Buffer[i] ^= (unsigned char)(DecryptKey + i);
    return EXIT_SUCCESS;
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
void blud2e::showInfo(std::stringstream& ret) {
    if (!isEmpty())
    {
    ret << "===== MAP INFO ============\n";
    ret << "Initial position: \n";
    ret <<"startX: " << dh.X  << "\n";
    ret << "startY: " << dh.Y  << "\n";
    ret << "startZ: " << dh.Z  << "\n";
    ret << "startAngle: " << dh.angle  << std::endl
         << "sectorNum: " << dh.sector  << std::endl;
        if (isEncrypted)
        {
            ret << std::endl << "Encrypted: yes" << std::endl;
            ret << "Map revision: " << Revision  << std::endl;
        } else
            ret << std::endl << "Encrypted: no" << std::endl;

    ret << "map version: " << dh.version << std::endl;
    ret << "amount Sectors: " << sV.size()  << std::endl;
    ret << "amount Walls: " << wV.size()  << std::endl;
    ret << "amount Sprites: " << spV.size()  << std::endl;
    } else {
        ret << " ERROR: view info about map impossible, because  map is empty!" <<std::endl;

    }
};

////////////////////// W R I T E ////////////////////////////////////
int  blud2e::write(char *filename, std::stringstream& referback) {

    std::ofstream out(filename, std::ofstream::binary);
    if (out.is_open())
    {
        if (sizeof(Sector) != size_sector || sizeof(Sprite)  != size_sprite || sizeof(Wall) != size_wall || sizeof(dh) != 20)
        {
            referback << "ERROR: Incorrect size of types" << std::endl;
            out.close();
            return EXIT_FAILURE;
        }

        out.write((char*)(&dh), sizeof(dh));

        Sector newSec; writeVector(sV, newSec, out);
        Wall newWall; writeVector(wV, newWall, out);
        Sprite newSpr; writeVector(spV, newSpr, out);

        out.close();
     } else {
        referback << "ERROR: can't open file " << *filename << " for write!" <<std::endl;
        return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
};

////////////////// R E A D /////////////////////////////////////////////
int blud2e::read(char *filename, std::stringstream& msg)
{
    std::ifstream in (filename, std::ifstream::binary);
	if (in.is_open())
	{
		unsigned short amountSectors, amountWalls, amountSprites;
		bool blood_format=true;
        // calculate lenght of file
        in.seekg (0, in.end);
		lengthMap = in.tellg();
		in.seekg (0, in.beg);

        if (lengthMap <26) {
            msg << "ERROR: lenght of map is too small!" << std::endl;
            return EXIT_FAILURE;
        }

        // get work buffer
        char Buffer[180];
		char * ptr= (char*)Buffer;
		unsigned char sign[6];

		in.read((char*)&sign, sizeof(sign));

		// if Blood map format
		if ( (sign[0] ^ sign[1] ^ sign[2] ^ sign[3]) == 0x59 )
            {
                msg << "Blood MAP format detection..." << std::endl;
                if (sign[4] == 3 && sign[5] == 6 )
                    dh.version=63;
                else if (sign[4] == 0 && sign[5] == 7)
                {
                    dh.version=7;
                    isEncrypted=true;
                } else
                {
                    msg << "\nDecryptor can only handle map versions 6.3 and 7.0.\n";
                    msg << "Try bringing the map up in the latest version of mapedit, then save it.\n";
                    msg << "This should produce a 7.0 version map, which Decryptor CAN convert to build.\n";
                    in.close();
                    return EXIT_FAILURE;
                };

                // read magic+version
                in.read(ptr,size_firstHeader);

                DecryptBuffer ((unsigned char*)ptr, size_firstHeader, 0x4D);
                firstHeader &hd1=*(firstHeader*)Buffer;

                dh.X=hd1.startX;  dh.Y=hd1.startY;  dh.Z=hd1.startZ;
                dh.angle=hd1.startAngle;  dh.sector=hd1.sectorNum;
                unsigned int NbUnknownElements = (1 << hd1.unknownElts);
                elm=hd1.unknownElts;

                msg << "count unknownElts: " << NbUnknownElements << std::endl;

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
                // go to begin the first sector
                in.seekg(offset, in.cur);
           } else
           {
                in.seekg(0, in.beg);
                in.read((char*)&dh, sizeof(dh));
                if ( dh.version < 6 && dh.version > 10 )
                {
                     msg << "ERROR: Incorrect format file!\n";
                     in.close();
                     return EXIT_FAILURE;
                };
                msg << "Duke MAP format detection..." <<std::endl;
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
            showInfo(msg); //???? re
        } else{
            msg << "ERROR: can't open file: " << filename << std::endl;
            return EXIT_FAILURE;
        };

	in.close();
    return EXIT_SUCCESS;
};

int blud2e::write_v7B(char* filename, std::stringstream& msg)
{
    std::ofstream out (filename, std::ofstream::binary);
    if (out.is_open())
    {        
        Revision++;
        unsigned char* buffer = new unsigned char[180];
        uLong crc = crc32(0L, Z_NULL, 0);

        //////////SIGNATURE////////////////////
        signature sg;
        buffer=(unsigned char*)&sg;
        out.write((char*)buffer, size_signature);
        crc=crc32(crc, buffer, size_signature);
        /////////MAGIC NUMBER ///////////////////////
        firstHeader head_first;
        head_first.startX=dh.X;
        head_first.startY=dh.Y;
        head_first.startZ=dh.Z;
        head_first.startAngle=dh.angle;
        head_first.startAngle=dh.sector;
        head_first.unknownElts=elm;
        buffer=(unsigned char*)&head_first;
        DecryptBuffer ((unsigned char*)buffer, size_firstHeader, 0x4D);
        out.write((char*)buffer,size_firstHeader);
        crc=crc32(crc, buffer, size_firstHeader);

        buffer=(unsigned char*)&sh;
        out.write((char*)buffer,size_secondHeader);
        crc=crc32(crc, buffer, size_secondHeader);

        ///// AMOUNT OF SECTORS / WALLS / SPRITES / REVISION NUMBER /////third header //////////
        thirdHeader hd3;
        hd3.numSectors=sV.size();
        hd3.numWalls=wV.size();
        hd3.numSprites=spV.size();
        hd3.mapRevisions=Revision;
        buffer=(unsigned char*)&hd3;
        DecryptBuffer ((unsigned char*)buffer, size_thirdHeader, 0x68);
        out.write((char*)buffer,size_thirdHeader);
        crc=crc32(crc, buffer, size_thirdHeader);

        int offset=size_extra+(2<<elm);
        buffer=(unsigned char*)&fh;
        out.write((char*)buffer, offset);
        crc=crc32(crc, buffer, offset);
        //std::cout << "offset: " << offset <<std::endl;

        /// SECTORS ///////////////
        unsigned char  decryptKey = ((Revision * size_sector) & 0xFF);
        for (auto T: sV)
        {
            Sector chunk;
            chunk=dynamic_cast<Sector&>(T);
            buffer=(unsigned char*)&chunk;
            DecryptBuffer ((unsigned char*)buffer, size_sector, decryptKey);
            out.write((char*)buffer, size_sector);
            crc=crc32(crc, buffer, size_sector);
            if (T.extra > 0)
            {
                xSector chunkX;
                chunkX=dynamic_cast<xSector&>(T);
                buffer=(unsigned char*)&chunkX;
                out.write((char*)buffer, size_extra_sector);
                crc=crc32(crc, buffer, size_extra_sector);
            }
        };

        //// WALLS /////////////////
        decryptKey = (((Revision * size_sector) | 0x4d) & 0xFF);
        for (auto T: wV)
        {
            Wall chunk;
            chunk=dynamic_cast<Wall&>(T);
            buffer=(unsigned char*)&chunk;
            DecryptBuffer ((unsigned char*)buffer, size_wall, decryptKey);
            out.write((char*)buffer, size_wall);
            crc=crc32(crc, buffer, size_wall);
            if (T.extra > 0)
            {
                xWall chunkX;
                chunkX=dynamic_cast<xWall&>(T);
                buffer=(unsigned char*)&chunkX;
                out.write((char*)buffer, size_extra_wall);
                crc=crc32(crc, buffer, size_extra_wall);
            }
        };

        //// SPRITES /////////////////////////
        decryptKey = (((Revision * size_sprite) | 0x4d) & 0xFF);
        for (auto T: spV)
        {
            Sprite chunk;
            chunk=dynamic_cast<Sprite&>(T);
            buffer=(unsigned char*)&chunk;
            DecryptBuffer ((unsigned char*)buffer, size_sprite, decryptKey);
            out.write((char*)buffer, size_sprite);
            crc=crc32(crc, buffer, size_sprite);
            if (T.extra > 0)
            {
                xSprite chunkX;
                chunkX=dynamic_cast<xSprite&>(T);
                buffer=(unsigned char*)&chunkX;
                out.write((char*)buffer, size_extra_sprite);
                crc=crc32(crc, buffer, size_extra_sprite);
            }
        };

        out.write((char*)&crc,4);
    } else
    {
        msg <<  "ERROR: can't  write to file: " << filename << std::endl;
        return EXIT_FAILURE;
    }
    out.close();
    return EXIT_SUCCESS;
}

///////////////////  S H O W ///////////////////////////////////////////
void blud2e::printSector(int  num, bool blood, std::stringstream& msg)
{
    /// prepare
    std::map<int, std::string> secType = {{600, "Z Motion"}, {0, "Normal"}};
	std::map<int, std::string> wallType = {{ 0, "Normal"} ,{20, "Toggle switch"}, {21, "1-way switch"},
		{500, "Wall Link"}, {501, "Wall stack"}, {511, "Gib Wall"}};
	std::map<int, std::string> spriteType = {{0, "Decoration"}, {1, "Player start" }};
	// check range of the nummber
    if (num >= getSectors())
	{
        msg << "ERROR: number of sector greater than you number!" << std::endl;
		return;
    };

    msg << "----------------------------------" << std::endl;
    msg << "Sector# " << num << std::endl;

    // MAIN BODY ///////
    sV.at(num).print(msg);

    for(auto it=wV.begin()+sV.at(num).wallptr; it < wV.begin()+sV.at(num).wallptr+sV.at(num).wallnum; it++)
    {
        it->print(msg);
    };

    for( auto T: spV)
    {
        if ( T.sectnum == num)  T.print(msg);
    };
};

void Sprite::print(std::stringstream& msg) {
    std::stringstream ss;
    std::streambuf *coutbuf=std::cout.rdbuf(); // redirection output of std::cout to file
    std::cout.rdbuf(ss.rdbuf());

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
    std::cout.rdbuf(coutbuf);
    msg <<ss.str();
};

void Sector::print(std::stringstream& msg) {
    std::stringstream ss;
    std::streambuf *coutbuf=std::cout.rdbuf(); // redirection output of std::cout to file
    std::cout.rdbuf(ss.rdbuf());
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
    std::cout.rdbuf(coutbuf);
    msg <<ss.str();
};

void Wall::print(std::stringstream& msg) {
    std::stringstream ss;
    std::streambuf *coutbuf=std::cout.rdbuf(); // redirection output of std::cout to file
    std::cout.rdbuf(ss.rdbuf());
	std::cout  << "-----WALL----------------------" << std::endl;
    SHOW(x); SHOW(y);
    SHOW(point2); SHOW(nextwall); SHOW(nextsector); SHOW(cstat);
    SHOW(picnum); SHOW(overpicnum);
    SHOW(shade);
    SHOW(pal); SHOW(xrepeat); SHOW(yrepeat); SHOW(xpanning); SHOW(ypanning);
    SHOW(lotag); SHOW(hitag); SHOW(extra);
    std::cout.rdbuf(coutbuf);
    msg <<ss.str();
};

void unionSector::print(std::stringstream& msg)
{
    Sector::print(msg);
    if (!over)
        return;

    std::stringstream ss;
    std::streambuf *coutbuf=std::cout.rdbuf(); // redirection output of std::cout to file
    std::cout.rdbuf(ss.rdbuf());
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
    std::cout.rdbuf(coutbuf);
    msg <<ss.str();
};


void unionSprite::print(std::stringstream& msg)
{
    Sprite::print(msg);
    if (!over)
        return;
    std::stringstream ss;
    std::streambuf *coutbuf=std::cout.rdbuf(); // redirection output of std::cout to file
    std::cout.rdbuf(ss.rdbuf());
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
    std::cout.rdbuf(coutbuf);
    msg <<ss.str();
};


void unionWall::print(std::stringstream& msg)
{
    Wall::print(msg);
    if (!over)
        return;

    std::stringstream ss;
    std::streambuf *coutbuf=std::cout.rdbuf(); // redirection output of std::cout to file
    std::cout.rdbuf(ss.rdbuf());


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
    std::cout.rdbuf(coutbuf);
    msg <<ss.str();
};

int soundTable::open(std::string source_file, std::string target_file, std::string textures_file) {
    source.erase(source.begin(), source.end());
    target.erase(target.begin(), target.end());
    texture.erase(texture.begin(), texture.end());

    for (int i=0; i<3; i++)
    {
		std::string filename;
		if (i == 0 )
			filename = source_file;
		else if (i == 1)
			filename = target_file;
		else filename=textures_file;

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
            while(it != w.end())
            {
                if ( *it == "define" )
                {
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
        } else {
            //std::cerr << "ERROR: can't open file: " << filename << std::endl;
            return EXIT_FAILURE;
        };
	};
    return EXIT_SUCCESS;
};

int blud2e::openPicsTable(std::string filename, std::map<int, glm::ivec2> &table)
{
    table.erase(table.begin(), table.end());
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
        //std::cerr << "ERROR: can't open file: " << filename << std::endl;
        return EXIT_FAILURE;
    };
    return EXIT_SUCCESS;
};

void blud2e::show(std::stringstream& msg)
{
    for (auto T: sV)
        T.print(msg);

    for (auto T: wV)
        T.print(msg);

    for (auto T: spV)
        T.print(msg);

};

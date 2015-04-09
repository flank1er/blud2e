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
const int size_wall=32;
const int size_sprite=44;

const int size_extra_sector=60;
const int size_extra_sprite=56;
const int size_extra_wall=24;

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

int get_lines_in_text_file(std::string filename);
int read_string(std::vector<std::string> &words, std::ifstream& in);

bool fileExists(const char* filename)
{
    struct stat buf;
    if (stat(filename, &buf) != -1)  { return true;} return false;
};

template<class myFunction>
myFunction my_func(const char* filename, std::stringstream& msg,  myFunction fn)
{
     int lines=get_lines_in_text_file(filename);
     if (lines > 0)
     {
         lines+=10;
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
                 std::string str=*it;
                 std::string str1=*(++it);
                 int d=atoi(str1.c_str());
                 fn(d, str);
             };
             ++it;
         };
     } else {
         msg <<"ERROR: couldn't open file: " << filename << std::endl;
     }
     return fn;
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


int blud2e::DecryptBuffer (unsigned char* Buffer, const size_t DataSize, unsigned char DecryptKey)
{
    if (!isEncrypted || Buffer == NULL)
        return EXIT_FAILURE;

	for (unsigned short i = 0; i < DataSize; i++)
        	Buffer[i] ^= (unsigned char)(DecryptKey + i);
    return EXIT_SUCCESS;
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

int get_lines_in_text_file(std::string filename)
{
    std::ifstream f(filename, std::ifstream::binary);
    if (f.is_open())
    {
        f.seekg(0,f.end); int l=f.tellg(); f.seekg(0,f.beg);
        int lines=0; char* buf = new char;  bool if_space=false;

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
        return lines+10; // overflow protect
    } else
        return -1;
}

int blud2e::read_text_file_to_string(const char* filename, std::string& ret, std::stringstream& msg)
{
    std::ifstream f(filename);
    if (f.is_open())
    {
        f.seekg(0, std::ios::end);
        ret.reserve(f.tellg());
        f.seekg(0, std::ios::beg);

        ret.assign((std::istreambuf_iterator<char>(f)),
                std::istreambuf_iterator<char>());
        f.close();
    } else {
        msg << "ERROR: can't open file: " << filename << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


///////----- M A P    C L A S S ////////////////////////////

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
    //sTable.load_tables();

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

int Resources::load_tables(std::stringstream& referback)
{
    source.erase(source.begin(), source.end());
    target.erase(target.begin(), target.end());
    texture.erase(texture.begin(), texture.end());

    if (!fileExists(tex_con_file) || !fileExists(sound_con_file) || !fileExists(original_sound) || !fileExists(pic_file))
    {
        referback << "ERROR: missing files: sounds.con or sounds_old.con or defs.con or pic_table.con" << std::endl;
        return EXIT_FAILURE;
    };

    my_func(original_sound, referback, [&] (int _n, std::string _s) {
        source[_n]=_s;
    });

    my_func(sound_con_file, referback,  [&] (int _n, std::string _s) {
        target[_s]=_n;
    });

    my_func(pic_file, referback, [&] (int _n, std::string _s) {
        texture[_n]=_s;
    });

    open_pics_resolution_table(referback);

    return EXIT_SUCCESS;
};

int Resources::open_pics_resolution_table(std::stringstream& msg)
{

    pics_table.erase(pics_table.begin(), pics_table.end());

    int lines=get_lines_in_text_file(pic_file);
    if (lines > 0)
    {
        std::ifstream in(pic_file);
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
                pics_table[d0]=v;
            };
            ++it;
        };
        w.erase(w.begin(), w.end());
    } else {
        //std::cerr << "ERROR: can't open file: " << filename << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
};


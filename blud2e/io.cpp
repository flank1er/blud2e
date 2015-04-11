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

const int size_sector=40;
const int size_wall=32;
const int size_sprite=44;

const int size_extra_sector=60;
const int size_extra_sprite=56;
const int size_extra_wall=24;

bool isEncrypted=true;
const int BUFFER_SIZE=192;

int get_lines_in_text_file(std::string filename);
int read_string(std::vector<std::string> &words, std::ifstream& in);

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

int DecryptBuffer (unsigned char* Buffer, const size_t DataSize, unsigned char DecryptKey, int offset=0)
{
    if (!isEncrypted || Buffer == NULL || (offset+DataSize) > BUFFER_SIZE)
        return EXIT_FAILURE;

    for (unsigned short i = offset; i < DataSize+offset; i++)
            Buffer[i] ^= (unsigned char)(DecryptKey + i - offset);
    return EXIT_SUCCESS;
};

template<typename T, typename T1, typename T2>
void readVector(std::vector<T> &the_vector, T1 &basic, T2 &extra,std::ifstream &file,int count, unsigned char key, bool format)
{
    char Buffer[BUFFER_SIZE];
    char * ptr= (char*)Buffer;
    for ( int i=0; i< count; i++)
    {
        T a;
        T1& b=dynamic_cast<T1&>(a);
        file.read(ptr,sizeof(b));
        DecryptBuffer ((unsigned char*)ptr, sizeof(b), key);
        b=*(T1*)Buffer;
        if (format && b.extra >0)
        {
            file.read(ptr, sizeof(extra));
            a.over=true;
            T2& c=dynamic_cast<T2&>(a);
            c=*(T2*)Buffer;
        }
        the_vector.push_back(a);
    };
}
template<typename T, typename T1, typename T2>
void writeVector7B(T &the_vector, T1 &basic, T2 &extra,std::ofstream &file,uLong& crc, unsigned char key)
{
    for(auto it:the_vector)
    {
        T1 chunk=dynamic_cast<T1&>(it);
        DecryptBuffer ((unsigned char*)&chunk, sizeof(chunk), key);
        file.write((char*)&chunk, sizeof(chunk));
        crc=crc32(crc, (const Bytef*)&chunk, sizeof(chunk));
        if (it.extra >0)
        {
            T2 chunkX=dynamic_cast<T2&>(it);
            file.write((char*)&chunkX, sizeof(chunkX));
            crc=crc32(crc, (const Bytef*)&chunkX, sizeof(chunkX));
        }
    }
}

int check_types_size()
{
    return (sizeof(Sector) != size_sector || sizeof(xSector) != size_extra_sector \
            || sizeof(Wall) != size_wall || sizeof(xWall) != size_extra_wall || \
            sizeof(Sprite) != size_sprite || sizeof(xSprite) != size_extra_sprite ||
            sizeof(BloodHeader) != size_blood_header) \
            ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool fileExists(const char* filename)
{
    struct stat buf;
    return (stat(filename, &buf) != -1) ? true : false;
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
    if (check_types_size() == EXIT_FAILURE)
    {
        referback << "ERROR: incorrect of size types!" << std::endl;
        return EXIT_FAILURE;
    }

    std::ofstream out(filename, std::ofstream::binary);
    if (out.is_open())
    {
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
// decrypt algorithm from blud2b.c, HTTP://blood.sourceforge.net
int blud2e::read(char *filename, std::stringstream& msg)
{
    if (check_types_size() == EXIT_FAILURE)
    {
        msg << "ERROR: incorrect of size types!" << std::endl;
        return EXIT_FAILURE;
    }
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

        char Buffer[BUFFER_SIZE];
        char * ptr= (char*)Buffer;

        in.read(ptr,size_blood_header);
        DecryptBuffer ((unsigned char*)ptr, size_firstHeader, 0x4D, size_signature);
        DecryptBuffer ((unsigned char*)ptr, size_thirdHeader, 0x68,size_signature+size_firstHeader+size_secondHeader);
        BH=*(BloodHeader*)ptr;

        if(BH.label == 0x1A4D4C42)
            {
                if (BH.ver == 0x0700)
                {
                    dh.version=7;
                    map_specification=dh.version*(-1);
                    isEncrypted=true;
                } else {
                    msg << "\nDecryptor can only handle map versions 7.0.\n";
                    msg << "Try bringing the map up in the latest version of mapedit, then save it.\n";
                    msg << "This should produce a 7.0 version map, which Decryptor CAN convert to build.\n";
                    in.close();
                    return EXIT_FAILURE;
                }

                dh.X=BH.startX;
                dh.Y=BH.startY;
                dh.Z=BH.startZ;
                dh.angle=BH.startAngle;
                dh.sector=BH.sectorNum;
                uKelm = (1 << BH.unknownElts);
                amountSectors=BH.numSectors;
                amountWalls=BH.numWalls;
                amountSprites=BH.numSprites;
                Revision=BH.mapRevisions;

                msg << "Blood MAP format detection: "<< map_descriptor[map_specification] << std::endl;
                msg << "count unknownElts: " << uKelm << std::endl;

                int offset = (uKelm * size_unknownElts);
                in.read(ptr, offset);
                map_offset=*(OF*)ptr;
                //in.seekg(offset, in.cur);
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
                map_specification=dh.version;
                BH.mapRevisions=0;
                BH.startX=dh.X;
                BH.startY=dh.Y;
                BH.startZ=dh.Z;
                BH.startAngle=dh.angle;
                BH.sectorNum=dh.sector;
                BH.unknownElts=4;
                isEncrypted=false;

                msg << "Duke MAP format detection: " << map_descriptor[map_specification] << std::endl;
                in.read((char*)&amountSectors, sizeof(amountSectors));
                BH.numSectors=amountSectors;
                blood_format=false;
           };

            unsigned char  decryptKey = ((Revision * size_sector) & 0xFF);
            Sector A; xSector B; readVector(sV,A,B,in,amountSectors,decryptKey, blood_format);

            if (!blood_format)
            {
                in.read((char*)&amountWalls,2);
                BH.numWalls=amountWalls;
            };

            decryptKey = (((Revision * size_sector) | 0x4d) & 0xFF);
            Wall C; xWall D; readVector(wV,C,D,in,amountWalls,decryptKey, blood_format);

            if (!blood_format)
            {
                in.read((char*)&amountSprites,2);
                BH.numSprites=amountSprites;
            };

            decryptKey = (((Revision * size_sprite) | 0x4d) & 0xFF);
            Sprite E; xSprite F; readVector(spV,E,F,in,amountSprites,decryptKey, blood_format);

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
    if (check_types_size() == EXIT_FAILURE)
    {
        msg << "ERROR: incorrect of size types!" << std::endl;
        return EXIT_FAILURE;
    }
    if ( map_specification > 0)
    {
        msg << "ERROR: impossible write to Blood format file from: " << map_descriptor[map_specification]<< std::endl;
        return EXIT_FAILURE;
    }

    isEncrypted=true;
    std::ofstream out (filename, std::ofstream::binary);

    if (out.is_open())
    {
        Revision++;
        BH.numSectors=(short)getSectors();
        BH.numWalls=(short)getWalls();
        BH.numSprites=(short)getSprites();
        BH.mapRevisions=Revision;
        msg << "map revision: " << Revision << std::endl;
        unsigned char* buffer = new unsigned char[BUFFER_SIZE];
        uLong crc = crc32(0L, Z_NULL, 0);
        buffer=(unsigned char*)&BH;
        DecryptBuffer ((unsigned char*)buffer, size_firstHeader, 0x4D, size_signature);
        DecryptBuffer ((unsigned char*)buffer, size_thirdHeader, 0x68,size_signature+size_firstHeader+size_secondHeader);
        crc=crc32(crc, buffer, size_blood_header);
        out.write((char*)buffer,size_blood_header);

        int offset = (uKelm * size_unknownElts);
        buffer=(unsigned char*)&map_offset;
        crc=crc32(crc, buffer, offset);
        out.write((char*)buffer,offset);

        unsigned char  decryptKey = ((Revision * size_sector) & 0xFF);
        Sector A; xSector B; writeVector7B(sV,A,B,out,crc,decryptKey);

        decryptKey = (((Revision * size_sector) | 0x4d) & 0xFF);
        Wall C; xWall D; writeVector7B(wV,C,D,out,crc,decryptKey);

        decryptKey = (((Revision * size_sprite) | 0x4d) & 0xFF);
        Sprite E; xSprite F; writeVector7B(spV,E,F,out,crc,decryptKey);

        out.write((char*)&crc,4);
        out.close();
    } else
    {
        msg <<  "ERROR: can't  write to file: " << filename << std::endl;
        return EXIT_FAILURE;
    }
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
        msg << "ERROR: can't open file: " << pic_file << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
};


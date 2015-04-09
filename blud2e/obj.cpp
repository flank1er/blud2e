#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <set>

#include "blud2e.h"
#include <glm/glm.hpp>

class OBJEXP
{
    std::set<int> the_list, the_done_list;
    std::vector<glm::vec3> vertex_array, normal_array;

    int plane, vertex, normal, deep, texcoor;
    float scaleX, scaleY, offsetX, offsetY,resX, resY;
    //glm::vec3 vn;

    void printVertex(glm::vec3& p){
        std::cout << "v " << p.x << " " << p.y << " " << p.z << " 1" <<  std::endl;
        vertex_array.push_back(p);
        vertex++;
    };

    int printTexCoor(
            glm::vec3 x0,
            glm::vec3 x1,
            glm::vec3 x2,
            glm::vec3 x3,
            std::stringstream& msg)
    {
        if (!(resY && resX))
        {
            msg << "ERROR in method: printTexCoor";
            return EXIT_FAILURE;
        }
        float v1=glm::distance(x1.z, x0.z)*scaleY*4/resY;
        float v2=glm::distance(x2.z, x3.z)*scaleY*4/resY;
        float v4=scaleX*8.f/resX;
        std::cout << "vt "<< offsetX<<" "<< offsetY << std::endl << "vt "<< v4+offsetX << " "<< offsetY<< std::endl <<
        "vt "<< v4+offsetX<< " "<< v2+offsetY << std::endl << "vt "<< offsetX<<" " << v1+offsetY<< std::endl;
        texcoor+=4;
        return EXIT_SUCCESS;
    };

    void printNormal(glm::vec3& vn)
    {
        std::cout << "vn " << vn.x << " " << vn.y << " " << vn.z  <<  std::endl;
        normal_array.push_back(vn);
        normal++;
    };

    void makeSet(int count, std::vector<unionWall>& w, std::vector<unionSector>& s)
    {
        std::set<int> temp={};
        for(int i=0; i < count; i++)
        {
            for(auto it=the_list.begin(); it!=the_list.end(); it++)
                for(auto iter=s.at(*it).firstWall; iter<(s.at(*it).firstWall+s.at(*it).wallnum); iter++)
                    if (iter->nextsector >= 0)
                            temp.insert(iter->nextsector);
            for(auto T : temp)
                the_list.insert(T);
            temp.erase(temp.begin(), temp.end());
        };
    };

public:
    OBJEXP(int plane=1, int vertex=1, int deep=1) : plane(plane), vertex(vertex), deep(deep) { normal=1; texcoor=1;};
    ~OBJEXP() {the_list.erase(the_list.begin(), the_list.end());};
    int print(int request,
              std::vector<unionWall>& walls,
              std::vector<unionSector>& sectors,
              //std::map<int, glm::ivec2> &tga_res,
              std::stringstream& msg,
              bool recursively=true);
};

int blud2e::write_obj(int num, char* filename, std::stringstream& msg)
{
    if (num >= getSectors())
	{
        msg << "ERROR: number " << num << " greater than count of sectors!" << std::endl;
        return EXIT_FAILURE;
    }

    if (prepare(msg) == EXIT_FAILURE)
    {
        msg << "ERROR: missing files: sounds.con or sounds_old.con or defs.con or pic_table.con" << std::endl;
        return EXIT_FAILURE;
    }

    OBJEXP obj;
    std::ofstream file(filename);
    if (file.is_open())
    {
        std::streambuf *coutbuf=std::cout.rdbuf(); // redirection output of std::cout to file
        std::cout.rdbuf(file.rdbuf());        

        if (num < 0)
            for(int i=0; i<(int)sV.size(); i++)
                obj.print(i,wV, sV, msg, true);
        else
            obj.print(num, wV, sV, msg);
        finish();
        file.close();
        std::cout.rdbuf(coutbuf);
    } else {
        msg << "ERROR: can't open file " << *filename << " for write!" << std::endl;
        return EXIT_FAILURE;
    };
    return EXIT_SUCCESS;
};

int OBJEXP::print(int request,
        std::vector<unionWall>& walls,
        std::vector<unionSector>& sectors,
        //std::map<int, glm::ivec2> &tga_res,
        std::stringstream& msg,
        bool recursively)
{

    if(the_done_list.count(request))
        return 0;
    the_list.insert(request);
    makeSet(deep, walls,sectors);
    std::cout << "mtllib bcm.mtl" << std::endl;

    for(auto T : the_list)
    {
        std::vector<int> f; // floor and ceiling
        for(auto it=sectors.at(T).firstWall; it<(sectors.at(T).firstWall+sectors.at(T).wallnum); it++)
        {
            glm::vec3 a,b,c, d, n;
            offsetX=(float)it->xpanning/64.f;
            offsetY=(float)it->ypanning/64.f;
            scaleX=(float)it->xrepeat;
            scaleY=(float)it->yrepeat;
            //glm::ivec2 tex=tga_res[it->texture_id];
            glm::ivec2 tex=it->res;
            resY=(float)tex.y; resX=(float)tex.x;

            if (it->nextsector < 0)
            {
                std::cout << "o Plane." << plane << std::endl;
                a=glm::vec3(it->gis.x, it->gis.y, it->gis.z);
                b=glm::vec3(it->gis.x, it->gis.y, it->gis.w);
                c=glm::vec3(it->nextPoint->gis.x, it->nextPoint->gis.y, it->nextPoint->gis.w);
                d=glm::vec3(it->nextPoint->gis.x, it->nextPoint->gis.y, it->nextPoint->gis.z);

                //n=glm::normalize(glm::cross((c-b),(a-b)));
                printVertex(a); //      b---->c     ceilingZ <- gis.w
                printVertex(d);//       ^     |
                printVertex(c);//       |     V
                printVertex(b);//       a<----d     floorZ <- gis.z
                //printNormal(n);

                printTexCoor(a,b,c,d,msg);
                std::cout << "usemtl Material." << it->texture_id << std::endl << "s off" << std::endl;
                std::cout << "f " << vertex-4  << "/"<< texcoor-4 << " " << vertex-3 << "/"<< texcoor-3 << " "
                << vertex-2 << "/"<< texcoor-2 << " " << vertex-1 << "/"<< texcoor-1  << std::endl;
                plane++;
            };

            if (it->nextsector > 0 && sectors.at(T).floor.z < sectors.at(it->nextsector).floor.z)
            {
                std::cout << "o Plane." << plane << std::endl;

                    std::vector<unionWall>::iterator next=it->nextWall->nextPoint;
                a=glm::vec3(next->gis.x, next->gis.y, next->gis.z);
                b=glm::vec3(it->gis.x, it->gis.y, it->gis.z);
                c=glm::vec3(it->nextPoint->gis.x, it->nextPoint->gis.y, it->nextPoint->gis.z);
                    next=it->nextWall;
                d=glm::vec3(next->gis.x, next->gis.y, next->gis.z);

                printVertex(b); printVertex(c); printVertex(d); printVertex(a);
                //n=glm::normalize(glm::cross((a-b),(c-b)));
                //printNormal(n);
                printTexCoor(a,b,c,d,msg);

                std::cout << "usemtl Material."<< it->texture_id << std::endl << "s off" << std::endl;
                std::cout << "f " << vertex-4  << "/"<< texcoor-4 << " " << vertex-3 << "/"<< texcoor-3 << " "
                << vertex-2 << "/"<< texcoor-2 << " " << vertex-1 << "/"<< texcoor-1  << std::endl;
                plane++;
            };

            if (it->nextsector > 0 && sectors.at(T).ceiling.z > sectors.at(it->nextsector).ceiling.z)
            {
                std::cout << "o Plane." << plane << std::endl;

                std::vector<unionWall>::iterator next=it->nextWall->nextPoint;
                b=glm::vec3(next->gis.x, next->gis.y, next->gis.w);
                next=it->nextWall;
                c=glm::vec3(next->gis.x, next->gis.y, next->gis.w);
                d=glm::vec3(it->nextPoint->gis.x, it->nextPoint->gis.y, it->nextPoint->gis.w);
                a=glm::vec3(it->gis.x, it->gis.y, it->gis.w);
                printVertex(b); printVertex(c); printVertex(d); printVertex(a);
                //n=glm::normalize(glm::cross((a-b),(c-b)));
                //printNormal(n);
                printTexCoor(a,b,c,d,msg);

                std::cout << "usemtl Material."<< it->texture_id << std::endl << "s off" << std::endl;
                std::cout << "f " << vertex-4  << "/"<< texcoor-4 << " " << vertex-3 << "/"<< texcoor-3 << " "
                << vertex-2 << "/"<< texcoor-2 << " " << vertex-1 << "/"<< texcoor-1  << std::endl;
                plane++;
            };
       };
//          FLOOR
        std::cout << "o Plane." << plane << std::endl;
        std::cout << "usemtl Material."<< sectors.at(T).floor.texture_id << std::endl << "s off" << std::endl;

        for_loop(sectors.at(T).generic->marker, [&](unionWall _s) {
            glm::vec3 vert=glm::vec3(_s.gis.x, _s.gis.y, _s.gis.z);
            printVertex(vert);
            f.insert(f.begin(), vertex-1);
        });

        std::cout << "f ";  for( auto J :f) std::cout <<" "<< J; std::cout << std::endl;
        f.erase(f.begin(), f.end());
        plane++;
//          CEILING
        std::cout << "o Plane." << plane << std::endl;
        std::cout << "usemtl Material."<< sectors.at(T).ceiling.texture_id << std::endl << "s off" << std::endl;

        for_loop(sectors.at(T).generic->marker, [&](unionWall _s) {
            glm::vec3 vert=glm::vec3(_s.gis.x, _s.gis.y, _s.gis.w);
            printVertex(vert);
            f.push_back(vertex-1);
        });

        std::cout << "f "; for( auto J :f) std::cout <<" "<< J; std::cout << std::endl;
        f.erase(f.begin(), f.end());
        plane++;
    };

    for(auto it=the_list.begin(); it!=the_list.end(); it++)
        the_done_list.insert(*it);
    the_list.erase(the_list.begin(), the_list.end());

    return 0;
};

#include "blud2e.h"
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


void blud2e::show(std::stringstream& msg)
{
    for (auto T: sV)
        T.print(msg);

    for (auto T: wV)
        T.print(msg);

    for (auto T: spV)
        T.print(msg);

};

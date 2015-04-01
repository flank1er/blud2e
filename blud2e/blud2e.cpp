/* Convertor MAP files by Blood video game, Monolith 1997(c) to eduke32 map format
   Project: Blood CrossMatching
   Author: flanker
   It's fork by blud2b.c, HTTP://blood.sourceforge.net with additional features
   License: GPL v.3
*/

#include <iostream>
#include <string>
#include <blud2e.h>

void show_help_message();

int main(int argc, char *argv[]) {
    const char* tex_con_file="defs.con";
    const char* sound_con_file="sounds.con";
    const char* original_sound="sounds_old.con";
    const char* pic_file="pic_table.con";

    Map map; const float version=0.7; int infoSector;
    std::string mode="blank"; char * blood_filemap;  char * duke_filemap;

	std::cerr << "\nDecryptor MAP files from Blood video game, Monolith Production, 1997(c)\n" << \
	"Author: flanker\n" << \
	"Project: Blood Crossmatching\n" << \
	"version: " << version << "-dev" << std::endl << \
	"====================================================\n\n";
    ////////////////////////////////////////////////
    ////  parsing command line arguments
    /// ////////////////////////////////////////////
    if (argc < 3 || argc > 5)
    {
        show_help_message();
        return EXIT_FAILURE;
    }

    if (argc == 3)
    {
        if (!fileExists(argv[2]))
        {
            std::cout << "ERROR: file: " << argv[2] << " doesn't exist!" << std::endl;
            return EXIT_FAILURE;
        };
        blood_filemap = argv[2];
        if ( std::string(argv[1]) == "-i" || std::string(argv[1]) == "--info" )
                mode="info";
        else if ( std::string(argv[1]) == "-e" || std::string(argv[1]) == "--export" )
                mode="export";
        infoSector=-1;
    };

    if (argc == 4)
    {
        if ( std::string(argv[1]) == "-c" || std::string(argv[1]) == "--convert" )
        {
            if (!fileExists(argv[2]))
			{
                std::cout << "ERROR: file: " << argv[2] << " doesn't exist!" << std::endl;
                return EXIT_FAILURE;
            };
            blood_filemap = argv[2]; duke_filemap  = argv[3];mode="convert";

        };
        if ( std::string(argv[1]) == "-i" || std::string(argv[1]) == "--info" ) {
            if (!fileExists(argv[3]))
            {
                std::cout << "ERROR: file: " << argv[3] << " doesn't exist!" << std::endl;
                return EXIT_FAILURE;
            }
            infoSector=atoi(argv[2]); blood_filemap = argv[3]; mode="info";
		};

        if ( std::string(argv[1]) == "-e" || std::string(argv[1]) == "--export" ) {
            if (!fileExists(argv[2]))
			{
                std::cout << "ERROR: file: " << argv[2] << " doesn't exist!" << std::endl;
                return EXIT_FAILURE;
            };
            infoSector=-1; blood_filemap = argv[2];	duke_filemap=argv[3]; mode="export";
        };
    };

    if ((argc == 5) && ( std::string(argv[1]) == "-e" || std::string(argv[1]) == "--export" ))
    {
            if (!fileExists(argv[3]))
			{
                std::cout << "ERROR: file: " << argv[3] << " doesn't exist!" << std::endl;
                return EXIT_FAILURE;
            };
            infoSector=atoi(argv[2]); blood_filemap = argv[3]; duke_filemap=argv[4];
			mode="export";
    };


    if (!fileExists(tex_con_file) || !fileExists(sound_con_file) || !fileExists(original_sound) || !fileExists(pic_file))
    {
		std::cerr << "ERROR: missing files: sounds.con or sounds_old.con or defs.con or pic_table.con" << std::endl;
        return EXIT_FAILURE;
	};

/// C O R E ///////////////////////////////
    if ((map.sTable.open(original_sound ,sound_con_file, tex_con_file) < 0) || (map.openPicsTable(pic_file, map.targa) < 0))
    {
        std::cerr << "ERROR: couldn't open files: sounds.con or sounds_old.con or defs.con or pic_table.con" << std::endl;
        return EXIT_FAILURE;
    }

	map.read(blood_filemap);


    if (mode == "export")
        map.extract(infoSector, duke_filemap);
    else if ((mode == "info" || mode == "duke_map_info") && argc == 3 )
		map.show();
	else if (mode == "info")
		map.printSector(infoSector, true);
	else if ( mode == "convert" )
	{
        map.scale=0.75f;
        map.processing(); // C O N V E R S I O N
        if (map.write(duke_filemap) <0)
        {
            std::cerr << "ERROR: couldn't write file : sounds.con or sounds_old.con or defs.con or pic_table.con" << std::endl;
            return EXIT_FAILURE;
        }
    };

    return 0;
};

void show_help_message()
{
	std::cout \
	<< "Syntax: blud2e -c  <Blood MAP file> <BloodCM MAP file>" << std::endl
	<< "or:" << std::endl
	<< "blud2e -i [sector number] <MAP file Duke3D or Blood crypted format>" << std::endl
	<< "blud2e -e [sector number] <MAP file Duke3D or Blood crypted format>" << std::endl
	<< "options: -h|--help|-v|--version|-c|--convert|-i|--info|-e" << std::endl
	<< "where: " << std::endl
	<< "-h | --help  ::show this message" << std::endl
	<< "-v | --version ::show version of the program" << std::endl
	<< "-i [sector number] <map file> ::show debug information about MAP " << std::endl
	<< "-e [sector number]  <Map file Duke3D or Blood format>  ::export level geometry to OBJ format" << std::endl
	<< "-c |--convert <Blood map file> <BloodCM map file> ::convert Blood's map file to BloodCM format"
	<< std::endl << std::endl;
};



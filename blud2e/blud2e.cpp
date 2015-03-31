/* Convertor MAP files by Blood video game, Monolith 1997(c) to eduke32 map format
   Project: Blood CrossMatching
   Author: flanker
   It's fork by blud2b.c, HTTP://blood.sourceforge.net with additional features
   License: GPL v.3
*/

#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <sys/stat.h> // check that file exists
#include <blud2e.h>


bool fileExists(const std::string& filename);
void show_help_message();

int main(int argc, char *argv[]) {
	std::string tex_con_file="defs.con";
	std::string sound_con_file="sounds.con";
	std::string original_sound="sounds_old.con";
	std::string pic_file="pic_table.con";

	Map map;

	const float version=0.6;
	int infoSector;

	std::cerr << "\nDecryptor MAP files from Blood video game, Monolith Production, 1997(c)\n" << \
	"Author: flanker\n" << \
	"Project: Blood Crossmatching\n" << \
	"version: " << version << "-dev" << std::endl << \
	"====================================================\n\n";
	std::string mode("blank");
	std::vector<std::string> arguments;
	for (int i=1; i<argc;i++){
		std::string arg(argv[i]);
		arguments.push_back(arg);
	};

	char * blood_filemap;
	char * duke_filemap;

	if (arguments.size() == 2) {
		if (!fileExists(arguments[1]))
		{
            std::cout << "ERROR: file: " << arguments[1] << " doesn't exist!" << std::endl;
            exit(EXIT_FAILURE);
        };
		blood_filemap = (char*)arguments[1].c_str();

		if ( arguments[0] == "-i" || arguments[0] == "--info" ) {
                mode="info";
		};
        if ( arguments[0] == "-e" || arguments[0] == "--export" ) {
                mode="export";
		};

		infoSector=-1;
	};


	if (arguments.size() == 3) {
		if (arguments[0] == "-c" || arguments[0] == "--convert" ) {
			if (!fileExists(arguments[1]))
			{
                std::cout << "ERROR: file: " << arguments[1] << " doesn't exist!" << std::endl;
                exit(EXIT_FAILURE);
            };

			blood_filemap = (char*)arguments[1].c_str();
			duke_filemap = (char*)arguments[2].c_str();
			mode="convert";

		};
		if ( arguments[0] == "-i" || arguments[0] == "--info" ) {
			if (!fileExists(arguments[2]))
				exit(EXIT_FAILURE);
			infoSector=atoi(arguments[1].c_str());
			blood_filemap = (char*)arguments[2].c_str();
			mode="info";
		};

		if ( arguments[0] == "-e" || arguments[0] == "--export" ) {
			if (!fileExists(arguments[1]))
			{
                std::cout << "ERROR: file: " << arguments[1] << " doesn't exist!" << std::endl;
                exit(EXIT_FAILURE);
            };
            infoSector=-1;
			blood_filemap = (char*)arguments[1].c_str();
			duke_filemap=(char*)arguments[2].c_str();
			mode="export";
		};
	};

    if ((arguments.size() == 4) && ( arguments[0] == "-e" || arguments[0] == "--export" )) {
            if (!fileExists(arguments[2]))
			{
                std::cout << "ERROR: file: " << arguments[2] << " doesn't exist!" << std::endl;
                exit(EXIT_FAILURE);
            };
            infoSector=atoi(arguments[1].c_str());
            blood_filemap = (char*)arguments[2].c_str();
			duke_filemap=(char*)arguments[3].c_str();
			mode="export";
    };

	if (mode == "blank" ) {
		show_help_message();
		exit(EXIT_FAILURE);
	};

	if (!fileExists(tex_con_file) || !fileExists(sound_con_file) ||
        !fileExists(original_sound) || !fileExists(pic_file)) {
		std::cerr << "ERROR: missing files: sounds.con or sounds_old.con or defs.con or pic_table.con" << std::endl;
	       	exit(EXIT_FAILURE);
	};


// C O R E ///////////////////////////////
	assert (map.sTable.open(original_sound ,sound_con_file, tex_con_file) >= 0);
    assert (map.openPicsTable(pic_file, map.targa) >= 0 );

	map.read(blood_filemap);
/////////////
	//exit(EXIT_SUCCESS);

    if (mode == "export")
        map.extract(infoSector, duke_filemap);
	else if ((mode == "info" || mode == "duke_map_info") && arguments.size() == 2 )
		map.show();
	else if (mode == "info")
		map.printSector(infoSector, true);
	else if ( mode == "convert" )
	{
        map.scale=0.75f;
        map.processing();
        assert(map.write(duke_filemap) >=0);
    };
// CORE OUT ////////////////////////////////////

    //map.rm();
	exit(EXIT_SUCCESS);
};

bool fileExists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
};

void show_help_message() {
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



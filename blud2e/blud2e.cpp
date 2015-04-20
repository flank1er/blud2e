/* Convertor MAP files by Blood video game, Monolith 1997(c) to eduke32 map format
   Project: Blood CrossMatching
   Author: flanker
   It's fork by blud2b.c, HTTP://blood.sourceforge.net with additional features
   License: GPL v.3
*/

#include <blud2e.h>

void show_help_message();

int main(int argc, char *argv[]) {

    blud2e map;
    const float version=0.7;
    int infoSector;
    std::string mode="blank";
    char * input_filename;
    char * output_filename;
    std::stringstream log;

    log << "\nDecryptor MAP files from Blood video game, Monolith Production, 1997(c)\n" << \
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
        input_filename = argv[2];
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
            input_filename = argv[2]; output_filename  = argv[3];mode="convert";

        };
        if ( std::string(argv[1]) == "-i" || std::string(argv[1]) == "--info" ) {
            if (!fileExists(argv[3]))
            {
                std::cout << "ERROR: file: " << argv[3] << " doesn't exist!" << std::endl;
                return EXIT_FAILURE;
            }
            infoSector=atoi(argv[2]); input_filename = argv[3]; mode="info";
		};

        if ( std::string(argv[1]) == "-e" || std::string(argv[1]) == "--export" ) {
            if (!fileExists(argv[2]))
			{
                std::cout << "ERROR: file: " << argv[2] << " doesn't exist!" << std::endl;
                return EXIT_FAILURE;
            };
            infoSector=-1; input_filename = argv[2];	output_filename=argv[3]; mode="export";
        };

        if ( std::string(argv[1]) == "-t" || std::string(argv[1]) == "--test" )
                {
                    if (!fileExists(argv[2]))
                    {
                        std::cout << "ERROR: file: " << argv[2] << " doesn't exist!" << std::endl;
                        return EXIT_FAILURE;
                    };
                    input_filename = argv[2]; output_filename  = argv[3];mode="testing";
                };
    };

    if ((argc == 5) && ( std::string(argv[1]) == "-e" || std::string(argv[1]) == "--export" ))
    {
            if (!fileExists(argv[3]))
			{
                std::cout << "ERROR: file: " << argv[3] << " doesn't exist!" << std::endl;
                return EXIT_FAILURE;
            };
            infoSector=atoi(argv[2]); input_filename = argv[3]; output_filename=argv[4];
			mode="export";
    };


/// C O R E ///////////////////////////////

    if (!map.read(input_filename, log))
    {
        if (mode == "export")
        {
            if (map.write_obj(infoSector, output_filename, log) == EXIT_FAILURE)
            {
                std::cout << log.str();
                return EXIT_FAILURE;
            }
        } else if ((mode == "info" || mode == "duke_map_info") && argc == 3 )
            map.show(log);
        else if (mode == "info")
            map.printSector(infoSector, true, log);
        else if ( mode == "convert" )
        {
            if (map.processing(log, 0.75f) == EXIT_FAILURE) // C O N V E R S I O N
            //if (map.processing(log, 1.f) == EXIT_FAILURE) // C O N V E R S I O N
            {
                std::cout << log.str();
                return EXIT_FAILURE;
            }
            if (map.write(output_filename,log) <0)
            {
                std::cerr << "ERROR: couldn't write file : sounds.con or sounds_old.con or defs.con or pic_table.con" << std::endl;
                return EXIT_FAILURE;
            }
        } else if ( mode == "testing") // write to crypt Blood format. only for testing
        {
            //std::cout << "Try saving map into a Blood format..." <<std::endl;
            if (map.write_v7B(output_filename, log) < 0)
            {
                std::cerr << "ERROR: couldn't write to Blood format" << std::endl;
                std::cerr << log.str() << std::endl;
                return EXIT_FAILURE;
            }
        }
    } else {
        std::cerr << log.str();
        return EXIT_FAILURE;
    }

    std::cout << log.str() << std::endl;
    return EXIT_SUCCESS;
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



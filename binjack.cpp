#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <benejson/pull.hh>
#include "GuillotineBinPack.h"

using BNJ::PullParser;

enum {
	KEY_CONTAINER_BOXES
	,KEY_CONTAINER_LENGTH
	,KEY_CONTAINER_WIDTH
	,COUNT_CONTAINER_KEYS
};

const char* s_free_rect_heuristics[GuillotineBinPack::COUNT_FREE_RECT_CHOICE_HEURISTICS] = {
	"BAF"
	,"BSSF"
	,"BLSF"
	,"WAF"
	,"WSSF"
	,"WLSF"
};

const char* s_guillotine_split_heuristics[GuillotineBinPack::COUNT_SPLIT_HEURISTICS] = {
	"SLAS"
	,"LLAS"
	,"MINAS"
	,"MAXAS"
	,"SAS"
	,"LAS"
};

const char* s_container_keys[COUNT_CONTAINER_KEYS] = {
	"boxes"
	,"length"
	,"width"
};

int main(int argc, const char* argv[]){
	try{
		/* Expecting following format:
		 * FREE_RECT_HEURISTIC GUILLOTINE_HEURISTIC INPUT_FILENAME */
		if(argc < 4)
			throw std::runtime_error("Please specify FREE_RECT_HEURISTIC GUILLOTINE_HEURISTIC INPUT_FILENAME");

		enum GuillotineBinPack::FreeRectChoiceHeuristic frc_heuristic =
			GuillotineBinPack::COUNT_FREE_RECT_CHOICE_HEURISTICS;
		for(unsigned i = 0; i < GuillotineBinPack::COUNT_FREE_RECT_CHOICE_HEURISTICS; ++i){
			if(!strcmp(s_free_rect_heuristics[i], argv[1])){
				frc_heuristic = i;
				break;
			}
		}
		if(frc_heuristic == GuillotineBinPack::COUNT_FREE_RECT_CHOICE_HEURISTICS)
			throw std::runtime_error("Invalid FREE_RECT_HEURISTIC!");

		enum GuillotineBinPack::GuillotineSplitHeuristic guill_heuristic =
			GuillotineBinPack::COUNT_SPLIT_HEURISTICS;
		for(unsigned i = 0; i < GuillotineBinPack::COUNT_SPLIT_HEURISTICS; ++i){
			if(!strcmp(s_guillotine_split_heuristics[i], argv[2])){
				guill_heuristic = i;
				break;
			}
		}
		if(guill_heuristic == GuillotineBinPack::COUNT_SPLIT_HEURISTICS)
			throw std::runtime_error("Invalid GUILLOTINE_HEURISTIC!");

		/* Determine heuristic from command line parameters. */

		/* Read JSON input from file. */
		/* First parse File Header. */
		struct stat buf;

		/* If file does not exist, then problems. */
		if(stat(argv[3], &buf))
			throw std::runtime_error("Input file does not exist!");

		if(!buf.st_size)
			throw std::runtime_error("Input file does not contain data!");

		/* Open file and allocate just enough data to hold it, timestamp, and '\n' */
		int fd = open(argv[3], O_RDONLY | O_CLOEXEC);
		if(-1 == fd)
			throw std::runtime_error("Unable to input file!");

		/* Mmap file (since its already in RAM). */
		void* mdata =
			mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if(MAP_FAILED == mdata){
			close(fd);
			throw std::runtime_error("mmap error");
		}

		/* Set up parser. */
		uint32_t stack[64];
		PullParser parser(64, stack);
		parser.Begin(reinterpret_cast<const uint8_t*>(mdata), buf.st_size);

		parser.Pull();
		BNJ::VerifyMap(parser);

		unsigned container_width = 0;
		unsigned container_length = 0;
		std::vector<RectSize> rects;

		while(PullParser::ST_ASCEND_MAP !=
			parser.Pull(s_container_keys, COUNT_CONTAINER_KEYS))
		{
			switch(parser.GetValue().key_enum){
				case KEY_CONTAINER_BOXES:
					BNJ::VerifyList(parser);
					/* Read in array of dimensions; (ignore third dimension for now). */
					while(PullParser::ST_ASCEND_LIST != parser.Pull()){
						BNJ::VerifyList(parser);
						unsigned idx = 0;
						unsigned dims[3];
						while(PullParser::ST_ASCEND_LIST != parser.Pull()){
							if(idx == 3)
								throw PullParser::invalid_value("Overlong array!", parser);
							BNJ::Get(dims[idx], parser);
							++idx;
						}

						RectSize r = {dims[0], dims[1]};
						rects.push_back(r);
					}
					break;

				case KEY_CONTAINER_LENGTH:
					BNJ::Get(container_length, parser);
					break;

				case KEY_CONTAINER_WIDTH:
					BNJ::Get(container_width, parser);
					break;

				default:
					throw PullParser::invalid_value("Invalid key key!", parser);
			}
		}

		GuillotineBinPack packer(container_width, container_length);
		packer.Insert(rects, true, frc_heuristic, guill_heuristic);

		std::vector<Rect> & used = packer.GetUsedRectangles();
		fprintf(stdout, "[\n");
		for(unsigned i = 0; i < used.size(); ++i){
			const Rect& r = used[i];
			if(i)
				fputc(',', stdout);
			fprintf(stdout, "[%u, %u, %u, %u]\n", r.width, r.height, r.x, r.y);
		}
		fprintf(stdout, "]\n");
		return 0;
	}
	catch(const std::exception& e){
		fprintf(stderr, "%s\n", e.what());
		return 1;
	}
}

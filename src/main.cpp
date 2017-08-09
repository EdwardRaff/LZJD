// Typical includes
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream> 
#include <iostream>
#include <cstdint>
#include <iterator>

// Other includes
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <stdint.h>


// Project includes
#include "LZJD.h"
#include "MurmurHash3.h"

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem; 
namespace bi = boost::archive::iterators;

//Used for handling output to file or STD out
ostream* out_target = NULL;

uint64_t digest_size = 1024;
bool compare;
bool gen_compare;
int32_t threshold;
uint32_t threads;
string alt_output;


static void readAllBytes(char const* filename, vector<char>& result)
{
    ifstream ifs(filename, ios::binary|ios::ate);
    ifstream::pos_type pos = ifs.tellg();

    result.clear();//empty out
    result.resize(pos); //make sure we have enough space
    ifs.seekg(0, ios::beg);
    ifs.read(&result[0], pos);
}

static pair<vector<string>,vector<vector<int32_t>>> readDigest(string path)
{
    vector<string> s_out;
    vector<vector<int32_t>> i_out;
    string line;
    ifstream inFile(path);
    if (inFile.is_open())
    {
        while (getline(inFile, line))
        {
            auto first_colon = line.find(":", 0);
            auto second_colon = line.find(":", first_colon+1);
            string path = line.substr(first_colon+1, second_colon-first_colon-1);
            string base64ints = line.substr(second_colon+1, line.size()-second_colon);
//            cout << path << "  " <<  base64ints;
//            cout << endl;
              // Remove the padding characters
            auto size = base64ints.size();
            while (size > 0 && base64ints[size - 1] == '=')
                size--;
            base64ints = base64ints.substr(0, size);
            
            
            //TODO this is not 100% kosher, but C++ is a pain. 
            
            typedef 
                bi::transform_width<
                    bi::binary_from_base64<bi::remove_whitespace<string::const_iterator>>,
                        8, 6
                > 
                base64_dec; 

            vector<uint8_t> int_parts;
            
            copy(
                 base64_dec(base64ints.cbegin()),
                 base64_dec(base64ints.cend()),
                 std::back_inserter(int_parts)
                 );

            vector<int32_t> decoded_ints(int_parts.size()/4);
            for (int i = 0; i < int_parts.size(); i+=4)
            {
                //big endian extraction of the right value
                int32_t dec_i = (int_parts[i+0]<<24) | (int_parts[i+1]<<16) | (int_parts[i+2]<<8) | (int_parts[i+3]<<0);
                decoded_ints[i/4] = dec_i;
//                cout << dec_i << ", ";
            }
//            cout << ": " << int_parts.size() << " " <<  base64ints.size() << " " << endl;
//            cout << decoded_ints.size() << endl;
            //out.push_back(make_pair(path, decoded_ints));
            i_out.push_back(decoded_ints);
            s_out.push_back(path);
        
        }
        inFile.close();
        return make_pair(s_out, i_out);
    }
    else
    {
        //throw an error
        return make_pair(s_out, i_out);
    }
}

void compareDigests(pair<vector<string>,vector<vector<int32_t>>>& A, pair<vector<string>,vector<vector<int32_t>>>& B)
{
    vector<string> A_s = A.first;
    vector<string> B_s = B.first;
    vector<vector<int32_t>> A_i = A.second;
    vector<vector<int32_t>> B_i = B.second;
    
    for (size_t i = 0; i < A_i.size(); i++)
    {
        string hAiN = A_s[i];
        vector<int32_t> hAiH = A_i[i];

        size_t j_start;
        if (&A == &B)
            j_start = i + 1; //don't self compare / repeat comparisons
        else
            j_start = 0;
        

        for (size_t j = j_start; j < B_i.size(); j++)
        {
            int32_t sim = similarity(hAiH, B_i[j]);
            if (sim >= threshold)
            {
                *out_target << hAiN << "|" << B_s[j] << "|";
                //make the similarity output formated as 00X, 0XX, or 100, depending on value
                if (sim < 100)
                {
                    *out_target << "0";
                    if (sim < 10)
                        *out_target << "0";
                }
                *out_target << sim << "\n";
            }
        }
    }
}

int main(int argc, char* argv[]) 
{
    bool deep;
    
	po::options_description desc("LZJD Options");
	desc.add_options()
		("help,h", "produce help message")
        ("deep,r", po::bool_switch(&deep)->default_value(false), "generate SDBFs from directories and files")
        ("compare,c", po::bool_switch(&compare)->default_value(false), "compare SDBFs in file, or two SDBF files")
        ("gen-compare,g", po::bool_switch(&gen_compare)->default_value(false), "compare all pairs in source data")
		("threads,p", po::value<uint32_t>(&threads)->default_value(1), "restrict compute threads to N threads")
        ("threshold,t", po::value<int32_t>(&threshold)->default_value(20), "only show results >=threshold")
        ("output,o", po::value<string>(&alt_output)->default_value(""), "send output to files")
	;
    
    //give me extra arguments https://stackoverflow.com/questions/10178525/how-to-handle-unsolicited-parameters-in-boostprogram-options
    vector<string> additionalParameters;

    po::variables_map vm;
    try
    {
        po::parsed_options parsed = po::command_line_parser(argc, argv).
            options(desc).allow_unregistered().run();
        po::store(parsed, vm);
        additionalParameters = po::collect_unrecognized(parsed.options,
                                                        po::include_positional);
        po::notify(vm);
	} catch(exception &e) {
		// cerr << e.what() << endl; // Useful for debugging boost::program_options
		cerr << "Invalid options provided." << endl << desc << endl;
		return EXIT_FAILURE;
	}

	if (vm.count("help")) {
		cout << desc << endl;
		return EXIT_FAILURE;
	}
    
    if(alt_output != "")
        out_target = new ofstream(alt_output);
    else
        out_target = &cout;

    
    if(compare)
    {
        if(additionalParameters.size() > 2 || additionalParameters.size() == 0)
        {
            cout << "Compare option (-c --compare) requires either one or two inputs, which are paths to digest files. Instead, " << additionalParameters.size() << " inputs were given." << endl;
            return EXIT_FAILURE;
        }
        
        pair<vector<string>,vector<vector<int32_t>>> digest1 = readDigest(additionalParameters[0]);
        if(additionalParameters.size() == 2)
        {
            pair<vector<string>,vector<vector<int32_t>>> digest2 = readDigest(additionalParameters[1]);
            compareDigests(digest1, digest2);
        }
        else
            compareDigests(digest1, digest1);
        
        if(out_target != &cout)
        {
            ((ofstream*)out_target)->close();
            delete out_target;
        }
        return EXIT_SUCCESS;
    }
    
    
    vector<fs::path> toProcess;
    for (auto input_path : additionalParameters)
    {
        fs::path p(input_path);
        if(!fs::exists(p))
            continue;
        if(fs::is_directory(p))
        {
            if(deep)
            {
                for (fs::recursive_directory_iterator end, dir(p); dir != end; ++dir)
                {
                    if(!fs::is_directory(*dir))
                        toProcess.push_back(*dir);
                }
            }
        }
        else
            toProcess.push_back(p);
    }
       

    
    //reused temp space
    vector<char> all_bytes;
    //store digsets IF we do gen-compare
    vector<string> digestNewName;
    vector<vector<int32_t>> digesstNewInts;
    for(fs::path p : toProcess)
    {
        readAllBytes(p.c_str(), all_bytes);
        
        vector<int32_t> di = digest(digest_size, all_bytes);
        
        if(gen_compare)
        {
            digesstNewInts.push_back(di);
            digestNewName.push_back(p.generic_string());
        }
        else
        {
            *out_target << "lzjd:" << p.generic_string() << ":";
            out_target->flush();
            //See http://www.boost.org/doc/libs/1_48_0/libs/serialization/doc/dataflow.html for base64 in boost
            std::stringstream os;
            typedef 
                bi::base64_from_binary<    // convert binary values to base64 characters
                    bi::transform_width<   // retrieve 6 bit integers from a sequence of 32 bit ints
                        vector<int32_t>::const_iterator,
                        6,
                        32
                    >
                > 
                base64_text; 

            copy(
                 base64_text(di.cbegin()),
                 base64_text(di.cend()),
                 ostream_iterator<char>(os)
                 );

            *out_target << os.str();
            //add padding to Base64 output 
            switch(di.size()*4 % 3)
            {
                case 1:
                    *out_target << "=";
                case 2:
                    *out_target << "=";
                case 0:
                    /*nothing happens case*/;
            }
            *out_target << "\n";
        }
    }
    
    if(gen_compare)
    {
        pair<vector<string>,vector<vector<int32_t>>> digestNew = make_pair(digestNewName, digesstNewInts);
        compareDigests(digestNew, digestNew);
    }

    if(out_target != &cout)
    {
        ((ofstream*)out_target)->close();
        delete out_target;
    }
	return EXIT_SUCCESS;
}

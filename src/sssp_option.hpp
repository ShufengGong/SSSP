//
// Created by gongsf on 26/9/19.
//

#ifndef SSSP_SSSP_OPTION_HPP
#define SSSP_SSSP_OPTION_HPP

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

namespace sssp {

    using std::string;

    struct sssp_option {
        boost::program_options::options_description option_desc_;
        string input = "";
        string output = "";
        string fun = "";
        int model = -1;
        int num_thread = -1;//for parallel
        int num_task = -1; // for distributed
        int num_vertex = -1;
        int source = -1;
        int req_capacity = -1;

        float delta = -1; // for delta step method
        int delta_length = -1;

        int iter_time = -1; //for bellman-ford

        sssp_option(const char *program_name) : option_desc_(
                (boost::format("%s options") % program_name).str().c_str()) {
            option_desc_.add_options()
                    ("help,h", "produce help message")
                    ("model", boost::program_options::value<int>(&model),
                     "perform model, 0: stand-alone, 1: parallel, 2: distributed")
                    ("fun", boost::program_options::value<string>(&fun), "perform function")
                    ("in", boost::program_options::value<string>(&input), "path of input")
                    ("out", boost::program_options::value<string>(&output), "path of output")
                    ("num_thread", boost::program_options::value<int>(&num_thread), "number of threads")
                    ("num_vertex", boost::program_options::value<int>(&num_vertex), "number of vertex")
                    ("source", boost::program_options::value<int>(&source), "source vertex id")
                    ("delta", boost::program_options::value<float>(&delta), "delta step method")
                    ("delta_length", boost::program_options::value<int>(&delta_length), "delta lenght")
                    ("num_task", boost::program_options::value<int>(&num_task), "num of task")
                    ("req_capacity", boost::program_options::value<int>(&req_capacity), "the capacity of req")
                    ("iter_time", boost::program_options::value<int>(&iter_time), "the iteration times of bellman-ford");

        }

        ~sssp_option() {

        }

        bool parse_command(int &argc, char **&argv) {

            using std::cerr;
            using std::endl;

            bool flag_help = false;

            try {

                boost::program_options::variables_map var_map;
                boost::program_options::store(boost::program_options::
                                              parse_command_line(argc, argv, option_desc_),
                                              var_map);
                boost::program_options::notify(var_map);

                if (var_map.count("help")) {
                    flag_help = true;
                }


            }
            catch (std::exception &excep) {
                cerr << "error: " << excep.what() << "\n";
                flag_help = true;
            }
            catch (...) {
                cerr << "Exception of unknown type!\n";
                flag_help = true;
            }

            if (true == flag_help) {
                cerr << option_desc_ << endl;
                return false;
            }
            return true;
        }

    };
}

#endif //SSSP_SSSP_OPTION_HPP

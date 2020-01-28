/*****************************************************
 * Author: Ehsan Haghshenas (ehaghshe AT sfu DOT ca) *
 *****************************************************/

#include "Commandline.hpp"

#include <getopt.h>
#include <sys/stat.h>
// #include <unistd.h>
#include "Common.hpp"

void print_help_short()
{
    fprintf(stderr, "usage: detectSV -c contig.fasta -l longread.fasta -m lr2contig.paf -d outdir [-r ref.fa -s short.fasta] [options]\n");
}

void print_help()
{
    print_help_short();
    fprintf(stderr, "\n");
    fprintf(stderr, "Required options:\n");
    fprintf(stderr, "    -c STR            Path to contigs file (also --contig)\n");
    fprintf(stderr, "    -l STR            Path to long read dataset (also --long)\n");
    fprintf(stderr, "    -m STR            Path to mappings of long reads onto contigs (also --mapping)\n");
    fprintf(stderr, "    -d STR            Path to the output directory (also --dir)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Advanced options:\n");
    // fprintf(stderr, "    --aln-block-uniq  Minimum length of alignment block for unique SR contigs [%d]\n", gopt.min_aln_block_uniq);
    // fprintf(stderr, "    --aln-sim-uniq    Minimum alignment similarity [%.2lf]\n", gopt.min_aln_sim_uniq);
    fprintf(stderr, "    --aln-block       Minimum length of alignment block [%d]\n", gopt.min_aln_block);
    fprintf(stderr, "    --aln-sim         Minimum alignment similarity [%.2lf]\n", gopt.min_aln_sim);
    fprintf(stderr, "    --uniq-dev        Maximum deviation from mean frequency of uniq contigs [%.2lf]\n", gopt.max_uniq_dev);
    fprintf(stderr, "    --edge-sup        Minimum number of long read supporting each edge [%d]\n", gopt.min_edge_sup);
    fprintf(stderr, "\n");
    fprintf(stderr, "Other options:\n");
    fprintf(stderr, "    -t INT            Number of CPU cores to use (also --threads)\n");
    fprintf(stderr, "    --long-fofn       The file passed by -l is fofn\n");
    fprintf(stderr, "    --mapping-fofn    The file passed by -m is fofn\n");
    // fprintf(stderr, "    -r STR            Path to reference genome (also --ref)\n");
    // fprintf(stderr, "    -s STR            Path to short read dataset (also --short)\n");
    fprintf(stderr, "    --version         Prints version (%s)\n", gopt.prog_version.c_str());
    fprintf(stderr, "    -h                Prints this help message (also --help)\n");
    fprintf(stderr, "\n");
}

void set_option_defaults()
{
    gopt.contig_path = ""; // path to the contig file constructed by bcalm
    gopt.long_path = ""; // path to the long read dataset
    gopt.long_fofn = false;
    gopt.mapping_path = ""; // path to mapping of long reads onto contigs generated by minimap2
    gopt.mapping_fofn = false;
    gopt.out_dir = "";
    // gopt.refPath = ""; // path to the reference genome fasta file
    // gopt.shortPath = ""; // path to the short read dataset
    gopt.min_aln_block = 500; // minimum alignment block size
    // gopt.min_aln_block_uniq = 500; // minimum alignment block size
    gopt.min_aln_sim = 0.85; // minimum alignment similarity
    // gopt.min_aln_sim_uniq = 0.85; // minimum alignment similarity
    gopt.min_aln_mapq = 55; // minimum alignment MAPQ
    // gopt.minAlnMapq_uniq = 55; // minimum alignment MAPQ for unique contigs
    gopt.max_uniq_dev = 0.15; // maximum deviation from mean frequency
    gopt.min_edge_sup = 3; // minimum support for each edge
    gopt.num_threads = 1;
    gopt.prog_version = "0.8a1";
}

bool parse_command_line(int argc, char *argv[])
{
    set_option_defaults();
    if(argc == 1)
    {
        print_help_short();
        return false;
    }

    static struct option long_options[] = 
    {
        {"contig",         required_argument, 0, 'c'}, // 0
        {"long",           required_argument, 0, 'l'}, // 1
        {"mapping",        required_argument, 0, 'm'}, // 2
        {"dir",            required_argument, 0, 'd'}, // 3
        {"help",           no_argument,       0, 'h'}, // 4
        {"threads",        required_argument, 0, 't'}, // 5
        {"version",        no_argument,       0,  0 }, // 6
        {"long-fofn",      no_argument,       0,  0 }, // 7
        {"mapping-fofn",   no_argument,       0,  0 }, // 8
        {"aln-block",      required_argument, 0,  0 }, // 9
        {"aln-sim",        required_argument, 0,  0 }, // 10
        {"uniq-dev",       required_argument, 0,  0 }, // 11
        {"edge-sup",       required_argument, 0,  0 }, // 12
        {0,0,0,0}
    };
    
    int c, long_index;
    int32_t optarg_int32;
    // while ( (c = getopt_long( argc, argv, "c:l:m:d:r:s:t:h", long_options, &long_index))!= -1 )
    while ( (c = getopt_long( argc, argv, "c:l:m:d:t:h", long_options, &long_index))!= -1 )
    {
        switch (c)
        {
            case 'c':
                gopt.contig_path = optarg;
                break;
            case 'l':
                gopt.long_path = optarg;
                break;
            case 'm':
                gopt.mapping_path = optarg;
                break;
            case 'd':
                gopt.out_dir = optarg;
                break;
            // case 'r':
            //     gopt.refPath = optarg;
            //     break;
            // case 's':
            //     gopt.shortPath = optarg;
            //     break;
            case 't':
                optarg_int32 = str2type<int32_t>(optarg);
                if(optarg_int32 < 1)
                    gopt.num_threads = 1;
                else if(optarg_int32 > int32_t(std::thread::hardware_concurrency()))
                    gopt.num_threads = std::thread::hardware_concurrency();
                else
                    gopt.num_threads = str2type<uint32_t>(optarg);
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
                // break;
            case 0:
                if(long_index == 6) // --version
                {
                    fprintf(stdout, "%s\n", gopt.prog_version.c_str());
                    exit(EXIT_SUCCESS);
                    // break;
                }
                else if(long_index == 7) // --long-fofn
                {
                    gopt.long_fofn = true;
                }
                else if(long_index == 8) // --mapping-fofn
                {
                    gopt.mapping_fofn = true;
                }
                else if(long_index == 9) // --aln-block
                {
                    optarg_int32 = str2type<int32_t>(optarg);
                    if(optarg_int32 < 0)
                        gopt.min_aln_block = 500;
                    else
                        gopt.min_aln_block = optarg_int32;
                }
                else if(long_index == 10) // --aln-sim
                {
                    gopt.min_aln_sim = str2type<double>(optarg);
                    if(gopt.min_aln_sim < 0 || gopt.min_aln_sim > 1)
                        gopt.min_aln_sim = 0.85;
                }
                else if(long_index == 11) // --uniq-dev
                {
                    gopt.max_uniq_dev = str2type<double>(optarg);
                    // if(gopt.max_uniq_dev < 0 || gopt.max_uniq_dev > 1)
                    //     gopt.max_uniq_dev = 0.15;
                }
                else if(long_index == 12) // --edge-sup
                {
                    optarg_int32 = str2type<int32_t>(optarg);
                    if(optarg_int32 < 0)
                        gopt.min_edge_sup = 3;
                    else
                        gopt.min_edge_sup = optarg_int32;
                }
                else
                {
                    print_help_short();
                    return false;
                }
                break;
            default:
                print_help_short();
                return false;
        }
    }

    if(gopt.contig_path == "")
    {
        fprintf(stderr, "[ERROR] (CommandLine:parseCommandLine) option -c is required!\n");
        print_help_short();
        return false;
    }

    if(gopt.long_path == "")
    {
        fprintf(stderr, "[ERROR] (CommandLine:parseCommandLine) option -l is required!\n");
        print_help_short();
        return false;
    }

    if(gopt.mapping_path == "")
    {
        fprintf(stderr, "[ERROR] (CommandLine:parseCommandLine) option -m is required!\n");
        print_help_short();
        return false;
    }

    if(gopt.out_dir == "")
    {
        fprintf(stderr, "[ERROR] (CommandLine:parseCommandLine) option -d is required!\n");
        print_help_short();
        return false;
    }

    errno = 0;
    int status = mkdir(gopt.out_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(status == -1 && errno != EEXIST)
        return false;

    if(gopt.long_fofn == true)
        fprintf(stderr, "[NOTE] file passed by -l is a file of file names (FOFN)\n");
    if(gopt.mapping_fofn == true)
        fprintf(stderr, "[NOTE] file passed by -m is a file of file names (FOFN)\n");
    fprintf(stderr, "\n");

//     if(gopt.refPath == "")
//     {
//         fprintf(stderr, "[ERROR] (CommandLine:parseCommandLine) option -r is required!\n");
//         print_help_short();
//         return false;
//     }

//     if(gopt.shortPath == "" && gopt.unitigPath == "")
//     {
//         fprintf(stderr, "[ERROR] (CommandLine:parseCommandLine) one of the options -s or -u is required!\n");
//         print_help_short();
//         return false;
//     }

    return true;
}

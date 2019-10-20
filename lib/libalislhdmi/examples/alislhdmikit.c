#include <alislhdmi.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


extern int hdmitest(int argc, char *argv[]);
extern int disptest(int argc, char *argv[]);
extern int videotest(int argc, char *argv[], unsigned int b_wait_finish);
extern int hdmikumsg(int argc, char* argv[]);
extern int hdcptest(int argc, char* argv[]);
//int	  l_opt_ind;
char *l_opt_arg = NULL;
char* const short_options = "c:h:d:v:k:";

struct option long_options[] = {
    { "hdcp",	1, NULL, 'c' },
    { "hdmi",	1, NULL, 'h' },
    { "disp",	1, NULL, 'd' },
    { "video",	1, NULL, 'v' },
    { "kumsg",  1, NULL, 'k' },
    { 0, 0, 0, 0},
};


int main(int argc, char *argv[])
{
    int i;
    int c;
    //  struct option *long_option_map =NULL;


    while((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (c) {

            case 'd':
                l_opt_arg = optarg;
                //printf("[%s][%d]l_opt_arg = %s\n",__FUNCTION__, __LINE__, l_opt_arg);
                for (i=0; i<argc; i++) {
                    if (argv[i] == optarg) break;
                }

                disptest(argc-i, &argv[i]);
                break;
            case 'c':
                l_opt_arg = optarg;
                //printf("[%s][%d]l_opt_arg = %s\n",__FUNCTION__, __LINE__, l_opt_arg);
                for (i=0; i<argc; i++) {
                    if (argv[i] == optarg) break;
                }
                hdcptest(argc-i, &argv[i]);
                break;
            case 'h':
                l_opt_arg = optarg;
                //printf("[%s][%d]l_opt_arg = %s\n",__FUNCTION__, __LINE__, l_opt_arg);
                for (i=0; i<argc; i++) {
                    if (argv[i] == optarg) break;
                }

                hdmitest(argc-i, &argv[i]);
                break;
            case 'v':
                l_opt_arg = optarg;
                //printf("[%s][%d]l_opt_arg = %s\n",__FUNCTION__, __LINE__, l_opt_arg);
                for (i=0; i<argc; i++) {
                    if (argv[i] == optarg) break;
                }

                videotest(argc-i, &argv[i], 1);
                break;
            case 'k':
                l_opt_arg = optarg;
                //printf("[%s][%d]l_opt_arg = %s\n",__FUNCTION__, __LINE__, l_opt_arg);
                for (i=0; i<argc; i++) {
                    if (argv[i] == optarg) break;
                }

                hdmikumsg(argc-i, &argv[i]);
                break;
        }

    }

    return 0;
}

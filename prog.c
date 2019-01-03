#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "rex.h"

extern int errno;

int main(int argc, char **argv)
{
	rexdata rex;
	int stat;
        FILE *fp;

        if (argc < 3) {
		fprintf(stderr, "%s Dateiname\n", argv[0]);
        system("PAUSE");
		return -1;
	}


        fp = fopen(argv[2], "w");


        if (fp == 0) {
                fprintf(stderr,"Fehler beim Schreiben der Datei %s: %s\n",
                                argv[2], strerror(errno));
                                system("PAUSE");
                                exit(2);
        return -1;
        }


	stat = getrex(argv[1], &rex);
	if (stat != 0) {
		if (stat == -2)
			fprintf(stderr,
				"Fehler beim Lesen der Datei %s: %s\n",
				argv[1], strerror(errno));
		else
			fprintf(stderr, "getrex error\n");
		return -1;
	}

        fprintf(fp, "*** Main Header ***\n");
        fprintf(fp, "Sample=%f Hz\n", rex.sample);
        fprintf(fp, "Max Value=%u\n", rex.max_val);
        fprintf(fp, "Channels=%i\n", (int) rex.channel_nr);
        fprintf(fp, "Block Len=%u\n", rex.blocklen); 
	{
		int i;
                fprintf(fp, "\n*** Channel Header ***\n");
		for (i = 0; i < rex.channel_nr; i++) {
			int k;
                        fprintf(fp, "Ch_num:%d\n", rex.channel[i].num);
                        fprintf(fp, "unit:");
			for (k = 0; k < 8; k++)
                                fprintf(fp, "%c", rex.channel[i].unit[k]);
                        fprintf(fp, "\n");
                        fprintf(fp, "scalefaktor:%f\n",
			       rex.channel[i].scalefaktor);
                        fprintf(fp, "coupling: ");
			switch (rex.channel[i].coupling) {
			case COUPLING_UNKNOW:
                                fprintf(fp, "unknow\n");
				break;
			case COUPLING_DC:
                                fprintf(fp, "DC\n");
				break;
			case COUPLING_AC:
                                fprintf(fp, "AC\n");
				break;
			default:
                                fprintf(fp, "huh ?\n");

			}
                        fprintf(fp, "delay:%d\n\n", rex.channel[i].delay);

                        fprintf(fp, "text len:%d\n",
			       (int) rex.channel[i].text_valid);
                        fprintf(fp, "text:%s\n", rex.channel[i].text);
                        fprintf(fp, "over_valid:%d\n",
			       rex.channel[i].over_valid);
                        fprintf(fp, "\n");
		}
	}
        {
		int i;
                fprintf(fp, "\n*** Data ***\n"); 
                fprintf(fp, "data nr:%ld\n\n", rex.data_nr);
		for (i = 0; i < rex.data_nr; i++) {
			int k;
               
         fprintf(fp, "%.10f",(double)i*(1.0/rex.sample)); 
         
			for (k = 0; k < rex.channel_nr; k++) { 
			k=0;
                                fprintf(fp, "\t%f", rex.channel[k].data[i]);
			}
                        fprintf(fp, "\n");
		}
	}
	freerex(&rex);
        fclose(fp);
	return 0;
}


#ifndef __REX_H__
#define __REX_H__


#define COUPLING_UNKNOW 0
#define COUPLING_DC 1
#define COUPLING_AC 2

typedef struct {
	unsigned short num;
	char unit[8];
	float scalefaktor;
	unsigned short coupling;
	unsigned short delay;

	char text_valid;
	char text[40];
	char over_valid;
	char over[2048];

	double *data;
} channel;

typedef struct {
	double sample;
	unsigned short max_val;
	char channel_nr;
	unsigned short blocklen;
	long data_nr;
	channel *channel;
} rexdata;


int getrex(char *name, rexdata * data);
void freerex(rexdata * data);

#endif

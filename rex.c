#include "rex.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void _freerex(rexdata * data, int channels)
{
	int i;
	if (data->channel != NULL) {
		for (i = 0; i < channels; i++) {
			if (data->channel[i].data != NULL)
				free(data->channel[i].data);
		}
		free(data->channel);
	}
}

void freerex(rexdata * data)
{
	_freerex(data, data->channel_nr);
}

static double round(double x)
{
	return ceil(x - 0.5);
}

int getrex(char *name, rexdata * data)
{
	enum states { s_magic, s_head, s_chead, s_data, s_error,
		s_done
	};
	FILE *file = fopen(name, "rb");
	int state = s_magic;
	int init_ch = 0;

	if (file == NULL) {
		return -2;
	}

	data->channel = NULL;
	while (state < s_done) {
		switch (state) {
		case s_magic:
			{
				char magic[4];
				char mag[] = { 0x03, 'R', 'X', '1' };
				int i;

				if (fread
				    (magic, sizeof(char), 4, file) != 4) {
					state = s_error;
					break;
				}
				for (i = 0; i < 4; i++) {
					if (magic[i] != mag[i]) {
						state = s_error;
						break;
					}
				}
			}
			state = s_head;
			break;

		case s_head:
			if (fread(&data->sample, sizeof(double), 1, file)
			    != 1) {
				state = s_error;
				break;
			}
			if (fread
			    (&data->max_val, sizeof(unsigned short),
			     1, file)
			    != 1) {
				state = s_error;
				break;
			}
			if (fseek(file, 10, SEEK_CUR) != 0) {
				state = s_error;
				break;
			}
			if (fread(&data->channel_nr, sizeof(char), 1, file)
			    != 1) {
				state = s_error;
				break;
			}
			if (fseek(file, 4, SEEK_CUR) != 0) {
				state = s_error;
				break;
			}
			if (fread
			    (&data->blocklen, sizeof(unsigned short),
			     1, file)
			    != 1) {
				state = s_error;
				break;
			}
			if (fseek(file, 17, SEEK_CUR) != 0) {
				state = s_error;
				break;
			}

			state = s_chead;
			break;

		case s_chead:
			{
				int i;
				data->channel = NULL;
				for (i = 0; i < data->channel_nr; i++) {
					channel *ch_tmp;
					ch_tmp =
					    realloc(data->channel,
						    (i +
						     1) * sizeof(*ch_tmp));
					if (ch_tmp == NULL) {
						state = s_error;
						break;
					}
					data->channel = ch_tmp;
					data->channel[i].data = NULL;
					init_ch++;
					if (fread
					    (&data->channel[i].num,
					     sizeof(unsigned short),
					     1, file) != 1) {
						state = s_error;
						break;
					}

					if (fread
					    (&data->channel[i].unit,
					     sizeof(char), 8, file) != 8) {
						state = s_error;
						break;
					}

					if (fread
					    (&data->channel[i].
					     scalefaktor,
					     sizeof(float), 1,
					     file) != 1) {
						state = s_error;
						break;
					}

					if (fread
					    (&data->channel[i].
					     coupling,
					     sizeof(unsigned short),
					     1, file) != 1) {
						state = s_error;
						break;
					}

					if (fread
					    (&data->channel[i].delay,
					     sizeof(unsigned short),
					     1, file) != 1) {
						state = s_error;
						break;
					}
				}

				if (state == s_error)
					break;

				for (i = 0; i < data->channel_nr; i++) {
					if (fread
					    (&data->channel[i].
					     text_valid,
					     sizeof(char), 1, file) != 1) {
						state = s_error;
						break;
					}
					if (data->channel[i].
					    text_valid == 0) {
						data->channel[i].
						    text[0] = '\0';
						if (fseek
						    (file, 40,
						     SEEK_CUR) != 0) {
							state = s_error;
							break;
						}
					} else {
						if (fread
						    (&data->
						     channel[i].text,
						     sizeof(char),
						     40, file) != 40) {
							state = s_error;
							break;
						}
						data->channel[i].
						    text[(int) data->
							 channel[i].
							 text_valid]
						    = '\0';
					}
					if (fread
					    (&data->channel[i].
					     over_valid,
					     sizeof(char), 1, file) != 1) {
						state = s_error;
						break;
					}
					if (data->channel[i].
					    over_valid == 0) {
						if (fseek
						    (file, 2048,
						     SEEK_CUR) != 0) {
							state = s_error;
							break;
						}
					} else {
						if (fread
						    (&data->
						     channel[i].over,
						     sizeof(char),
						     2048, file) != 2048) {
							state = s_error;
							break;
						}
					}
				}
				if (state == s_error)
					break;
			}
			state = s_data;
			break;

		case s_data:
			{
				long pos1 = ftell(file);
				long pos2 = 0;
				long delta;
				long i, k, m, n;
				double bits =
				    (int) round(log(data->max_val) /
						log(2) + 1);
				int bytes = bits / 8;
/*                                printf("bits:%f\n", bits);
                                printf("bytes:%d\n", bytes);   */
				if (pos1 == -1) {
					state = s_error;
					break;
				}
				if (fseek(file, 0, SEEK_END) != 0) {
					state = s_error;
					break;
				}
				pos2 = ftell(file);
				if (pos2 == -1) {
					state = s_error;
					break;
				}
				delta = pos2 - pos1;
				data->data_nr =
				    delta / (data->channel_nr * bytes);
				if (fseek(file, pos1, SEEK_SET) != 0) {
					state = s_error;
					break;
				}
				for (i = 0; i < data->channel_nr; i++) {
					data->channel[i].data =
					    (double *) malloc(data->
							      data_nr
							      *
							      sizeof
							      (double));
					if (data->channel[i].data == NULL) {
						state = s_error;
						break;
					}
				}

				for (i = 0, k = 0, n = 0, m = 0;
				     i <
				     data->data_nr *
				     data->channel_nr; i++) {
					int l;
					long int buff = 0;

					for (l = 0; l < bytes; l++) {
						int buf;
						if ((buf =
						     fgetc(file)) == EOF) {
							state = s_error;
							break;
						}
						buff |= buf << 8 * l;
					}

					if (state == s_error)
						break;

					if (buff >= data->max_val)
						buff -=
						    2 * (data->max_val);

					data->channel[k].data[m] =
					    (double) buff *data->
					    channel[k].scalefaktor /
					    data->max_val;
					if ((i + 1) % data->blocklen == 0) {
						k++;
						if (k >= data->channel_nr) {
							k = 0;
							n += data->
							    blocklen;
						}
						m = n;
					} else {
						m++;
					}
				}
				if (state == s_error)
					break;

			}
			state = s_done;
			break;

		case s_error:
			_freerex(data, init_ch);
			fclose(file);
			return 1;

		default:
			state = s_error;
		}

	}

	fclose(file);
	return 0;
}

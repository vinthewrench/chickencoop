//
//  macos_gpiod.c
//  coopserver
//
//  Created by Vincent Moscaritolo on 12/24/21.
//

#include <stdio.h>
#include <errno.h>

#if defined(__APPLE__)
// used for cross compile on osx

#include "macos_gpiod.h"

struct gpiod_chip {
	struct gpiod_line **lines;
	unsigned int num_lines;

	int fd;

	char name[32];
	char label[32];
};


struct gpiod_chip *gpiod_chip_open(const char *path) {
	
	printf("gpiod_chip_open(%s)\n",path );
	 
	struct gpiod_chip * chip = malloc(sizeof (struct gpiod_chip));
  
	return  chip;
};


void gpiod_chip_close(struct gpiod_chip *chip){
	if(chip){
		printf("gpiod_chip_close\n" );
		free(chip);
	}
}

int gpiod_chip_get_lines(struct gpiod_chip *chip,
			 unsigned int *offsets, unsigned int num_offsets,
			 struct gpiod_line_bulk *bulk){
	
	printf("gpiod_chip_get_lines\n" );
	return 0;
}

void gpiod_line_release_bulk(struct gpiod_line_bulk *bulk) {
	 
	printf("gpiod_line_release_bulk\n" );
}
 
int gpiod_line_request_bulk(struct gpiod_line_bulk *bulk,
				 const struct gpiod_line_request_config *config,
				 const int *default_vals){
	
	printf("gpiod_line_request_bulk\n" );
	return 0;
}
 
#endif

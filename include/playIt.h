/*
 * playIt.h
 *
 *  Created on: 29/6/2016
 *      Author: Santiago Real
 *
 */

#ifndef PLAYIT_H_
#define PLAYIT_H_


// Types definition
/*
struct EVENT_DATA {
	uint16_t ID:8;
	uint16_t options:8;
};

union DATA {
	uint16_t  all;
	struct EVENT_DATA bytes;
};

typedef struct NEXT_SIGNAL_EVENT {                  // bits description
	uint32_t time;
	union DATA data;
} next_Signal;
*/

struct OPTIONS_DATA {
	uint16_t periodic:1;
	uint16_t feedback:1;
	uint16_t FREE:14;
};

union PLAYLISTOPTIONS {
	uint16_t all;
	struct OPTIONS_DATA data;
};

struct SIGNAL_DATA {
	uint32_t time:24;
	uint32_t ID:7;
	uint32_t FeedbackReq:1;
};

typedef union NEXTSIGNAL {
	uint32_t all;
	struct SIGNAL_DATA data;
} next_Signal;

typedef struct PLAYLISTPKG {
	uint16_t length;
	union PLAYLISTOPTIONS options;
	next_Signal* list;
} playList_pkg;




// Functions
void deletePlayList(void);
void loadPlayList();

void playListStop(void);
void playListRestart(void);

void saveSignal_playList(uint16_t id, uint16_t *data);
void getSignal_playList(uint16_t id, uint16_t *data);

void waitForFeedback();
uint16_t *getFeedback();
uint16_t getFeedbackLength();


#endif /* PLAYIT_H_ */

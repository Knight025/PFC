/*
 * interp.c
 *
 *  Created on: 22/6/2016
 *      Author: Santiago Real
 *      (modified version of 'interp.c' from Jose Moya, LSEL)
 */
//#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "interp.h"
#include "serial.h"

#define MAXCMDS 100



typedef struct {
  uint16_t ID;
  int (*func) (uint16_t*);
  char *doc;
} cmd_t;

//static int done = 0;


int com_help (uint16_t* arg);


cmd_t commands[MAXCMDS] = {
  { 1, com_help, "help: Display this text" },
  { (char) 0, 0, (char *) 0 },	//Substituted NULL -> 0
};

cmd_t* find_command (uint16_t ID);

/*	Adds a command, and returns its ID
 * 	 - 'cmd_func': function to be executed when its command is called
 * 	 - 'doc': function documentation for the 'help' command
 */


int interp_addcmd (int (*cmd_func) (uint16_t*), char* doc){
  int i;
  for (i = 0; i < (MAXCMDS - 1); ++i) {
    if (! commands[i].ID) {
      commands[i].ID = i+1;
      commands[i].func = cmd_func;
      commands[i].doc = doc;
      commands[++i].ID = 0;		//Substituted NULL -> 0
      return commands[i].ID;
    }
  }
  return 0;
}


int execute_pkg (struct SCI_PKG* pkg){
  cmd_t *command;

  command = find_command(pkg->commandID);
  if (!command)
      return (-1);

  /* Call the function. */
  return (*(command->func)) (pkg->data);
}


/* Look up ID as the ID of a command, and return a pointer to that
   command.  Return a NULL pointer if ID isn't a command ID. */
cmd_t * find_command (uint16_t ID) {
	int i;
	for (i = 0; commands[i].ID; i++)
		if (ID == commands[i].ID)	//I prefer to avoid using strcmp from String.h just to compare two 'char'
			return (&commands[i]);
	return (cmd_t *) 0;	//Substituted NULL -> 0
}


/* Interface to Readline Completion */


int com_help (uint16_t *arg){
	uint16_t length = 2;
	int i;
	char* line = "\n\0";
	char* space = " \0";
	uint16_t charID;

	for (i = 0; commands[i].ID; i++) {
		//Sumo la longitud de caracteres linea a linea
		length += 2
				+ strlen(space)
				+ strlen(commands[i].doc)
				+ strlen(line);	//2 es la longitud en caracteres del ID
	}

	scia_xmit(length);
	for (i = 0; commands[i].ID; i++) {
		charID = ('0' + (commands[i].ID/10)%10) << 8 | ('0' + commands[i].ID%10);	//Envia los dos digitos del ID
		scia_xmit(charID);
		scia_msg(space, 1);
		scia_msg(commands[i].doc, 1);
		scia_msg(line, 1);
	}

  return 0;
}

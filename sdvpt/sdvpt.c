/*
    Copyright 2018 Joel Svensson	svenssonjoel@yahoo.se

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h> 

#include <rcontrolstationcomm_wrapper.h>
#include <rcontrolstationcomm_types.h> 

const char *header = "SDVPT\ntyping \"help\" shows a list of applicable commands\n";
const char *tokdelim = "\n\t\r ";

const char *hlp_str =
  "----------------------------------------------------------------------\n"\
  "help - Displays this message\n"\
  "exit - Exits from sdvpt\n"\
  "connectTcp <host> <port> - Connect to RControlStation\n"\
  "getState <car> [timeoutms] - Get state from car\n"\
  "getRoute <route> [timeoutms] - Get route points\n"\
  "addRoutePoints <car> <replace> <map only> <route> <lenght> [timeoutms] -\n"\
  "   Add points to a route\n"\
  "clearRoute <route> [timeoutms] - Clear a route\n"	\
  "errors - Lists errors if any occured on the SDVP\n"\
  "setDebugLevel <level> - Set debug level\n"\
  "carTerminal - Connect to a terminal on the car\n"\
  "----------------------------------------------------------------------\n";


const char *cmds[] ={"help",
		     "exit",
		     "q",
		     "connectTcp",
		     "disconnectTcp",
		     "getState",
                     "getRoute",
		     "addRoutePoints",
		     "clearRoute",
		     "errors",
		     "setDebugLevel",
		     "carTerminal"};

#define MAX_PROMPT_SIZE 256
char prompt[MAX_PROMPT_SIZE] = "> ";

char car_term_prompt[MAX_PROMPT_SIZE] = "('exit' to return)>";

/* ------------------------------------------------------------ 
 * Command implementations 
 * ------------------------------------------------------------ */
int exit_cmd(int n, char **args) {
  return 0; 
}

int help_cmd(int n, char **args) {
  printf("%s\n", hlp_str);
  return 1; 
}

int connectTcp_cmd(int n, char **args) {

  int port;
  int rval;
  
  if (n < 3 || n > 3) {
    printf("Wrong number of arguments!\nUsage: connectTcp <host> <port>\n");
    return 1; 
  }

  port = atoi(args[2]); 
  rval = rcsc_connectTcp(args[1], port);

  if(rval) {
    snprintf(prompt, MAX_PROMPT_SIZE, "%s:%d> ", args[1], port);
  } else {
    printf("Unable to connect!\n"); 
  }
  
  return 1; 
}

int disconnectTcp_cmd(int n, char **args) {
  
  rcsc_disconnectTcp();
  snprintf(prompt, MAX_PROMPT_SIZE, "> ");
  return 1;
}

int getState_cmd(int n, char **args) {

  CAR_STATE s;
  int rval = 0;
  int car;
  int timeoutms = 1000;
  
  if ( n < 1 || n > 2) {
    printf("Wrong number of arguments!\nUsage: getState <car> [timeoutms]\n");
    return 1;
  }

  if (n == 2) {
    timeoutms = atoi(args[2]);
  }
  car = atoi(args[1]); 
  rval = rcsc_getState(car,&s,timeoutms);
  if (rval) {
    printf("FW_MAJOR: %d\n", s.fw_major);
    printf("FW_MINOR: %d\n", s.fw_minor);
    printf("ROLL: %lf\n", s.roll);
    printf("PITCH: %lf\n", s.pitch);
    printf("YAW: %lf\n", s.yaw);
    printf("ACCEL: (%lf : %lf : %lf)\n", s.accel[0], s.accel[1], s.accel[2]);
    printf("GYRO: (%lf : %lf : %lf)\n", s.gyro[0], s.gyro[1], s.gyro[2]);
    printf("MAG: (%lf : %lf : %lf)\n", s.mag[0], s.mag[1], s.mag[2]);
    printf("PX: %lf\n", s.px);
    printf("PY: %lf\n", s.py);
    printf("SPEED: %lf\n", s.speed);
    printf("VIN: %lf\n", s.vin);
    printf("TEMP_FET: %lf\n", s.temp_fet);
    printf("MC_FAULT: %d\n", s.mc_fault);
    printf("PX_GPS: %lf\n", s.px_gps);
    printf("PY_GPS: %lf\n", s.py_gps);
    printf("AP_GOAL_PX: %lf\n", s.ap_goal_px);
    printf("AP_GOAL_PY: %lf\n", s.ap_goal_py);
    printf("AP_RAD: %lf\n", s.ap_rad);
    printf("MS_TODAY: %d\n", s.ms_today);
    printf("AP_ROUTE_LEFT: %d\n", s.ap_route_left);
    printf("PX_UWB: %lf\n", s.px_uwb);
    printf("PY_UWB: %lf\n", s.py_uwb);
  } else {
    printf("Fail!\n");
  }
  
  return 1;
}

int getRoute_cmd(int n, char **args) {

  const int max_route_len = 4096;
  ROUTE_POINT *route; 
  int car_id = 0;
  int route_id;
  int timeout = 1000;
  int route_len = 0;
  int i; 
  
  if (!(n == 2 || n == 3)) {
    printf("Wrong number of arguments!\nUsage: getRoute <route_id> [timeoutms]\n");
    return 1;
  }
  
  route = (ROUTE_POINT*)malloc(max_route_len * sizeof(ROUTE_POINT));

  if (!route){
    printf("Error allocating memory for route data\n");
    return 1;
  }

  /* car_id does not seem to matter */ 
  /*car_id = atoi(args[1]); */ 
  route_id = atoi(args[1]); 
  if (n == 3) {
    timeout = atoi(args[2]);
  }
   
  rcsc_getRoutePoints(car_id, route, &route_len, max_route_len, route_id, timeout);

  for (i = 0; i < route_len; i ++) {
    printf("%lf, %lf, %lf, %d\n",
	   route[i].px,
	   route[i].py,
	   route[i].speed,
	   route[i].time);
  }
  free(route); 
  return 1; 
}

/* <car> <replace> <map only> <route> <route len>[timeoutms] */
int addRoutePoints_cmd(int n, char **args) {
  int car;
  int replace;
  int map_only;
  int route_id;
  int len;
  int timeout = 1000;
  int i = 0;
  char buffer[256];
  int res;

  ROUTE_POINT *route = NULL; 
  
  if ( !(n = 6 || n == 7)) {
    printf("Wrong number of arguments!\nUsage: addRoutePoints <car> <replace> <map only> <route_id> <route length> [timeoutms]\n");
    return 1;
  }

  car = atoi(args[1]);
  replace = atoi(args[2]);
  map_only = atoi(args[3]);
  route_id = atoi(args[4]);
  len = atoi(args[5]);
  if (n == 7) {
    timeout = atoi(args[6]);
  }

  route = (ROUTE_POINT*)malloc(len * sizeof(ROUTE_POINT)); 
  if (!route) {
    printf("Error allocating memory!\n");
    return 1;
  }
  
  for (i = 0; i < len; i ++) {
    memset(buffer, 0, 256);
    fgets(buffer, 256, stdin); 
    sscanf(buffer, "%lf , %lf , %lf , %d \n",
	   &route[i].px,
	   &route[i].py,
	   &route[i].speed,
	   &route[i].time);
  }

  res = rcsc_addRoutePoints(car, route, len, replace, map_only, route_id, timeout);
  printf("addRoutePoints: %s!\n", res ? "Success" : "Failed"); 

  free(route);
  return 1;
}

int clearRoute_cmd(int n, char **args) {

  int route; 
  int timeout = 1000;
  int car = 0; /* Assuming this does not matter */
  int res = 0;
  
  if (!(n == 2 || n == 3)) {
    printf("Wrong number of arguments!\nUsage: clearRoute <route_id> [timeoutms]\n");
    return 1;
  }

  route = atoi(args[1]); 

  res = rcsc_clearRoute(car, route, timeout);

  printf("%s\n", res ? "Success!" : "Failed!");

  return 1;
}

int errors_cmd(int n, char **args) {

  int num_errors = 0; 

  while (rcsc_hasError()) {
    num_errors++;
    printf("%s\n", rcsc_lastError());
  }

  if (num_errors == 0) {
    printf("No errors!\n");
  }

  return 1;
}

int setDebugLevel_cmd(int n, char **args) {

  int level;

  if ( n != 2) {
    printf("Wrong number of arguments!\nUsage: setDebugLevel <level>\n");
    return 1; 
  }

  level = atoi(args[1]);
  rcsc_setDebugLevel(level);
  return 1; 
}

/* Does not work as desired */ 
int carTerminal_cmd(int n, char **args) {

  int i = 0;
  int res; 
  
  char cmdbuffer[2048];
  char *replybuffer;

  replybuffer = malloc(65536);
  
  while (1) {

    printf("%s", car_term_prompt);
    memset(cmdbuffer, 0, 2048);
    fgets(cmdbuffer, 2048, stdin);
    
    if (!strncmp("exit", cmdbuffer, 4)) {
      return 1;
    }

    /* Probably need changes to rcontrolstationcomm for this 
       to make sense */ 
    res = rcsc_sendTerminalCmd(0, cmdbuffer, replybuffer, 4000);
    
    if (!res) printf("Error!\n");
    
    printf("%s",replybuffer);
  }
}


/* ------------------------------------------------------------ 
 * Array of command functions
 * ------------------------------------------------------------ */
int (*cmd_func[]) (int, char **) = {
  &help_cmd,
  &exit_cmd,
  &exit_cmd,
  &connectTcp_cmd,
  &disconnectTcp_cmd,
  &getState_cmd,
  &getRoute_cmd,
  &addRoutePoints_cmd,
  &clearRoute_cmd, 
  &errors_cmd,
  &setDebugLevel_cmd,
  &carTerminal_cmd};

/* ------------------------------------------------------------ */ 
int tokenize(char *cmd_str, char ***tokens) {
  int num_tokens = 0; 
  char *token;
  
  token = strtok(cmd_str, tokdelim);
  while (token) {
    (*tokens)[num_tokens++] = token;

    /* Realloc or abort if out of tokenspace */ 
    
    token = strtok(NULL, tokdelim);     
  }
  return num_tokens;
}

/* ------------------------------------------------------------ */ 
int dispatch(int num_toks, char **tokens) {
  int i = 0;
  int n_cmds = sizeof(cmds) / sizeof(char *);
  
  if (!num_toks) return 1; 
  
 
  for ( i = 0; i < n_cmds; i ++) {
    if (strcmp(tokens[0], cmds[i]) == 0) {
      return (*cmd_func[i])(num_toks, tokens);
    }
  }
  printf("%s: command not found\n", tokens[0]);  
  return 1;
} 

/* ------------------------------------------------------------ 
 *  Main 
 * ------------------------------------------------------------ */
int main(int argc, char **argv){

  int running = 1; 

  char *cmd_buffer;
  size_t  cmd_buffer_size = 512;
  size_t  chars_read;

  int num_tokens = 20; /* Initial value */
  char **tokens;
  int n = 0;
  
  /* Initialisation */
  setlocale(LC_NUMERIC, "C");
  
  cmd_buffer = malloc(cmd_buffer_size * sizeof(char));

  tokens = malloc(num_tokens * sizeof(char*));
  
  printf("%s", header);

  while(running) {
    printf("%s", prompt);
    
    chars_read = getline(&cmd_buffer, &cmd_buffer_size, stdin);

    /* Tokenize the string */
    n = tokenize(cmd_buffer, &tokens);
    
    /* and dispatch */
    running = dispatch(n, tokens); 
  }
  
  
  free(tokens); 
  free(cmd_buffer);
  setlocale(LC_ALL,NULL);
  printf("Done!\n");
  
}

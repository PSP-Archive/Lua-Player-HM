#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspaudio.h>
#include <pspaudiolib.h>
#include <psputility_avmodules.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <pspthreadman.h>


#include "luaplayer.h"

#include "sound.h"

extern "C" {
#include "players/pspaudiolib.h"
#include "players/mp3playerME.h"
#include "players/aa3playerME.h"
#include "mp3.h"
#include "oggplayer.h"
};

// Forward declaration
static Voice* pushVoice(lua_State *L);


// ===================================
// Sound through mikmodlib and sound.*
// ===================================


// Music
// ------------------------------

static int Music_loadAndPlay(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
	
	const char *path = luaL_checkstring(L, 1);
	BOOL loop = false;
	if(argc == 2) loop = lua_toboolean(L, 2);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);

	FILE* soundFile = fopen(fullpath, "rb");
	if (!soundFile) return luaL_error(L, "can't open sound file %s.", fullpath);
	fclose(soundFile);
	
	lua_gc(L, LUA_GCCOLLECT, 0);
	loadAndPlayMusicFile(fullpath, loop);
	
	return 0;
}

static int Music_StopAndUnload(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	
	stopAndUnloadMusic();
	
	return 0;
}

static int Music_playing(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	
	BOOL result = musicIsPlaying();

	lua_pushboolean(L, result);
	return 1;
}

static int Music_pause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	musicPause();
	return 0;
}
static int Music_resume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	musicResume();
	return 0;
}


static int Music_volume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if(argc)
		lua_pushnumber(L, setMusicVolume((unsigned int) luaL_checknumber(L, 1)));
	else
		lua_pushnumber(L, setMusicVolume(9999));

	return 1;
}


static const luaL_reg Music_functions[] = {
  {"playFile",          Music_loadAndPlay},
  {"stop",           	Music_StopAndUnload},
  {"pause",		 		Music_pause},
  {"resume",	 		Music_resume},
  {"playing", 			Music_playing},
  {"volume", 			Music_volume},
  {0, 0}
};
// aa3me  functions
bool aa3meplaying = false;

static int aa3me_load(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
	
	const char *path = luaL_checkstring(L, 1);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);

	FILE* soundFile = fopen(fullpath, "rb");
	if (!soundFile) return luaL_error(L, "can't open sound file %s.", fullpath);
	fclose(soundFile);
	lua_gc(L, LUA_GCCOLLECT, 0);
	if (aa3meplaying == false){
	AA3ME_Init(1);
	AA3ME_Load(fullpath);
	sceKernelDelayThread(10000);
	}
	return 0;
}

static int aa3me_play(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (aa3meplaying == false){
	AA3ME_Play();
	aa3meplaying = true;
	}
	return 0;
}
static int aa3me_Stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (aa3meplaying == true){
	AA3ME_End();
	aa3meplaying = false;
	}
	return 0;
}
static int aa3me_EndOfStream(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (AA3ME_EndOfStream() == 1) { 
		lua_pushstring(L, "true");
	} else {
		lua_pushstring(L, "false");
	}
	return 1;
}
static int aa3me_getTime(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    char aa3time[200];
    char aa3timefeed[200];
	if (aa3meplaying == true){
	AA3ME_GetTimeString(aa3time);
	sprintf(aa3timefeed, "%s", aa3time);
	lua_pushstring(L, aa3timefeed);
	}
	return 1;
}
static int aa3me_percent(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	    char percent[200];
	if (aa3meplaying == true){
	AA3ME_GetPercentage();
	lua_pushstring(L, percent);
	}
	return 1;
}
static int aa3me_pause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (aa3meplaying == true){
	AA3ME_Pause();
	}
	return 0;
}
static const luaL_reg Aa3me_functions[] = {
  {"load",				aa3me_load},
  {"play",				aa3me_play},
  {"stop",				aa3me_Stop},
  {"eos",				aa3me_EndOfStream},
  {"gettime",				aa3me_getTime},
  {"percent",				aa3me_percent},
  {"pause",				aa3me_pause},
  {0, 0}
};
// MP3ME  functions
bool mp3meplaying = false;

static int Mp3me_load(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
	
	const char *path = luaL_checkstring(L, 1);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);

	FILE* soundFile = fopen(fullpath, "rb");
	if (!soundFile) return luaL_error(L, "can't open sound file %s.", fullpath);
	fclose(soundFile);
	lua_gc(L, LUA_GCCOLLECT, 0);
	if (mp3meplaying == false){
	MP3ME_Init(1);
	MP3ME_Load(fullpath);
	sceKernelDelayThread(10000);
	}
	return 0;
}

static int Mp3me_play(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (mp3meplaying == false){
	MP3ME_Play();
	mp3meplaying = true;
	}
	return 0;
}
static int Mp3me_Stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (mp3meplaying == true){
	MP3ME_End();
	mp3meplaying = false;
	}
	return 0;
}
static int Mp3me_EndOfStream(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (MP3ME_EndOfStream() == 1) { 
		lua_pushstring(L, "true");
	} else {
		lua_pushstring(L, "false");
	}
	return 1;
}
static int Mp3me_getTime(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    char mp3time[200];
    char mp3timefeed[200];
	if (mp3meplaying == true){
	MP3ME_GetTimeString(mp3time);
	sprintf(mp3timefeed, "%s", mp3time);
	lua_pushstring(L, mp3timefeed);
	}
	return 1;
}
static int Mp3me_percent(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	    char percent[200];
	if (mp3meplaying == true){
	MP3ME_GetPercentage();
	lua_pushstring(L, percent);
	}
	return 1;
}
static int Mp3me_pause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (mp3meplaying == true){
	MP3ME_Pause();
	}
	return 0;
}
static const luaL_reg Mp3me_functions[] = {
  {"load",				Mp3me_load},
  {"play",				Mp3me_play},
  {"stop",				Mp3me_Stop},
  {"eos",				Mp3me_EndOfStream},
  {"gettime",				Mp3me_getTime},
  {"percent",				Mp3me_percent},
  {"pause",				Mp3me_pause},
  {0, 0}
};
// MP3  functions
bool mp3playing = false;

static int Mp3_load(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
	
	const char *path = luaL_checkstring(L, 1);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);

	FILE* soundFile = fopen(fullpath, "rb");
	if (!soundFile) return luaL_error(L, "can't open sound file %s.", fullpath);
	fclose(soundFile);
	lua_gc(L, LUA_GCCOLLECT, 0);
	if (mp3playing == false){
	MP3_Init(1);
	MP3_Load(fullpath);
	sceKernelDelayThread(10000);	
	}
	return 0;
}

static int Mp3_Stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (mp3playing == true){
	MP3_End();
	mp3playing = false;
	}
	return 0;
}

static int Mp3_pause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (mp3playing == true){
	MP3_Pause();
	}
	return 0;
}

static int Mp3_play(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (mp3playing == false){
	MP3_Play();
	mp3playing = true;
	}
	return 0;
}

static int Mp3_EndOfStream(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (MP3_EndOfStream() == 1) { 
		lua_pushstring(L, "true");
	} else {
		lua_pushstring(L, "false");
	}
	return 1;
}

static int Mp3_getTime(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
    char mp3time[200];
    char mp3timefeed[200];
	if (mp3playing == true){
	MP3_GetTimeString(mp3time);
	sprintf(mp3timefeed, "%s", mp3time);
	lua_pushstring(L, mp3timefeed);
	}
	return 1;
}

static int Mp3_volume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "wrong number of arguments");
	MP3_Volume(luaL_checkint(L, 1));
	return 0;
}


static const luaL_reg Mp3_functions[] = {
  {"load",				Mp3_load},
  {"stop",           	Mp3_Stop},
  {"pause",		 		Mp3_pause},
  {"play",		 		Mp3_play},
  {"EndOfStream",		Mp3_EndOfStream},
  {"getTime",			Mp3_getTime},
  {"volume",			Mp3_volume},
  {0, 0}
};

// Ogg  functions
bool oggplaying = false;

static int Ogg_load(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2) return luaL_error(L, "wrong number of arguments");
	
	const char *path = luaL_checkstring(L, 1);
	
	char fullpath[512];
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);

	FILE* soundFile = fopen(fullpath, "rb");
	if (!soundFile) return luaL_error(L, "can't open sound file %s.", fullpath);
	fclose(soundFile);
	lua_gc(L, LUA_GCCOLLECT, 0);
	if (oggplaying == false){
	OGG_Init(2);
	OGG_Load(fullpath);
	sceKernelDelayThread(10000);
	}
	return 0;
}

static int Ogg_stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (oggplaying == true){
	OGG_End();
	oggplaying = false;
	}
	return 0;
}

static int Ogg_pause(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (oggplaying == true){
	OGG_Pause();
	}
	return 0;
}

static int Ogg_play(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (oggplaying == false){
	OGG_Play();
	oggplaying = true;
	}
	return 0;
}

static int Ogg_EndOfStream(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 0 ) return luaL_error(L, "wrong number of arguments");
	if (oggplaying == true){
	
	if (OGG_EndOfStream() == 1){
		lua_pushstring(L, "true");
	} else {
		lua_pushstring(L, "false");
	}
	}
	return 1;
}

static int Ogg_getSec(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, OGG_GetSec());
	return 1;
}

static int Ogg_getMin(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, OGG_GetMin());
	return 1;
}

static int Ogg_getHour(lua_State *L)
{
	if (lua_gettop(L) != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, OGG_GetHour());
	return 1;
}

static int Ogg_volume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1) return luaL_error(L, "wrong number of arguments");
	OGG_Volume(luaL_checkint(L, 1));
	return 0;
}

static const luaL_reg Ogg_functions[] = {
  {"load",				Ogg_load},
  {"stop",           	Ogg_stop},
  {"pause",		 		Ogg_pause},
  {"play",		 		Ogg_play},
  {"EndOfStream",		Ogg_EndOfStream},
  {"getSec",			Ogg_getSec},
  {"getMin",			Ogg_getMin},
  {"getHour",			Ogg_getHour},
  {"volume",			Ogg_volume},
  {0, 0}
};

// Utility functions
// ------------------------------

static int lua_setSFXVolume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "wrong number of arguments");
	int arg = (int) luaL_checknumber(L, 1);

	setSFXVolume(arg);

	return 0;
}
static int lua_setReverb(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "wrong number of arguments");
	int arg = (int) luaL_checknumber(L, 1);

	setReverb(arg);

	return 0;
}
static int lua_setPanSep(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "wrong number of arguments");
	int arg = (int) luaL_checknumber(L, 1);

	setPanSep(arg);

	return 0;
}

static const luaL_reg SoundSystem_functions[] = {
  {"SFXVolume",          		lua_setSFXVolume},
  {"reverb",           			lua_setReverb},
  {"panoramicSeparation", 		lua_setPanSep},
  {0, 0}
};


// The "Sound" userdata object.
// ------------------------------
// note: To implement a usedata object, see 
// http://lua-users.org/wiki/UserDataExample

UserdataStubs(Sound, Sound*)

static int Sound_load(lua_State *L) {
	int argc = lua_gettop(L);
	if(argc != 1 && argc != 2)
		return luaL_error(L, "Argument error: Sound.load(filename, [bool loop]) takes one or two arguments.");
	
	// Load variables
	char fullpath[512];
	const char *path = luaL_checkstring(L, 1);
	if(!path) return luaL_error(L, "Argument must be a file path.");
	getcwd(fullpath, 256);
	strcat(fullpath, "/");
	strcat(fullpath, path);
	BOOL doloop = (argc==2)?lua_toboolean(L, 2):false;
	
	FILE* soundFile = fopen(fullpath, "rb");
	if (!soundFile) return luaL_error(L, "can't open sound file %s.", fullpath);
	fclose(soundFile);
	
	// Create the user object
	lua_gc(L, LUA_GCCOLLECT, 0);
	Sound* newsound = loadSound(fullpath);;
	if (!newsound) return luaL_error(L, "error loading sound");
	Sound** luaNewsound = pushSound(L);
	*luaNewsound = newsound;
	if (doloop) setSoundLooping(newsound, 1, 0, 0);
	
	// Note: a userdata object has already been pushed.
	return 1;
}

static int Sound_gc(lua_State *L) // garbage collect
{
	Sound** handle= toSound(L, 1);
	unloadSound(*handle);
	return 0;
}

static int Sound_tostring (lua_State *L)
{
  char buff[32];
  sprintf(buff, "%p", *toSound(L, 1));
  lua_pushfstring(L, "Sound (%s)", buff);
  return 1;
}


static int Sound_play(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "Sound:play() takes no arguments. Also, call it with a colon, not a dot.");
	
	Sound** handle = toSound(L, 1);
	
	Voice *voice = pushVoice(L);
	*voice = playSound(*handle);
	
	// the Voice is already pushed
	return 1;
}






static const luaL_reg Sound_methods[] = {
  {"load",          Sound_load},
  {"play", 			Sound_play},
  {0, 0}
};
static const luaL_reg Sound_meta[] = {
  {"__gc",       Sound_gc},
  {"__tostring", Sound_tostring},
  {0, 0}
};

UserdataRegister(Sound, Sound_methods, Sound_meta)

// The "Voice" userdata object.
// ------------------------------
// note: To implement a usedata object, see 
// http://lua-users.org/wiki/UserDataExample

UserdataStubs(Voice, Voice)

static int Voice_tostring (lua_State *L)
{
  lua_pushfstring(L, "Voice (%d)", *toVoice(L, 1));
  return 1;
}


static int Voice_stop(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 ) return luaL_error(L, "Voice:stop() takes no arguments. Also, call it with a colon, not a dot.");
	
	unsigned handle = *toVoice(L, 1);

	stopSound(handle);

	return 0;
}

static int Voice_resume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2 ) return luaL_error(L, "Voice:resume(Sound) one argument. Also, call it with a colon, not a dot.");
	
	unsigned handle = *toVoice(L, 1);
	Sound* soundhandle = *toSound(L, 1);
	resumeSound(handle, soundhandle);

	return 0;
}

static int Voice_setVolume(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2 )
		return luaL_error(L, "Voice:setVolume() takes one argument.");
	
	unsigned handle = *toVoice(L, 1);
	int arg = (int) luaL_checknumber(L, 2);

	setVoiceVolume(handle, arg);

	return 0;
}

static int Voice_setPanning(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2 )
		return luaL_error(L, "Voice:setPanning() takes one argument.");
	
	unsigned handle = *toVoice(L, 1);
	int arg = (int) luaL_checknumber(L, 2);
	
	setVoicePanning(handle, arg);

	return 0;
}

static int Voice_setFrequency(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 2 )
		return luaL_error(L, "Voice:setFrequency() takes one argument.");
	
	unsigned handle = *toVoice(L, 1);
	int arg = (int) luaL_checknumber(L, 2);

	
	setVoiceFrequency(handle, arg);

	return 0;
}

static int Voice_playing(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc != 1 )
		return luaL_error(L, "Voice:playing() takes no arguments. Also, call it with a colon, not a dot.");
	
	unsigned handle = *toVoice(L, 1);
	
	BOOL result = voiceIsPlaying(handle);

	lua_pushboolean(L, result);
	return 1;
}



static const luaL_reg Voice_methods[] = {
  {"stop",          Voice_stop},
  {"resume",		Voice_resume},
  {"volume", 		Voice_setVolume},
  {"pan", 			Voice_setPanning},
  {"frequency", 	Voice_setFrequency},
  {"playing", 	 	Voice_playing},
  {0, 0}
};
static const luaL_reg Voice_meta[] = {
  // Doesn't need to be gc'd, mikmod deallocates voice when it has stopped
  {"__tostring", Voice_tostring},
  {0, 0}
};
UserdataRegister(Voice, Voice_methods, Voice_meta)



void luaSound_init(lua_State *L) {
	luaL_openlib(L, "Music", Music_functions, 0);
	luaL_openlib(L, "SoundSystem", SoundSystem_functions, 0);
	luaL_openlib(L, "Mp3me", Mp3me_functions,0);
	luaL_openlib(L, "Mp3", Mp3_functions,0);
	luaL_openlib(L, "Ogg", Ogg_functions,0);
	Sound_register(L);
	Voice_register(L);
}


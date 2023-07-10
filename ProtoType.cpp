// ProtoType.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <sys/stat.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __vita__
int _newlib_heap_size_user = 128 * 1024 * 1024;
#include <cstdio>
#include <dirent.h>
#include <vitasdk.h>
#include <vitaGL.h>
char patched_fname[256];
void patch_fname(char *fname) {
	if (!strncmp(fname, "./", 2)) {
		sprintf(patched_fname, "ux0:data/prototype/%s", &fname[2]);
	} else if (!strncmp(fname, "(null)/", 7)) {
		sprintf(patched_fname, "ux0:data/prototype/%s", &fname[7]);
	} else {
		sprintf(patched_fname, "ux0:data/prototype/%s", fname);
	}
}

extern "C" int __real_remove(const char *fname);
extern "C" int __wrap_remove(const char *fname) {
	printf("remove %s\n", fname);
	if (!strncmp(fname, "ux0:", 4)) {
		return __real_remove(fname);
	} else {
		patch_fname((char *)fname);
	}
	return __real_remove(patched_fname);
}

extern "C" FILE *__real_fopen(char *fname, char *mode);
extern "C" FILE *__wrap_fopen(char *fname, char *mode) {
	printf("fopen %s\n", fname);
	if (!strncmp(fname, "ux0:", 4)) {
		return __real_fopen(fname, mode);
	} else {
		patch_fname(fname);
	}
	return __real_fopen(patched_fname, mode);
}

extern "C" int __real_stat(const char *fname, struct stat *buf);
extern "C" int __wrap_stat(const char *fname, struct stat *buf) {
	printf("stat %s\n", fname);
	if (!strncmp(fname, "ux0:", 4)) {
		return __real_stat(fname, buf);
	} else {
		patch_fname((char *)fname);
	}
	return __real_stat(patched_fname, buf);
}

extern "C" DIR *__real_opendir(const char *fname);
extern "C" DIR *__wrap_opendir(const char *fname) {
	printf("opendir %s\n", fname);
	if (!strncmp(fname, "ux0:", 4)) {
		return __real_opendir(fname);
	} else {
		patch_fname((char *)fname);
	}
	return __real_opendir(patched_fname);
}
#endif

#ifdef AMIGAOS4
static const char* __attribute__((used)) stackcookie = "$STACK: 1000000";
#endif
#ifdef __EMSCRIPTEN__
int fileSynched = 0;

static void em_wait_loop()
{
	if(!fileSynched)
		return;
	emscripten_cancel_main_loop();
	Engine engine(640,480,false,"PROTOTYPE");
}
#endif

int main(int argc, char* argv[])
{
#ifdef __vita__
	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);
	sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
	vglInitExtended(16 * 1024 * 1024, 960, 544, 4 * 1024 * 1024, SCE_GXM_MULTISAMPLE_4X);
#endif	
	char cfg_path[PATH_MAX];
#ifdef AMIGAOS4
	snprintf(cfg_path, PATH_MAX, "%s.prototype", "PROGDIR:");
	printf("Will create %s folder\n", cfg_path);
#elif defined(__EMSCRIPTEN__)
	// that HEAP32 on &fileSynched looks like a hack, but I needed a way to be sure the DB is read before reading the ini files
	printf("Permanent storage init\n");
	EM_ASM_INT({
		FS.mkdir('prototype_data'); 
		FS.mount(IDBFS,{},'prototype_data');
		Module.print("Will import permanent storage");
		FS.syncfs(true, function() {
			Module.print("Permanent storage imported");
			HEAP32[$0>>2] = 1;
		});
	}, &fileSynched);
#elif defined(__vita__)
	snprintf(cfg_path, PATH_MAX, "ux0:data/prototype/.prototype/");
#else
	snprintf(cfg_path, PATH_MAX, "%s/.prototype/", getenv("HOME"));
#endif
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(em_wait_loop, 0, 1);
#else
	mkdir(cfg_path, 0755);
#ifdef __vita__
	Engine engine(960,544,false,"PROTOTYPE");
#else
	Engine engine(640,480,false,"PROTOTYPE");
#endif
#endif
	return 0;
}


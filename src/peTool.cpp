#include "peTool.h"

void copyBin(const char* src, const char* dest) {
	FILE* fp = fopen(src, "rb");
	if (fp == NULL) {
		return;
	}
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);

	FILE* wp = fopen(dest, "wb");
	if (wp == NULL) {
		fclose(fp);
		return;
	}

	if (len > 0) {
		byte* buf = (byte*)malloc(len);
		if (buf != NULL) {
			fseek(fp, 0, SEEK_CUR);
			fread(buf, 1, len, fp);
			fwrite(buf, 1, len, wp);
			free(buf);
			log("copy suc!");
		}
	}
	fclose(wp);
	fclose(fp);
}
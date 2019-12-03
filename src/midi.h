#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int midiOpenAll();
int midiPoll();
void midiClose();

extern unsigned char midi_ctls[256];

#ifdef __cplusplus
} // extern "C"
#endif
